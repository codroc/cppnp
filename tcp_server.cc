#include "tcp_server.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>

#include <iostream>
#include <string>
#include <vector>

#include "channel.h"

TcpServer::TcpServer(unsigned short port, int back_log = 5){
    // initialization
    _port = port;
    _epollfd = -1;
    _listenfd = -1;
    _back_log = back_log;
}
TcpServer::~TcpServer(){}

void TcpServer::perror(std::string msg){
    std::cout << msg << " " << strerror(errno) << std::endl;
    exit(1);
}
int TcpServer::CreateAndListen(){
    int listenfd, on = 1;
    struct sockaddr_in saddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1)
        perror("listenfd");

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(_port);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    fcntl(listenfd, F_SETFL, O_NONBLOCK);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    if(-1 == bind(listenfd, (struct sockaddr*) &saddr, sizeof(saddr)))
        perror("bind");
    if(-1 == listen(listenfd, _back_log))
        perror("listen");

    return listenfd;
}
void TcpServer::Method(int fd){
    std::cout << "Method:" << fd << std::endl;
    if(fd < 0){
        std::cout << "fd < 0 error!\n";
        exit(1);
    }

    if(fd == _listenfd){
        int connfd;
        struct sockaddr_in caddr;
        socklen_t len = sizeof(caddr);

        connfd = accept(_listenfd, (struct sockaddr*) &caddr, &len);
        if(connfd == -1)
            perror("accept");
        if(connfd > 0){
            std::cout 
                << "new connection from host:"
                << "[" << inet_ntoa(caddr.sin_addr)
                << ":" << ntohs(caddr.sin_port) << "]"
                << " accepted fd: " << connfd << std::endl;
        }
        else
        {
            std::cout << "accept error, connfd:" << connfd
                << " errno:" << errno << std::endl;
        }

        fcntl(connfd, F_SETFL, O_NONBLOCK);
        Channel *pChannel_conn = new Channel(_epollfd, connfd);
        pChannel_conn->set_callback(this);
        pChannel_conn->EnableReading();
        /* 会产生内存泄漏 */
    }
    else{
        int readnum;
        const int kMaxSize = 1500;
        char buf[kMaxSize];
        memset(buf, 0, kMaxSize);
        if((readnum = read(fd, buf, kMaxSize)) < 0){
            if(errno == ECONNRESET){
                std::cout << "ECONNRESET closed socket fd:" << fd << std::endl;
                close(fd);
            }
        }
        else if(readnum == 0){
            std::cout << "fd: " << fd << " has read the end of file.\n";
            close(fd);
        }
        else{
            if(write(fd, buf, readnum) != readnum)
                std::cout << "not finished one time!\n";
            else std::cout << "--->fd = " << fd << std::endl;
        }
    }
}
void TcpServer::Start(){
    const int kMaxEventsNum = 200;
    struct epoll_event events[kMaxEventsNum];
    _epollfd = epoll_create(1);
    if(_epollfd == -1)
        perror("epoll");

    _listenfd = CreateAndListen();
    Channel listen_channel(_epollfd, _listenfd);

    listen_channel.set_callback(this);
    listen_channel.EnableReading();

    for(;;){
        std::vector<Channel*> channels;
        channels.clear();
        int fd_num = epoll_wait(_epollfd, events, kMaxEventsNum, -1);
        if(fd_num < 0)
            perror("epoll_wait");
        for(int i = 0;i < fd_num;++i){
            Channel *pChannel = (Channel*) events[i].data.ptr;
            pChannel->set_revents(events[i].events);
            channels.push_back(pChannel);
        }

        std::vector<Channel*>::iterator it;
        for(it = channels.begin();it != channels.end();++it){
            (*it)->HandleEvent();
        }
    }
}
