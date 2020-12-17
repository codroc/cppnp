#ifndef CPPNP_ECHOSERVER_H_
#define CPPNP_ECHOSERVER_H_ 
#include "i_cppnp_usr.h"
#include "i_run.h"
#include "declare.h"
class Eventloop;
class Buffer;
class TcpConnection;
class TcpServer;
class EchoServer : public ICppnpUsr, 
    public IRun
{
public:
    EchoServer(Eventloop*, unsigned short);
    ~EchoServer();

    void Start();
    const string Core(Buffer*);
    virtual void OnConnection(TcpConnection *);
    virtual void OnMessage(TcpConnection *, Buffer *); 
    virtual void OnWriteComplete(TcpConnection *);

    virtual void run(void*);
private:
    TcpServer *_ptcp_sv;
    Eventloop *_pEventloop;

    int64_t _timer;
    int _index;
};

#endif // CPPNP_ECHOSERVER_H_