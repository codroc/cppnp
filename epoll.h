#ifndef CPPNP_EPOLL_H_
#define CPPNP_EPOLL_H_ 

#include "declare.h"
#include <vector>
using namespace std;
class Channel;
class Epoll{
public:
    Epoll();
    ~Epoll();

    void Poll(vector<Channel* > &);
    void Update(Channel*, int);
private:
    int _epollfd;
    struct epoll_event _events[MAXEVENTS];
};

#endif // CPPNP_EPOLL_H_
