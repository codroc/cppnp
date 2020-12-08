#include "channel.h"

#include "sys/epoll.h"

#include "eventloop.h"
Channel::Channel(Eventloop *pEventloop, int sockfd) :
    _pEventloop(pEventloop),
    _sockfd(sockfd),
    _events(0),
    _revents(0),
    _pchannel_callback(NULL)
{}

Channel::~Channel(){}

void Channel::set_callback(IChannelCallBack *p) { _pchannel_callback = p; }

Eventloop* Channel::eventloop() { return _pEventloop; }
int Channel::sockfd() { return _sockfd; }
void Channel::set_revents(int revents) { _revents = revents; }
int Channel::events() { return _events; }

void Channel::Update(int ep_op=EP_ADD){
    _pEventloop->Update(this, ep_op);
}

void Channel::EnableReading(){
    _events |= EPOLLIN;
    Update(EP_ADD);
}

void Channel::HandleEvent(){
    _pchannel_callback->Method(_sockfd);
}
