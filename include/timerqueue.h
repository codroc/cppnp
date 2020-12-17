#ifndef CPPNP_TIMERQUEUE_H_
#define CPPNP_TIMERQUEUE_H_ 
#include <set>
#include <vector>
using namespace std;
#include "timestamp.h"
#include "declare.h"
#include "i_channel_callback.h"
#include "i_run.h"
class Eventloop;
class Channel;
class TimerQueue : public IChannelCallBack{
public:
    class Timer{
    public:
        Timer(Timestamp stamp, IRun* pRun, double interval)
            : _stamp(stamp), _id(stamp), _pRun(pRun), _interval(interval)
        {}
        ~Timer(){};

        Timestamp stamp() { return _stamp; }
        Timestamp id() { return _id; }
        void run() { _pRun->run(this); }
        bool isRepeat() { return _interval > 0.0; }
        void moveToNext() { _stamp = Timestamp::NowAfter(_interval); }
    private:
        Timestamp _stamp;
        Timestamp _id;
        IRun *_pRun;
        double _interval; // seconds
    };

    class AddTimerWrapper : public IRun{
    public:
        AddTimerWrapper(TimerQueue* pTimerQueue) :
            _pTimerQueue(pTimerQueue){}
        virtual void run(void *param){ _pTimerQueue->DoAddTimer(param); }
    private:
        TimerQueue *_pTimerQueue;
    };

    class CancelTimerWrapper : public IRun{
    public:
        CancelTimerWrapper(TimerQueue* pTimerQueue) :
            _pTimerQueue(pTimerQueue){}
        virtual void run(void *param){ _pTimerQueue->DoCancelTimer(param); }
    private:
        TimerQueue *_pTimerQueue;
    };
    TimerQueue(Eventloop*);
    ~TimerQueue();
    void DoAddTimer(void *param);
    void DoCancelTimer(void *param);
    int64_t AddTimer(IRun*, Timestamp, double);
    void CancelTimer(int64_t);

    virtual void HandleReading(int);
    virtual void HandleWriting(int);
private:
    typedef std::pair<Timestamp, Timer*> Entry;
    typedef std::set<Entry> TimerList;

    int CreateTimerfd();
    struct timespec HowMuchTimeFromNow(Timestamp);
    void ResetTimerfd(Timestamp);
    bool insert(Timer*);
    void reset(const vector<Entry>&, Timestamp);
    int _timerfd;
    TimerList _timers;
    Eventloop *_pEventloop;
    Channel *_pTimerfdChannel;

    AddTimerWrapper *_pAddTimerWrapper;
    CancelTimerWrapper *_pCancelTimerWrapper;
};

#endif //CPPNP_TIMERQUEUE_H_
