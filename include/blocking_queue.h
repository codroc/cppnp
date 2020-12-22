#ifndef CPPNP_BLOCKING_QUEUE_H_
#define CPPNP_BLOCKING_QUEUE_H_ 
#include "mutex.h"

#include <deque>
#include <pthread.h>
using namespace std;
template<class T>
class BlockingQueue{
public:
    BlockingQueue(){ ::pthread_cond_init(&_cond, NULL); }
    ~BlockingQueue(){ ::pthread_cond_destroy(&_cond); }

    void Put(T elem){
        MutexGuard lock(_mutex);
        _dq.push_back(elem);
        ::pthread_cond_signal(&_cond);
    }
    T Get(){
        MutexGuard(_mutex);
        while(_dq.empty){
            ::pthread_cond_wait(&_cond, _mutex.GetPhreadMutex());
        }
        T elem = _dq.front();
        // T elem(_dq.front());
        _dq.pop_back();
        return elem;
    }
private:
    deque<T> _dq;
    Mutex _mutex;
    pthread_cond_t _cond;
};

#endif // CPPNP_BLOCKING_QUEUE_H_
