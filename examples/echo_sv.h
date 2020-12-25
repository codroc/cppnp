#ifndef CPPNP_ECHOSERVER_H_
#define CPPNP_ECHOSERVER_H_ 
#include "i_cppnp_usr.h"
#include "i_run.h"
#include "declare.h"
#include "thread_pool.h"
#define MUTITHREAD
class Eventloop;
class Buffer;
class TcpConnection;
class TcpServer;
class EchoServer : public ICppnpUsr, 
    public IRun0,
    public IRun2
{
public:
    EchoServer(Eventloop*, unsigned short);
    ~EchoServer();

    void Start();
    const string Core(Buffer*);
    virtual void OnConnection(TcpConnection *);
    virtual void OnMessage(TcpConnection *, Buffer *); 
    virtual void OnWriteComplete(TcpConnection *);

    virtual void run0();
    virtual void run2(const string&, void*);
private:
    long Fib(int);
    TcpServer *_ptcp_sv;
    Eventloop *_pEventloop;
    ThreadPool _threadpool;

    int64_t _timer;
    int _index;
};

#endif // CPPNP_ECHOSERVER_H_
