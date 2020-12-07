#include "epoll.h"

#include <sys/epoll.h>
#include "channel.h"
Epoll::Epoll(){
    _epollfd = epoll_create(1);
}

Epoll::~Epoll(){}

void Epoll::Update(Channel *pChannel, int ep_op){
    struct epoll_event ev;
    if(ep_op == EP_ADD || ep_op == EP_MOD){
        ev.data.ptr = pChannel;
        ev.events = pChannel->events();
        if(-1 == epoll_ctl(_epollfd, EPOLL_CTL_ADD, pChannel->sockfd(), &ev))
            printf("epoll_ctl return -1!\n");
    }
    else if(ep_op == EP_DEL)
        epoll_ctl(_epollfd, EPOLL_CTL_DEL, pChannel->sockfd(), &ev);
}
void Epoll::Poll(vector<Channel*> &channels){
    int num = epoll_wait(_epollfd, _events, MAXEVENTS, -1);
    if(num < 0)
        ::perror("epoll_wait return < 0");
    for(int i = 0;i < num;i++){
        Channel *pChannel = static_cast<Channel*> (_events[i].data.ptr);
        channels.push_back(pChannel);
    }
}