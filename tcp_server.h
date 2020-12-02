#ifndef CPPNP_TCPSERVER_H_
#define CPPNP_TCPSERVER_H_
#include <string>

#include "call_back.h"
class TcpServer : public ConcreteHandler{
public:
    TcpServer(unsigned short port, int);
    ~TcpServer();

    virtual void Method(int);

    void perror(std::string);
    int CreateAndListen();
    void Start();

private:
    int _epollfd;
    int _listenfd;
    int _back_log;

    unsigned short _port;
};


#endif // CPPNP_TCPSERVER_H_
