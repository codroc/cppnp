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

    _pAcceptor = NULL;
    _pCon = NULL;
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
            else (*it)->set_callback(_pCon);
            (*it)->HandleEvent();
        }
    }
}
void Eventloop::Update(Channel *pChannel, int ep_op=EP_ADD) { _pEpoll->Update(pChannel, ep_op); }
void Eventloop::Quit() { _quit = true; }
void Eventloop::set_pAcceptor(Acceptor *pAcceptor) { _pAcceptor = pAcceptor; }
void Eventloop::set_pCon(TcpConnection *pCon) { _pCon = pCon; }
void Eventloop::set_connection_usr(ICppnpUsr *pusr) { _pCon->set_usr(pusr); }
