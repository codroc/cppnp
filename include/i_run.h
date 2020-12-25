#ifndef CPPNP_I_RUN_H_
#define CPPNP_I_RUN_H_ 
#include<string>
using namespace std;
class IRun0{
public:
    virtual void run0()=0;
};

class IRun2{
public:
    virtual void run2(const string&, void*)=0;
};
#endif // CPPNP_I_RUN_H_
