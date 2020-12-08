#ifndef CPPNP_EVENTLOOP_H_
#define CPPNP_EVENTLOOP_H_ 


#include "i_cppnp_usr.h"
#include "declare.h"
class Epoll;
class Channel;
class Acceptor;
class TcpConnection;
class Eventloop{
public:
    Eventloop();
    ~Eventloop();

    void Loop();
    void Update(Channel*, int);
    void Quit();

    void set_pAcceptor(Acceptor *);
    void set_pCon(TcpConnection *);
    void set_connection_usr(ICppnpUsr *);
private:
    Epoll *_pEpoll;
    bool _quit;

    Acceptor *_pAcceptor;
    TcpConnection *_pCon;
};

#endif // CPPNP_EVENTLOOP_H_
