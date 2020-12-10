#ifndef CPPNP_I_CPPNP_USR_H_
#define CPPNP_I_CPPNP_USR_H_ 

#include "declare.h"
class TcpConnection;
class Buffer;
class ICppnpUsr{
public:
    virtual void OnConnection(TcpConnection*){}
    virtual void OnMessage(TcpConnection*, Buffer *){}
};

#endif // CPPNP_I_CPPNP_USR_H_
