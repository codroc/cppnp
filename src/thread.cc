#include "thread.h" 
#include <pthread.h>
#include <assert.h>


__thread pid_t t_cacheTid = 0;// 线程私有数据ID，避免系统调用获取ID
void* func(void* arg){ ((Thread*)arg)->Run(); return NULL; }

Thread::Tread(IRun *pIRun, const string &name) :
    _pIRun(pIRun),
    _started(false),
    _joined(false),
    _name(name)
{}

Thread::~Thread(){
    if(_started && !_joined)
        ::pthread_detach(_t);// 分离线程
}

void Thread::Start(){ 
    assert(!_started);
    _started = true;
    ::pthread_create(&_t, NULL, func, this);
}
void Thread::Join(){
    assert(_started);
    assert(!_joined);
    _joined = true;
    ::pthread_join(_t, NULL);
}
void Thread::Run(){ _pIRun->run(NULL); }

pid_t Thread::gettid() const { 
    if(t_cacheTid == 0)
        t_cacheTid = static_cast<pid_t>::syscall(SYS_gettid); 
    return t_cacheTid;
}
