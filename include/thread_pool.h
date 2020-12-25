#ifndef CPPNP_THREAD_POOL_H_
#define CPPNP_THREAD_POOL_H_ 
#include "i_run.h"
#include "blocking_queue.h"
#include "thread.h"
#include <vector>
class Task;
using namespace std;
class ThreadPool : public IRun0{
public:
    ThreadPool();

    void Start(int);
    void AddTask(Task &);

    virtual void run0();

private:
    void __RunInThread();
    int _kMaxThreadNum;
    BlockingQueue<Task> _taskqueue;
    vector<Thread*> _threads;
};

#endif // CPPNP_THREAD_POOL_H_
