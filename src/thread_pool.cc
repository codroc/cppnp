#include "thread_pool.h"
#include "current_thread.h"
#include <iostream>
#include <algorithm>
#include <vector>
#include "tcp_connection.h"
#include "task.h"
ThreadPool::ThreadPool() :
    _kMaxThreadNum(0)
{}

void ThreadPool::Start(int n){
    _kMaxThreadNum = n > 0 ? n : 2;
    _threads.reserve(_kMaxThreadNum);
    for(int i = 0;i < _kMaxThreadNum;i++){
        Thread *pThread = new Thread(this, "thread_pool");
        _threads.push_back(pThread);
        pThread->Start();
    }
}

void ThreadPool::AddTask(Task &task){
    _taskqueue.Put(task);
}
void ThreadPool::run0(){
    // 一大堆线程会跑到这来
    __RunInThread();
}
void ThreadPool::__RunInThread(){
    while(true){
        printf("tid = %d start work!\n", CurrentThread::tid());
        _taskqueue.Get().DoTask();
        printf("tid = %d finished work!\n", CurrentThread::tid());
    }
}
