#ifndef CPPNP_MUTEX_H_
#define CPPNP_MUTEX_H_

#include <pthread.h>
class Mutex{
public:
    Mutex(){ ::pthread_mutex_init(&_mutex, NULL); }
    ~Mutex() { ::pthread_mutex_destroy(&_mutex); }

    void Lock() { ::pthread_mutex_lock(&_mutex); }
    void UnLock() { ::pthread_mutex_unlock(&_mutex); }
    pthread_mutex_t* GetPthreadMutex() { return &_mutex; }
private:
    pthread_mutex_t _mutex;
    // noncopyable
    Mutex(const Mutex&){};
    Mutex operator=(const Mutex&){};
};

class MutexGuard{
public:
    explicit MutexGuard(Mutex &mutex) :
        _mutex(mutex)
    { _mutex.Lock(); }
    ~MutexGuard() { _mutex.UnLock(); }
private:
    Mutex &_mutex;// 注意这里只能是通过传引用或传指针的方式！ 因为 mutex 是 noncopyable 的！
    // noncopyable
    MutexGuard(const MutexGuard&)=delete; 
    MutexGuard operator=(const MutexGuard&)=delete;
};
#endif // CPPNP_MUTEX_H_
