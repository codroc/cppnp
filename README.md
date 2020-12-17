# Version0.07

## OnWriteComplete 回调的实现

这个版本首先要实现 OnWriteComplete 回调，用于表明某个 I/O 流完整地完成了对某条信息的发生，并通知到用户。

所以用户首先要自己实现一个 OnWriteComplete 函数，当网络库完整地发送完一条消息后会回调用户的 OnWriteComplete 函数。由于每个用户对 OnWriteComplete 的实现不同，而我们网络库希望 OnWriteComplete 的接口是不变的，因此将 OnWriteComplete 函数设置为虚函数加入到 ICppnpUsr 类中，让用户继承该类并实现虚函数即可。

在上一个版本中，TcpConnection::HandleWriting 函数会处理 Buffer 中的数据，那么什么时候表示一条信息被完整地发送出去了呢？很明显，当然是 Buffer::Write 过后 Buffer::_len = 0 的时候，照理说此时我们应该就要回调用户自己定义的 OnWriteComplete 函数，但是这里有一个隐含的问题，我们需要理一下一个 I/O 流的通信流程：

1. epoll 轮询 socket 是否有事件发生。
2. 对每个发生事件的 channel 都进行 HandleEvent。
3. 对于 TcpConnection 的接收消息事件来说是用 HandleReading 来处理的，此时会读 socket 中的数据到 InputBuffer，之后再回调用户定义的 OnMessage 函数将数据传递给用户。
4. 用户得到数据和进行相关处理，将处理后的结果传递给 TcpConnection::Send 函数， TcpConnection::Send 函数再调用 TcpConnection::HandleWrting 函数，此时可能完整地完成一条消息的发生，也就是说有可能回调用户定义的 OnWriteComplete 函数。

在第 4 步，我们发现可能存在 OnMessage 回调嵌套调用 OnWriteComplete 回调的现象，这样事件的序列性被打破，会引入一堆问题。所以我们将在 EventLoop 类中设计一个异步方法 EventLoop::QueueLoop，让这个异步方法来回调 OnWriteComplete，这样就避免了回调嵌套。

EventLoop::QueueLoop 方法是本版本新添加的方法，这是一个非常非常重要的方法。在没有这个方法之前，我们使用epoll_wait() 接收的所有事件，都是来自操作系统的，比如 EPOLLIN/EPOLLOUT，我们使用 epollfd 只是用来接收操作系统的网络通知。现在我们需要让 epollfd 多做点事情，让他能接收网络库自己发送的通知。这种通知有两个重要的价值

1. 使得网络库内部在单线程的情况下具备异步处理事件的能力。
2. 使得网络库的IO线程(跑 epoll_wait 的那个线程)，可以接收来自非本线程的发送请求。

这种通知正是通过 eventfd 机制实现的，eventfd 由 Linux 2.6.22 新引入，可以像 socket 一样被 epollfd 所监听，如果向 eventfd 写点东西，epoll 就会获得这个通知并返回。EventLoop 正是通过封装 eventfd 才获得了异步处理能力。可以通过 int eventfd(unsigned int initval, int flags) 来生成一个 eventfd，将其包含入 EventLoop::_pWakeupChannel 所指的 channel 中，并将回调设置为 EventLoop::HandleReading。当第 4 步中的 Buffer::_len = 0 时就调用 EventLoop::QueueLoop，将 OnWriteComplete 回调存入 EventLoop::_pendingFunctors，并往 eventfd 中写数据，下次 epoll_wait 就会看到 eventfd 中有事件发生就会进行相应的 HandleEvent。最后 EventLoop::DoPendingFunctors 会从 EventLoop::_pendingFunctors 中一一拿出 OnWriteComplete 回调来执行。

## Timer 定时器

> 为什么网络库需要定时器？这是我首先想到的问题。

用户会用你的网络库来打造自己的服务器，如果用户需要用到定时器，例如是需要定时器的游戏服务器，例如 LOL 中技能冷却时间的定时，此时如果网络库没有提供定时器，那么用户只能自己去实现了，这就给用户带来了麻烦。

除了这一点以外，网络库有时候会出现隔一段时间去检测一个事件是否发生这种情况（例如连接是否超时等），如果用 while 循环来每次查询就会占用 CPU 时间，此时如果有个定时器那岂不是爽歪歪，时间一到通知线程即可。

所以一个好的网络库，不光是提供了网络数据收发功能，还提供了一个包括定时器，任务队列等在内的事件驱动框架。

---

### 计时器与定时器的选择

Linux 的计时函数，用于获取当前时间：
- time(2) / time_t (秒)
- ftime(3) / struct timeb (毫秒)
- gettimeofday(2) / struct timeval (微秒)
- clock_gettime(2) / struct timespec (纳秒)

定时函数，用于让程序等待一段时间或安排计划任务：
- sleep(3)
- alarm(2)
- usleep(3)
- nanosleep(2)
- clock_nanosleep(2)
- getitimer(2) / setitimer(2)
- timer_create(2) / timer_settime(2) / timer_gettime(2) / timer_delete(2)
- timerfd_create(2)) / timerfd_settime(2) / timerfd_gettime(2)

在 **muduo** 中，作者选择如下：
- (计时)只是用 gettimeofday(2) 来获取当前时间
- (定时)只是用 timerfd_* 系列函数来处理定时任务

**muduo** 作者在 7.82 节总结了选择 timerfd 来作为多线程服务器程序的定时器的原因：
1. sleep(3)/alarm(2)/usleep(3) 在实现时有可能用了 SIGALRM 信号， 在多线程程序中处理信号是个相当麻烦的事情，应当尽量避免
2. nanosleep(2) 和clock_nanosleep(2) 是线程安全的，但是在非阻塞网络编程中，绝对不能用让线程挂起的方式来等待一段时间，这样一来程序会失去响应。正确的做法是注册一个时间回调函数。
3. getitimer(2) 和 timer_create(2) 也是用信号来 deliver 超时，在多线程程序中也会有麻烦。
4. timerfd_create(2) 把时间变成了一个文件描述符，该文件描述符在定时器超时的那一刻变得可读，这样就能很方便的融入select(2)/poll(2) 框架中，用统一的方式来处理 I/O 时间和超时事件。
5. 传统的 Reactor 利用 select(2)/poll(2)/epoll(4) 的 timeout 来实现定时功能，但 poll(2)/和epoll_wait(2) 的定时精度只有毫秒，远低于 timerfd_settime(2) 的定时精度。

```c++

int timerfd_create(int clockid, int flags); //创建一个定时器文件
int timerfd_settime(int ufd, int flags, const struct itimerspec * utmr, struct itimerspec * otmr); //设置新的超时时间，并开始计时
```

timerfd_settime 可以设定 timerfd 的到时时间，如果到时则 timerfd 可读，epoll_wait 会检测到 timerfd 可读就会返回。读 timerfd 会返回一个 int64_t 类型的值，所以一般传进去一个 int64_t 类型对象的地址即可，然后判断 read 的返回值是不是 8 即可（因为 int64_t 占 8 个字节）。

---

### 具体的操作过程

首先通过 timerfd_create 创建一个 Timer 文件描述符，然后通过 timerfd_settime 来设置超时时间，之后将 Timer 文件描述符加入到 epoll 的检测，程序通过一个循环等待在 epoll_wait 上，因为没有 Timer 到时而导致阻塞。一旦定时器到时， epoll_wait 就会返回，我们就可以进行相关处理。

> 用户怎么使用网络库的 Timer 呢？
> 
> 用户要使用网络库的定时器，必须通过EventLoop。EventLoop有三个接口暴露了Timer相关的操作
> ```
> int runAt(Timestamp when, IRun* pRun); //在指定的某个时刻调用函数
>int runAfter(double delay, IRun* pRun); //等待一段时间后，调用函数
>int runEvery(double interval, IRun* pRun); //以固定的时间间隔反复调用函数
>void cancelTimer(int timerfd); //关闭一个Timer
> ```


一个定时器是否对应一个 Timer 文件描述符呢？可以这么做，但是没必要，因为可以实现一个 Timer 文件描述符为多个定时器进行服务。

假设我们有个定时器队列，按照定时时长从小到大排列，也就是说定时器先到时的排在前面（这个可以有 std::set 来实现），把 Timer 文件描述符的时间设置成队列中最前面的定时器的到时时间。这样一来，Timer 文件描述符一旦被 epoll_wait 检测到可读，就说明定时器队列中的第一个定时器到时了，就可以将到时的定时器出队，然后重新设置 Timer 文件描述符的时间。所以用户每次加入一个定时器到队列时，一定要判断定时器是否排在队列第一个位置上，如果是的话就要更新 Timer 文件描述符哦！！！

如果 Timer 文件描述符可读，那么就会去调用其对应的 Channel::HandleEvent 中的 _pchannel_callback->HandleReading，根据动态绑定技术，将此时的 _pchannel_callback 指向 TimerQueue 类型的对象首地址（令 TimerQueue 继承 IChannelCallBack 来实现），所以调用 TimerQueue::HandleReading，在这个方法里面将到时的 Timer 从 set 中移除，并执行到时时 Timer 应该做的事情，即 *Timer::run(){ _pIRun->run(param); }*，其中 _pIRun 指向 EchoSever 类的对象的首地址（令 EchoSever 继承 IRun），所以就可以让定时器到时时执行用户自己定义的 run 方法了！！！最后将那些间隔定时器重新加入到 set 中，并更新 Timer 文件描述符下次到时的时间。

整个流程大概就是这样了。


由于定时器的加入，有引进了几个内存泄漏，到时候再改~~~~











