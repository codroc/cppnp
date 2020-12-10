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

void Channel::EnableWriting(){
    _events = EPOLLOUT;
    Update(EP_MOD);
}
void Channel::EnableReading(){
    _events |= EPOLLIN;
    Update(EP_ADD);
}
void Channel::DisableWriting(){
    _events &= ~EPOLLOUT;
    Update(EP_MOD);
}
void Channel::HandleEvent(){
    if(_events & EPOLLIN)
        _pchannel_callback->HandleReading(_sockfd);
    else if(_events & EPOLLOUT)
        _pchannel_callback->HandleWriting(_sockfd);
}
