#ifndef CPPNP_EVENTLOOP_H_
#define CPPNP_EVENTLOOP_H_ 


#include <vector>
using namespace std;
#include <pthread.h>
#include "i_channel_callback.h"
#include "declare.h"
#include "i_run.h"
#include "mutex.h"
#include "blocking_queue.h"
#include "thread_pool.h"
class Task;
class Epoll;
class Channel;
class TimerQueue;
class Timestamp;
class Eventloop : public IChannelCallBack{
public:
    Eventloop();
    ~Eventloop();

    void Loop();
    void Update(Channel*, int);
    void Quit();
    bool isInMainThread();

    // 处理 OnWriteComplete 时间和 增删 Timer 时间
    void Wakeup();
    virtual void HandleReading(int);
    virtual void HandleWriting(int);
    void QueueLoop(Task&);

    int64_t runAt(Timestamp, IRun0*);
    int64_t runAfter(double, IRun0*);
    int64_t runEvery(double, IRun0*);
    void cancelTimer(int64_t);
private:
    Epoll *_pEpoll;
    bool _quit;

    int CreateEventfd();
    void DoPendingFunctors();
    int _eventfd;
    Channel *_pEventfdChannel;

    TimerQueue *_pTimerQueue;
    vector<Task> _pendingFunctors;

    Mutex _mutex;
    pid_t _tid;
    ThreadPool _threadpool;
};

#endif // CPPNP_EVENTLOOP_H_
