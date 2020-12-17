#ifndef CPPNP_EVENTLOOP_H_
#define CPPNP_EVENTLOOP_H_ 


#include <vector>
using namespace std;
#include "i_channel_callback.h"
#include "declare.h"
#include "i_run.h"
class Epoll;
class Channel;
class TimerQueue;
class Timestamp;
class Eventloop : public IChannelCallBack{
public:
    class Runner{
    public:
        Runner(IRun* pIRun, void *p) :
            _pIRun(pIRun),
            _param(p)
        {}
        ~Runner(){}
        void run(){ _pIRun->run(_param); }
    private:
        IRun *_pIRun;
        void *_param;
    };
    Eventloop();
    ~Eventloop();

    void Loop();
    void Update(Channel*, int);
    void Quit();

    // 处理 OnWriteComplete 时间和 增删 Timer 时间
    void Wakeup();
    virtual void HandleReading(int);
    virtual void HandleWriting(int);
    void QueueLoop(IRun*, void*);

    int64_t runAt(Timestamp, IRun*);
    int64_t runAfter(double, IRun*);
    int64_t runEvery(double, IRun*);
    void cancelTimer(int64_t);
private:
    Epoll *_pEpoll;
    bool _quit;

    int CreateEventfd();
    void DoPendingFunctors();
    int _eventfd;
    Channel *_pWakeupChannel;
    vector<Runner> _pendingFunctors;

    TimerQueue *_pTimerQueue;
};

#endif // CPPNP_EVENTLOOP_H_
