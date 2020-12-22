#include "thread_pool.h"

ThreadPool::ThreadPool(int maxThreadNum = 4) : 
{ kMaxThreadNum = maxThreadNum > 0 ? maxThreadNum : 4; }

void ThreadPool::Start(){
    _threads.reserve(kMaxThreadNum);
    for(int i = 0;i < kMaxThreadNum;i++){
        Thread *pThread = new Thread(this, "thread_pool");
        _threads.push_back(pThread);
        pThread->Start();
    }
}

void ThreadPool::AddTask(IRun* pIRun){
    _taskqueue.Put(pIRun);
}
void ThreadPool::run(void *param){
    // 一大堆线程会跑到这来
    __RunInThread();
}
void ThreadPool::__RunInThread(){
    while(true){
        IRun *task = static_cast<IRun*>(_taskqueue.Get());
        task->run(NULL);
    }
}
