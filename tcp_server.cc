#include "tcp_server.h"
#include <vector>
#include "declare.h"
TcpServer::TcpServer(unsigned short port, Eventloop *pEventloop) {
    _port = port;
    _pEventloop = pEventloop;

    Acceptor* __pAcceptor = new Acceptor(_pEventloop, _port);
    __pAcceptor->set_callback(this);
    Communicator* __pComm = new Communicator(_pEventloop);
    __pComm->_pmp = &mp;

    _pListen = new Channel(_pEventloop, __pAcceptor->listenfd());

    _pEventloop->set_pAcceptor(__pAcceptor);
    _pEventloop->set_pComm(__pComm);
    std::cout << "Listen fd = " << _pListen->sockfd() << std::endl;
}

TcpServer::~TcpServer(){
    map<int, Channel* >::iterator it;
    for(it = mp.begin();it != mp.end();++it)
        delete it->second;
}

void TcpServer::Start(){
    _pListen->EnableReading();
}

int TcpServer::Connect(){
    int listenfd = _pListen->sockfd();
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
    Channel* pChannel_tmp = new Channel(_pEventloop, connfd);
    pChannel_tmp->EnableReading();

    if(mp.end() == mp.find(connfd)){
        mp.insert(pair<int, Channel* >(connfd, pChannel_tmp));
    }
}
