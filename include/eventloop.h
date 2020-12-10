#ifndef CPPNP_EVENTLOOP_H_
#define CPPNP_EVENTLOOP_H_ 


#include "i_cppnp_usr.h"
#include "declare.h"
class Epoll;
class Channel;
class Eventloop{
public:
    Eventloop();
    ~Eventloop();

    void Loop();
    void Update(Channel*, int);
    void Quit();
private:
    Epoll *_pEpoll;
    bool _quit;
};

#endif // CPPNP_EVENTLOOP_H_
