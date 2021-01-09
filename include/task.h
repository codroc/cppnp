#ifndef CPPNP_TASK_H_
#define CPPNP_TASK_H_ 
#include "i_run.h"
#include <memory>
#include <string>
class TcpConnection;
class Task{
public:
    Task(std::weak_ptr<IRun0> pIRun0) :
        _pIRun0(pIRun0),
        _pIRun2(std::weak_ptr<IRun2>()),
        _param(std::weak_ptr<void>())
    {}
    Task(std::weak_ptr<IRun2> pIRun, const std::string &str, std::weak_ptr<void> param) :
        _pIRun0(std::weak_ptr<IRun0>()),
        _pIRun2(pIRun),
        _str(str),
        _param(param)
    {}
        
    ~Task(){}
    void DoTask(){
        if(shared_ptr<IRun0> sp = _pIRun0.lock()){
            // 提升为 shared_ptr 此时计数肯定 >= 2, 这一步是线程安全的
                sp->run0();
        } 
        else if(shared_ptr<IRun2> sp1 = _pIRun2.lock()){
            // 提升为 shared_ptr 此时计数肯定 >= 2, 这一步是线程安全的
            // 提升为 shared_ptr 此时计数肯定 >= 2, 这一步是线程安全的
            shared_ptr<void> sp2 = _param.lock();
            if(sp2)
                sp1->run2(_str, sp2.get());
        } 
    }
private:
//    IRun0 *_pIRun0;
//    IRun2 *_pIRun2;
    std::weak_ptr<IRun0> _pIRun0;
    std::weak_ptr<IRun2> _pIRun2;
    std::string _str;
    std::weak_ptr<void> _param;
};

#endif // CPPNP_TASK_H_
