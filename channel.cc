#include "channel.h"

#include <sys/epoll.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <string>
#include <iostream>

#include "call_back.h"
Channel::Channel(int epollfd, int sockfd) : _epollfd(epollfd), _sockfd(sockfd), _Revents(0), _callback(NULL), _event(0) {}
Channel::~Channel(){}

int Channel::sockfd() { return _sockfd; }
void Channel::perror(std::string msg){
    std::cout << msg << " " << strerror(errno) << " fd = " << _sockfd << std::endl;
    exit(1);
}
void Channel::Update(){
    struct epoll_event ev;
    ev.data.ptr = this;
    ev.events = _event;
    if(-1 == epoll_ctl(_epollfd, EPOLL_CTL_ADD, _sockfd, &ev))
        perror("epoll_ctl");
}

void Channel::EnableReading(){
    _event |= EPOLLIN;
    Update();
}

void Channel::set_revents(int revents) { _Revents = revents; }
void Channel::set_callback(ConcreteHandler *callback){ _callback = callback; }
void Channel::HandleEvent(){ 
    if(_Revents & EPOLLIN)
        _callback->Method(_sockfd); 
}
