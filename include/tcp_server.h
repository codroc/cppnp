#ifndef CPPNP_TCP_SERVER_H_
#define CPPNP_TCP_SERVER_H_ 

#include <map>
#include <memory>

#include "i_acceptor_callback.h"
class Eventloop;
class Acceptor;
class TcpConnection;
class ICppnpUsr;
class TcpServer : public IAcceptorCallBack{
public:
    TcpServer(unsigned short, Eventloop*);
    ~TcpServer();

    virtual int NewConnection();
    void set_usr(ICppnpUsr *);
    void Start();
private:
    Eventloop *_pEventloop;
    // 用于用户沟通
    ICppnpUsr *_pusr;
    unsigned short _port;

    Acceptor *_pAcceptor;
    std::map<int, std::shared_ptr<TcpConnection>> mp;
};

#endif  // CPPNP_TCP_SERVER_H_
