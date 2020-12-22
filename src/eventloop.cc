#include "eventloop.h"

#include <sys/eventfd.h>

#include <vector>

#include "epoll.h"
#include "channel.h"
#include "acceptor.h"
#include "tcp_connection.h"
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

}
Eventloop::~Eventloop(){
    delete _pEventfdChannel;// new Channel
    delete _pTimerQueue;// new TimerQueue
    delete _pEpoll;// new Epoll
}

void Eventloop::Loop(){
//    cout << "quit= " << _quit << endl;
    while(!_quit){
        vector<Channel* > channels;
        _pEpoll->Poll(channels);
        
        vector<Channel* >::iterator it = channels.begin();
        for(;it != channels.end();it++){
            (*it)->HandleEvent();
        }
        // 处理异步事件，现在有 OnWriteComplete 和 Timer 事件 
        DoPendingFunctors();
    }
    cout << "Server Quit!\n";
}

void Eventloop::Update(Channel *pChannel, int ep_op=EP_ADD) { _pEpoll->Update(pChannel, ep_op); }

void Eventloop::QueueLoop(IRun *pIRun, void *param){
    Runner r(pIRun, param);
    _pendingFunctors.push_back(r);
    Wakeup();
}
void Eventloop::Wakeup(){
    uint64_t one = 1;
    int n = ::write(_eventfd, &one, sizeof(one));
    if(n != sizeof(one))
        cout << "write just writes " << n << "bytes!\n";
}
void Eventloop::HandleReading(int fd){
    uint64_t one = 1;
    int n = ::read(_eventfd, &one, sizeof one);
    if(n != sizeof one)
        cout << "reads " << n <<" bytes instead of 8\n";
}
void Eventloop::HandleWriting(int fd){}

void Eventloop::Quit() { _quit = true; }

int Eventloop::CreateEventfd(){
    int fd = eventfd(0, EFD_NONBLOCK|EFD_CLOEXEC);
    if(fd < 0)  ::perror("eventfd");
    return fd;
}

void Eventloop::DoPendingFunctors(){
    vector<Runner>::iterator it;
    vector<Runner> tmp;
    tmp.swap(_pendingFunctors);
    for(it = tmp.begin();it != tmp.end();it++)
        it->run();
}

int64_t Eventloop::runAt(Timestamp when, IRun *pIRun){
    return _pTimerQueue->AddTimer(pIRun, when, 0.0);
}
int64_t Eventloop::runAfter(double delay, IRun *pIRun){
    return _pTimerQueue->AddTimer(pIRun, Timestamp::NowAfter(delay), 0.0);
}
int64_t Eventloop::runEvery(double interval, IRun* pIRun){
    return _pTimerQueue->AddTimer(pIRun, Timestamp::NowAfter(interval), interval);
}
void Eventloop::cancelTimer(int64_t timerid){ _pTimerQueue->CancelTimer(timerid); }
