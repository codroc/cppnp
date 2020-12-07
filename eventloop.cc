#include "eventloop.h"
#include <vector>

#include "channel.h"
#include "acceptor.h"
#include "communicator.h"
using namespace std;
Eventloop::Eventloop(){
    _quit = false;
    _pEpoll = new Epoll;

    _pAcceptor = NULL;
    _pComm = NULL;
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
            if((*it)->sockfd() == _pAcceptor->listenfd())
                (*it)->set_callback(_pAcceptor);
            else (*it)->set_callback(_pComm);
            (*it)->HandleEvent();
        }
    }
}
void Eventloop::Update(Channel *pChannel, int ep_op=EP_ADD) { _pEpoll->Update(pChannel, ep_op); }
void Eventloop::Quit() { _quit = true; }
void Eventloop::set_pAcceptor(Acceptor *pAcceptor) { _pAcceptor = pAcceptor; }
void Eventloop::set_pComm(Communicator *pComm) { _pComm = pComm; }
