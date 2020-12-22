#ifndef CPPNP_THREAD_POOL_H_
#define CPPNP_THREAD_POOL_H_ 
#include "i_run.h"
#include "blocking_queue.h"
#include "thread.h"
#include <vector>
using namespace std;
class ThreadPool : public IRun{
public:
    ThreadPool(int);

    void Start();
    void AddTask(IRun*);
    

    virtual void run(void*);

private:
    void __RunInThread();
    const int _kMaxThreadNum;
    BlockingQueue<IRun*> _taskqueue;
    vector<Thread*> _threads;
};

#endif // CPPNP_THREAD_POOL_H_
