#ifndef CPPNP_BLOCKING_QUEUE_H_
#define CPPNP_BLOCKING_QUEUE_H_ 

#include <deque>
#include <pthread.h>
#include "mutex.h"
#include "tcp_connection.h"
#include "task.h"
using namespace std;

class Task;
template<class T>
class BlockingQueue{
public:
    BlockingQueue(){ ::pthread_cond_init(&_cond, NULL); }
    ~BlockingQueue(){ ::pthread_cond_destroy(&_cond); }

    void Put(T elem){
        {
            MutexGuard lock(_mutex);
            _dq.push_back(elem);
        }
        ::pthread_cond_signal(&_cond);
    }
    T Get(){
        MutexGuard lock(_mutex);
        while(_dq.empty()){
            ::pthread_cond_wait(&_cond, _mutex.GetPthreadMutex());
        }
        T elem = _dq.front();
        // T elem(_dq.front());
        _dq.pop_front();
        return elem;
    }
private:
    std::deque<T> _dq;
    Mutex _mutex;
    pthread_cond_t _cond;
};

#endif // CPPNP_BLOCKING_QUEUE_H_
