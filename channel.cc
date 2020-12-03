#include "channel.h"

#include "sys/epoll.h"

#include "declare.h"

Channel::Channel(int epollfd, int sockfd) :
    _epollfd(epollfd),
    _sockfd(sockfd),
    _events(0),
    _revents(0),
    _pchannel_callback(NULL)
{}

Channel::~Channel(){}

void Channel::set_callback(IChannelCallBack *p) { _pchannel_callback = p; }

int Channel::epollfd() { return _epollfd; }
int Channel::sockfd() { return _sockfd; }
void Channel::set_revents(int revents) { _revents = revents; }

void Channel::update(){
    struct epoll_event ev;
    ev.data.ptr = this;
    ev.events = _events;
    if(-1 == epoll_ctl(_epollfd, EPOLL_CTL_ADD, _sockfd, &ev))
        ::perror("epoll_ctl", _sockfd);
}

void Channel::EnableReading(){
    _events |= EPOLLIN;
    update();
}

void Channel::HandleEvent(){
    _pchannel_callback->Method(_sockfd);
}
