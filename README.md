1. 程序启动，主线程通过 ThreadPool::Start 后产生一大堆子线程不断地*while（true）死循环*去取任务和处理任务，在没有连接接入和不考虑定时器的情况下，子线程最终阻塞在 BlockingQueue::Get （等待生产者将任务放入任务队列）；主线程会阻塞在 ::epoll_wait
2. 当有一个连接接入时，主线程从 ::epoll_wait 返回，执行 Channel::HandleEvent -> TcpConnection::HandleReading -> EchoServer::OnMessage -> ThreadPool::AddTask -> BlockingQueue::Put -> 唤醒子线程 -> 继续 eventloop
3.  子线程从 BlockingQueue::Get 执行 Task::DoTask
4.  分析一下子线程需要做什么 Task。子线程只需要处理用户需要处理的任务（也就是 EchoServer 中需要处理的任务，可以是 IO 阻塞的任务或是 CPU 密集型任务等）。而发送消息、消息发送完毕的用户回显（OnWriteComplete）、增/删定时器，这些任务都由父线程（主线程）来做（因为这些任务都不是 CPU 密集型或可能导致 I/O 阻塞的任务）。这三个任务最终都会进入 EventLoop::QueueLoop 方法并由子线程放入 _pendingFunctors 中，由于只有父线程才会去执行 EventLoop::DoPendingFunctors 所以就能达到我们的目的。
```C++
void EventLoop::DoPendingFunctors(){// 这个函数应该只有主线程执行到
    vector<Task>::iterator it;
    vector<Task> tmp;
    _callingPendingFunctors = true;
    tmp.swap(_pendingFunctors);
    for(it = tmp.begin(); it != tmp.end(); ++it)
        it->DoTask();
}
```

5.  这里把最终的收发消息都由父线程来进行能够带来一个好处，那就是在多线程环境下保证了子线程不会干涉收消息和发消息，写起代码来逻辑更加清晰
6.  由于子线程要把增/删定时器的回调，发送消息的回调以及消息发送完毕的用户回显的回调都放入 _pendingFunctors 中，所以修改上一个版本的 IRun* 接口，增加 IRun0 和 IRun2 类，并封装进入到 Task 类中
```c++

class Task{
public:
    Task(IRun0 *pIRun0) :
        _pIRun0(pIRun0),
        _pIRun2(NULL),
        _param(NULL)
    {}
    Task(IRun2 *pIRun2, const string& str, void *pCon) :
        _pIRun(NULL),
        _pIRun2(pIRun),
        _param(pCon)
        _str(str)
    {}
    void DoTask() { 
        if(_pIRun0) _pIRun0->run(); 
        else if(_pIRun2)    _pIRun2->run2(_str, _param);
    }
private:
    IRun0 *_pIRun0;
    IRun2 *_pIRun2;
    string _str;
    void *_param;
};

class IRun2{
public:
    virtual void run2(const string&, void *)=0;
};
class IRun0{
public:
    virtual void run0()=0;
};
```
7.  在这个版本里面，由于引用了多线程，很容易导致出现以下几种情况，稍有不慎那将会导致段错误！！！我引入了两个变量来解决这几种可能导致段错误的问题。一个是 **TcpConnection::_theothersideisclosed** 用来判断对方是否已经断开连接，另一个是 TcpConnection::_pacCount 用来记录已经收到但还没发出的包（package）的个数
    1.  有链接接入，建立了 TcpConnection 对象，主线程还没读取数据，对方就断开连接了（ **_theothersideisclosed = true     _pacCount = 0** ）
    2.  有链接接入，建立了 TcpConnection 对象，主线程已经读取数据，并把数据放到任务队列中了，此时对方断开连接( **_theothersideisclosed = true     _pacCount = n    其中n>0** )
    3.  有链接接入，建立了 TcpConnection 对象，主线程已经读取数据并发送完数据了，此时对方断开连接( **_theothersideisclosed = true     _pacCount = 0** )
可以看到其中 1 和 3 这两种情况下我们就可以 **delete TcpConnection** 的对象了，但是 2 这种情况，我们要等到 _pacCount = 0 再删除，因为如果提前删除了，那么那些子线程再次进入到这个 TcpConnection 对象时回报段错误！那么在什么地方 **_pacCount++** 在什么地方 **_pacCount--** 呢？很简单，就是在 TcpConnection::HandleReading 方法种调用 ICppnpUsr::OnMessage 的时候 ++，在 TcpConnection::SendInMainThread 中调用 write 返回 -1 或者写完成（即 OnWriteComplete 后 --）
```C++
EchoServer::OnMessage(TcpConnection *pCon, Buffer *pBuf){
    // 注意这个函数由主线程执行，子线程是不会执行这个函数的
    ···
    Task task(this, string(pBuf->str()), pCon);
    _threadpool.AddTask(task);// 往线程池中的任务队列中添加任务
}
void TcpConnection::run0(){
    _pusr->OnWriteComplete();// 回调用户的 OnWriteComplete 方法
    _pacCount--;
}
void TcpConnection::HandleReading(int fd){
    ···
    _pcppnp_usr->OnMessage(this, _inputbuf);
    _pacCount++;
    ···

    if(_theothersideisclosed == true && _pacCount == 0) 删除该 TcpConneciton 对象
}
void TcpConnection::SendInMainThread(){
    ···
    if(write 返回 -1)   _pacCount--;
    ···

    if(_theothersideisclosed == true && _pacCount == 0) 删除该 TcpConneciton 对象
}
```
8. 在设计多线程网络库时一定要牢记，用户有权利开启或不开启多线程。所以我设计的网络库必须满足以下 2 点：（1）在多线程开启的情况下能正常运行。（2）只有一个线程的情况下能正常运行。



```c++
void EchoServer::run2(const string &msg, void *pCon) { 
    // 子线程从任务队列中拿出的任务 task，执行 task.DoTask 就可能会到这个方法里面
    cout << "fib = " << EchoServer::fib(30) << " tid = " << CurrentThread::tid() << endl; 
    // fib 是斐波那契
    ((TcpConnection*) pCon)->Send(msg); // 最后将数据传递给网络库，由网络库将数据传递给 OS，最终 OS 会把数据发送出去。
}

void TcpConnection::Send(const string& msg){
    // 注意这个方法只会由子线程来执行，因为它是被 EchoServer::run2 调用的
    Task task(this, msg, this);
    _pEventLoop->QueueLoop(task);// 子线程把这个 msg 打包成 Task 放到 EventLoop::_pendingFunctors 中
}

void TcpConnection::run2(const string &msg, void *pCon){
    SendInMainThread(msg);
}
void EventLoop::QueueLoop(Task &task){
    // 由于会有多个线程争用 _pendingFunctors 所以操作前一定要加锁
    {
        MutexGuard lock(_mutex);
        _pendingFunctors.push_back(task);       
    }
    if(!isInMainThread())
        wakeup(); // 如果时子线程就唤醒主线程去执行异步事件
}



// 增/删定时器 由于有增删两种可能，所以使用 IRun2 类的 run2 方法，其中的 const string& 参数为 AddTimer 和 CancelTimer 两种情况
// 修改 EventLoop::runAt、EventLoop::runAfter、EventLoop::runEvery、EventLoop::cancelTimer 接口

int64_t EventLoop::runAt(Timestamp, IRun0*);
int64_t EventLoop::runAfter(double, IRun0*);
int64_t EventLoop::runEvery(double, IRun0*);
void EventLoop::cancelTimer(int64_t timerid);


int64_t TimerQueue::AddTimer(IRun0* pRun, Timestamp when, double interval){
    Timer *pTimer = new Timer(when, pRun, interval);
    string s("AddTimer");
    Task task(this, s, (void*)pTimer);
    _pEventLoop->QueueLoop(task);
}
void TimerQueue::CancelTimer(int64_t timerid){
    string s("CancelTimer");
    Task task(this, s, (void*)timerid);
    _pEventLoop->QueueLoop(task);
}
void TimerQueue::DoAddTimer(){}
void TimerQueue::DoCancelTimer(Timer*){}
void TimerQueue::run2(const string& str, void* timerid){
    if(str == "AddTimer")
        DoAddTimer();
    else if(str == "AddTimer")
        DoCancelTimer((Timer*)timerid);
}
```