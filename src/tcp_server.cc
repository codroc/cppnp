#include "tcp_server.h"

#include <vector>

#include "declare.h"
#include "eventloop.h"
#include "channel.h"
#include "acceptor.h"
#include "tcp_connection.h"
TcpServer::TcpServer(unsigned short port, Eventloop *pEventloop) {
    _pEventloop = pEventloop;
    _pusr = NULL;
    _port = port;
    _pAcceptor = NULL;
    TcpConnection::_pmp = &mp;
}

TcpServer::~TcpServer(){
    map<int, TcpConnection* >::iterator it;
    for(it = mp.begin();it != mp.end();++it)
        delete it->second;// new TcpConnection
    delete _pAcceptor;// new Acceptor
}

void TcpServer::set_usr(ICppnpUsr *pusr) { _pusr = pusr; }

void TcpServer::Start(){
    _pAcceptor = new Acceptor(_pEventloop, _port);
    _pAcceptor->set_callback(this);

    std::cout << "Listen fd = " << _pAcceptor->listenfd() << std::endl;
    _pAcceptor->EnableReading();
}

int TcpServer::NewConnection(){
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
    TcpConnection *ptcp_connection_tmp = new TcpConnection(_pEventloop, connfd);
    ptcp_connection_tmp->set_usr(_pusr);
    ptcp_connection_tmp->ConnectionEstablished();
    if(mp.end() == mp.find(connfd))
        mp.insert(pair<int, TcpConnection*>(connfd, ptcp_connection_tmp));
    else cout << "fd: " << connfd << "has exist!\n";
    return connfd;
}
