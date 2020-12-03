#include "tcp_server.h"
#include <vector>
#include "declare.h"
TcpServer::TcpServer(unsigned short port) {
    _port = port;
    _pAcceptor = new Acceptor(_epollfd, _port);
    _pComm = new Communicator(_epollfd);
    _pComm->_pmp = &mp;
    _pAcceptor->set_callback(this);
    std::cout << "Listen fd = " << _pAcceptor->listenfd() << std::endl;
}

TcpServer::~TcpServer(){
    map<int, Channel* >::iterator it;
    for(it = mp.begin();it != mp.end();++it)
        delete it->second;
    delete _pAcceptor;
}

void TcpServer::Start(){
    _epollfd = epoll_create(1);
    if(_epollfd < 0)
        ::perror("epoll_create");
    int listenfd = _pAcceptor->listenfd();

    Channel *ppppp = new Channel(_epollfd, listenfd);

    struct epoll_event lev, events[MAXEVENTS];
    lev.data.ptr = ppppp;
    lev.events = EPOLLIN;
    if(-1 == epoll_ctl(_epollfd, EPOLL_CTL_ADD, listenfd, &lev))
        ::perror("epoll_ctl");

    for(;;){
        std::vector<Channel*> channels;
        int num = epoll_wait(_epollfd, events, MAXEVENTS, -1);
        if(num < 0)
            ::perror("epoll_wait");
        for(int i = 0;i < num;i++){
            Channel *pChannel = static_cast<Channel*>(events[i].data.ptr);
            if(pChannel->sockfd() == listenfd) // listen socket 
                pChannel->set_callback(_pAcceptor);
            else
                pChannel->set_callback(_pComm);
            channels.push_back(pChannel);
        }

        std::vector<Channel*>::iterator it;
        for(it = channels.begin(); it != channels.end();++it)
            (*it)->HandleEvent();
    }

    delete ppppp;
}

int TcpServer::Connect(){
    int listenfd = _pAcceptor->listenfd();
    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    int connfd = accept(listenfd, (struct sockaddr*) &caddr, &len);
    fcntl(connfd, F_SETFL, O_NONBLOCK);
    if(-1 == connfd){
        std::cout << "accept failed!\n";
        return -1;
    }
    else{
        cout << "new connection from host:"
            << "[" << inet_ntoa(caddr.sin_addr)
            << ":" << ntohs(caddr.sin_port) << "]"
            << " accepted fd = " << connfd << std::endl;
    }
    Channel* pChannel_tmp = new Channel(_epollfd, connfd);
    pChannel_tmp->EnableReading();

    if(mp.end() == mp.find(connfd)){
        mp.insert(pair<int, Channel* >(connfd, pChannel_tmp));
    }
}
