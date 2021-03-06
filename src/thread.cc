#include "thread.h" 
#include <unistd.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <assert.h>

namespace CurrentThread{
    __thread pid_t t_cacheTid = 0;// thread local storage 线程私有数据ID，避免系统调用获取ID
}

void* func(void* arg){ ((Thread*)arg)->Run(); return NULL; }

Thread::Thread(IRun0 *pIRun, const string &name) :
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
void Thread::Run(){ _pIRun->run0(); }

pid_t Thread::gettid() const { 
    if(CurrentThread::t_cacheTid == 0)
        CurrentThread::t_cacheTid = static_cast<pid_t>(::syscall(SYS_gettid)); 
    return CurrentThread::t_cacheTid;
}
