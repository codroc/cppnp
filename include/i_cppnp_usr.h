#ifndef CPPNP_I_CPPNP_USR_H_
#define CPPNP_I_CPPNP_USR_H_ 

#include "declare.h"
#include <memory>
class TcpConnection;
class Buffer;
class ICppnpUsr{
public:
    virtual void OnConnection(weak_ptr<TcpConnection>)=0;
    virtual void OnMessage(weak_ptr<TcpConnection>, Buffer *)=0;

    virtual void OnWriteComplete(weak_ptr<TcpConnection>)=0;
};

#endif // CPPNP_I_CPPNP_USR_H_
