#ifndef CPPNP_TASK_H_
#define CPPNP_TASK_H_ 
#include "i_run.h"
class TcpConnection;
class Task{
public:
    Task(IRun0 *pIRun) :
        _pIRun0(pIRun),
        _pIRun2(NULL),
        _param(NULL)
    {}
    Task(IRun2 *pIRun, const string &str, void *param) :
        _pIRun0(NULL),
        _pIRun2(pIRun),
        _str(str),
        _param(param)
    {}
        
    ~Task(){}
    void DoTask(){
        if(_pIRun0) _pIRun0->run0();
        else if(_pIRun2) _pIRun2->run2(_str, _param);
    }
private:
    IRun0 *_pIRun0;
    IRun2 *_pIRun2;
    string _str;
    void *_param;
};

#endif // CPPNP_TASK_H_
