#include "tcp_server.h"

#include <vector>

#include "declare.h"
#include "eventloop.h"
#include "channel.h"
#include "acceptor.h"
#include "tcp_connection.h"
using namespace std;
TcpServer::TcpServer(unsigned short port, Eventloop *pEventloop) {
    _pEventloop = pEventloop;
    _pusr = NULL;
    _port = port;
    _pAcceptor = NULL;
    TcpConnection::_pmp = &mp;
}

TcpServer::~TcpServer(){
    map<int, shared_ptr<TcpConnection>>::iterator it;
    for(it = mp.begin();it != mp.end();++it)
        mp.erase(it);// 销毁 map 中 it 对应的 shared_ptr，当计数为 0 时，shared_ptr 会帮我们销毁对应的 TcpConnection 对象！
    delete _pAcceptor;// new Acceptor
}

void TcpServer::set_usr(ICppnpUsr *pusr) { _pusr = pusr; }

void TcpServer::Start(){
    _pAcceptor = new Acceptor(_pEventloop, _port);
    _pAcceptor->set_callback(this);

    printf("Listen fd = %d\n", _pAcceptor->listenfd());
    _pAcceptor->EnableReading();
}

int TcpServer::NewConnection(){
    int listenfd = _pAcceptor->listenfd();
    struct sockaddr_in caddr;
    socklen_t len = sizeof(caddr);
    int connfd = accept(listenfd, (struct sockaddr*) &caddr, &len);
    fcntl(connfd, F_SETFL, O_NONBLOCK);
    if(-1 == connfd){
        printf("accept failed!\n");
        return -1;
    }
    else{
        printf("new connection from host:[%s:%d] accepted fd = %d\n",
                inet_ntoa(caddr.sin_addr),
                ntohs(caddr.sin_port),
                connfd);
    }
    shared_ptr<TcpConnection> ptcp_connection_tmp = make_shared<TcpConnection>(_pEventloop, connfd);
    if(mp.end() == mp.find(connfd))
        mp.insert(pair<int, shared_ptr<TcpConnection>>(connfd, ptcp_connection_tmp));
    else printf("fd: %d has exist!\n", connfd);

    ptcp_connection_tmp->set_usr(_pusr);
    ptcp_connection_tmp->ConnectionEstablished();
    return connfd;
}// ptcp_connection_tmp 销毁；ptcp_connection_tmp 所指向的对象只有一个指向其的智能指针，在map中
