#ifndef CPPNP_EVENTLOOP_H_
#define CPPNP_EVENTLOOP_H_ 

#include "epoll.h"

#include "declare.h"
class Epoll;
class Channel;
class Acceptor;
class Communicator;
class Eventloop{
public:
    Eventloop();
    ~Eventloop();

    void Loop();
    void Update(Channel*, int);
    void Quit();

    void set_pAcceptor(Acceptor *);
    void set_pComm(Communicator *);
private:
    Epoll *_pEpoll;
    bool _quit;

    Acceptor *_pAcceptor;
    Communicator *_pComm;
};

#endif // CPPNP_EVENTLOOP_H_
