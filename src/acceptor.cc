#include "acceptor.h"

#include "channel.h"
#include "declare.h"


Acceptor::Acceptor(Eventloop *pEventloop, unsigned short port = 10086){
    _pEventloop = pEventloop;
    _port = port;
    _listenfd = CreateAndListen();
    _pcallback = NULL;
    _paccept_channel = new Channel(_pEventloop, _listenfd);
    _paccept_channel->set_callback(this);
}

Acceptor::~Acceptor(){
    delete _paccept_channel;
}

int Acceptor::CreateAndListen(){
    int lfd, on = 1;
    struct sockaddr_in saddr;

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if(lfd == -1)
        ::perror("socket");
    fcntl(lfd, F_SETFL, O_NONBLOCK);
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(_port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(-1 == bind(lfd, (struct sockaddr *) &saddr, sizeof(saddr)))
        ::perror("bind");
    if(-1 == listen(lfd, BACKLOG))
        ::perror("listen");
    return lfd;
}

int Acceptor::listenfd() { return _listenfd; }
void Acceptor::set_callback(IAcceptorCallBack *p) { _pcallback = p; }


void Acceptor::HandleReading(int fd){
    _pcallback->NewConnection();
}
void Acceptor::HandleWriting(int fd){}
void Acceptor::EnableReading(){ _paccept_channel->EnableReading(); }
