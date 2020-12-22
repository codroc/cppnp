#include "timerqueue.h"

#include <algorithm>
#include <endian.h>
#include <iterator>
#include <sys/timerfd.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "channel.h"
#include "eventloop.h"
#include "timestamp.h"

#include <set>


TimerQueue::TimerQueue(Eventloop *pEventloop) :
    _timerfd(CreateTimerfd()),
    _pEventloop(pEventloop),
    _pTimerfdChannel(new Channel(_pEventloop, _timerfd)),   
    _pAddTimerWrapper(new AddTimerWrapper(this)),           
    _pCancelTimerWrapper(new CancelTimerWrapper(this))      
{
    _pTimerfdChannel->set_callback(this);
    _pTimerfdChannel->EnableReading();
}
TimerQueue::~TimerQueue(){
    delete _pCancelTimerWrapper;// new CancelTimerWrapper
    delete _pAddTimerWrapper;// new AddTimerWrapper
    delete _pTimerfdChannel;// new Channel
}    

int TimerQueue::CreateTimerfd(){
    int timerfd = ::timerfd_create(CLOCK_MONOTONIC, 
            TFD_NONBLOCK|TFD_CLOEXEC);
    if(timerfd < 0)
    ::perror("timerfd_create");
    return timerfd;
}

int64_t TimerQueue::AddTimer(IRun *pRun, Timestamp stamp, double interval)
{
    Timer* pTimer = new Timer(stamp, pRun, interval);
    _pEventloop->QueueLoop(_pAddTimerWrapper, pTimer);
    return (int64_t)pTimer;
}
void TimerQueue::CancelTimer(int64_t timerid){
    _pEventloop->QueueLoop(_pCancelTimerWrapper, (void*)timerid);
}
void TimerQueue::DoCancelTimer(void *param){
    Timer *pTimer = static_cast<Timer*> (param);
    TimerList::iterator it;
    for(it = _timers.begin(); it != _timers.end();++it){
        if(it->second == pTimer){
            delete it->second;//new pTimer;
            _timers.erase(it);
            break;
        }
    }
}
void TimerQueue::DoAddTimer(void *param){
    Timer *pTimer = static_cast<Timer*>(param);
    bool isEarlist = insert(pTimer);
    if(isEarlist)   ResetTimerfd(pTimer->stamp());
}

void TimerQueue::ResetTimerfd(Timestamp stamp){
    struct itimerspec newvalue;
    struct itimerspec oldvalue;
    bzero(&newvalue, sizeof(newvalue));
    bzero(&oldvalue, sizeof(oldvalue));
    newvalue.it_value = HowMuchTimeFromNow(stamp);
    int ret = ::timerfd_settime(_timerfd, 0, &newvalue, &oldvalue);
    if(ret) ::perror("timerfd_settime");
}

struct timespec TimerQueue::HowMuchTimeFromNow(Timestamp when){
    int64_t microseconds = when.microSecondsSinceEpoch() - 
        Timestamp::NowMicroSeconds();
    if(microseconds < 100) microseconds = 100;
    struct timespec ts;
    ts.tv_sec = static_cast<time_t>(
            microseconds / Timestamp::kMicroSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(
            (microseconds % Timestamp::kMicroSecondsPerSecond)
            * 1000);
    return ts;
}


bool TimerQueue::insert(Timer *pTimer){
    bool earlist = false;
    Timestamp when = pTimer->stamp();
    Entry e(when, pTimer);
    TimerList::iterator it = _timers.begin();
    if(it == _timers.end() || when < it->second->stamp())
        earlist = true;
    _timers.insert(e);
    return earlist;
}
void TimerQueue::HandleReading(int fd){
    Timestamp now = Timestamp::Now();

    // 这一段是读 timerfd 时固定的写法
    int64_t one;
    int n = ::read(_timerfd, &one, sizeof(one));
    if(n != sizeof one) ::perror("read");
    /////////////////////////////////////////////
    
    /* 获取那些到时的定时器，并做出相应的动作 */
    vector<Entry> vExpired;
    Entry e(now, (Timer*)0xffffffff);
    TimerList::iterator end = _timers.lower_bound(e);
    std::copy(_timers.begin(), end, back_inserter(vExpired));
    _timers.erase(_timers.begin(), end);

    vector<Entry>::iterator it = vExpired.begin();
    for(; it != vExpired.end(); ++it)
        it->second->run();// 这里执行的是 Timer::run(){ _pRun->run(this); } 也就是会调用 用户自己写的 run 方法
    reset(vExpired, now);// 如果在 vExpired 中有 Timer 是间隔定时的则需要重新加入到 TimerQueue 中，并重置下 timerfd 
}
void TimerQueue::HandleWriting(int fd){}
void TimerQueue::reset(const vector<Entry> &vi, Timestamp stamp){
    vector<Entry>::const_iterator cit;
    for(cit = vi.begin();cit != vi.end();cit++){
        Timer *pTimer = cit->second;
        if(pTimer->isRepeat()){
            pTimer->moveToNext();
            _timers.insert(Entry(pTimer->stamp(), pTimer));
        }
    }

    Timestamp nextExpire;// 下一次到时时间应该是 TimerQueue 中第一个定时器的 Timestamp
    if(!_timers.empty())
        nextExpire = _timers.begin()->second->stamp();
    if(nextExpire.valid())
        ResetTimerfd(nextExpire);
}
