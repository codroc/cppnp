#ifndef CPPNP_ECHOSERVER_H_
#define CPPNP_ECHOSERVER_H_ 
#include "i_cppnp_usr.h"
#include "declare.h"
class Eventloop;
class TcpConnection;
class TcpServer;
class EchoServer : public ICppnpUsr{
public:
    EchoServer(Eventloop*, unsigned short);
    ~EchoServer();

    void Start();
    const string Core(const string&);
    virtual void OnConnection(TcpConnection *);
    virtual void OnMessage(TcpConnection *, int, const string&);
private:
    TcpServer *_ptcp_sv;
    Eventloop *_pEventloop;
};

#endif // CPPNP_ECHOSERVER_H_
