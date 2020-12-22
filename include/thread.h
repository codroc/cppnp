#ifndef CPPNP_THREAD_H_
#define CPPNP_THREAD_H_ 

#include "i_run.h"
#include <pthread.h>
#include <string>
using namespace std;
class Thread{
public:
    explicit Thread(IRun*, const string&);
    ~Thread();

    void Start();
    void Run();
    void Join();
    pid_t gettid const ();
private:
    pthread_t _t;
    IRun *_pIRun;
    bool _started;
    bool _joined;
    const string _name;
};

#endif // CPPNP_THREAD_H_
