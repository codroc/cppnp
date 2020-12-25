#include "eventloop.h"

#include <sys/eventfd.h>

#include <vector>

#include "current_thread.h"
#include "epoll.h"
#include "channel.h"
#include "acceptor.h"
#include "tcp_connection.h"
#include "task.h"
#include "timestamp.h"
#include "timerqueue.h"
using namespace std;
Eventloop::Eventloop(){
    _quit = false;
    _pEpoll = new Epoll;
    _pTimerQueue = new TimerQueue(this);

    _eventfd = CreateEventfd();
    _pEventfdChannel = new Channel(this, _eventfd);
    _pEventfdChannel->set_callback(this);
    _pEventfdChannel->EnableReading();
    _tid = CurrentThread::tid();
}
Eventloop::~Eventloop(){
    delete _pEventfdChannel;// new Channel
    delete _pTimerQueue;// new TimerQueue
    delete _pEpoll;// new Epoll
}

void Eventloop::Loop(){
    while(!_quit){
        vector<Channel* > channels;
        _pEpoll->Poll(channels);
        
        vector<Channel* >::iterator it = channels.begin();
        for(;it != channels.end();it++){
            (*it)->HandleEvent();
        }
        DoPendingFunctors();
    }
    printf("Server Quit\n");
}

void Eventloop::Update(Channel *pChannel, int ep_op=EP_ADD) { _pEpoll->Update(pChannel, ep_op); }
bool Eventloop::isInMainThread() { return _tid == CurrentThread::tid(); }
void Eventloop::QueueLoop(Task &task){
    if(isInMainThread())
        task.DoTask();
    else{
        {
            MutexGuard lock(_mutex);
            _pendingFunctors.push_back(task);
        }
        Wakeup();// 子线程来唤醒父线程去处理异步事件
    }
}
void Eventloop::Wakeup(){
    uint64_t one = 1;
    int n = ::write(_eventfd, &one, sizeof(one));
    if(n != sizeof(one))
        printf("just writes %d bytes!\n", n);
}
void Eventloop::HandleReading(int fd){
    uint64_t one = 1;
    int n = ::read(_eventfd, &one, sizeof one);
    if(n != sizeof one)
        printf("reads %d bytes instead of 8\n", n);
}
void Eventloop::HandleWriting(int fd){}

void Eventloop::Quit() { _quit = true; }

int Eventloop::CreateEventfd(){
    int fd = eventfd(0, EFD_NONBLOCK|EFD_CLOEXEC);
    if(fd < 0)  ::perror("eventfd");
    return fd;
}

void Eventloop::DoPendingFunctors(){// 在多线程环境下必定是父线程来执行
    vector<Task>::iterator it;
    vector<Task> tmp;
    {
        MutexGuard lock(_mutex);
        tmp.swap(_pendingFunctors);// 防止我这里交换的时候，有子线程往 _pendingFunctors 中添加。
    }
    for(it = tmp.begin();it != tmp.end();it++){
        it->DoTask();
    }
}

int64_t Eventloop::runAt(Timestamp when, IRun0 *pIRun){
    return _pTimerQueue->AddTimer(pIRun, when, 0.0);
}
int64_t Eventloop::runAfter(double delay, IRun0 *pIRun){
    return _pTimerQueue->AddTimer(pIRun, Timestamp::NowAfter(delay), 0.0);
}
int64_t Eventloop::runEvery(double interval, IRun0* pIRun){
    return _pTimerQueue->AddTimer(pIRun, Timestamp::NowAfter(interval), interval);
}
void Eventloop::cancelTimer(int64_t timerid){ _pTimerQueue->CancelTimer(timerid); }
