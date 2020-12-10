#include "eventloop.h"
#include <vector>

#include "epoll.h"
#include "channel.h"
#include "acceptor.h"
#include "tcp_connection.h"
using namespace std;
Eventloop::Eventloop(){
    _quit = false;
    _pEpoll = new Epoll;
}
Eventloop::~Eventloop(){
    delete _pEpoll;
}

void Eventloop::Loop(){
    while(!_quit){
        vector<Channel* > channels;
        _pEpoll->Poll(channels);
        
        vector<Channel* >::iterator it = channels.begin();
        for(;it != channels.end();it++){
            (*it)->HandleEvent();
        }
    }
}
void Eventloop::Update(Channel *pChannel, int ep_op=EP_ADD) { _pEpoll->Update(pChannel, ep_op); }
void Eventloop::Quit() { _quit = true; }
