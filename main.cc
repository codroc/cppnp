#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fcntl.h>
#include <arpa/inet.h>
#include <error.h>

using namespace std;

#define MAXSIZE 4096
#define MAXQUEUE 5
#define MAXEVENTS 200
const short int serv_port = 22222;

void perror(const string msg){
    cout << msg << errno << endl;
    exit(0);
}
int createAndListen(){
    int listenfd;
    int on = 1;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    fcntl(listenfd, F_SETFL, O_NONBLOCK); // non-block IO
//    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(serv_port);
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(-1 == bind(listenfd, (sockaddr*) &serv_addr, (socklen_t) sizeof(serv_addr)))
        perror("bind error:");
    if(-1 == listen(listenfd, MAXQUEUE))
        perror("listen:");
    return listenfd;
}
int main(){
    char buf[MAXSIZE];
    int sockfd, listenfd, connfd;
    struct epoll_event ev, events[MAXEVENTS];
    struct sockaddr_in clieaddr;
    socklen_t client = sizeof(sockaddr_in);

//    int num = 0;
    map<int, pair<in_addr, short int> > connfd2addr;
//    vector<pair<in_addr, short int> > clients;
    listenfd = createAndListen();

    ev.data.fd = listenfd;
    ev.events = EPOLLIN;

    int epfd = epoll_create(1);
    if(epfd < 0){
        cout << "epfd < 0 error!\n";
        exit(0);
    }
    if(-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev))
        perror("epoll_ctl:");

    for(;;){
        int n = epoll_wait(epfd, events, MAXEVENTS, -1);
        if(n < 0)
            perror("epoll_wait:");
        for(int i = 0;i < n;i++){
            if(events[i].data.fd == listenfd){
                connfd = accept(listenfd, (sockaddr*) &clieaddr, (socklen_t*) &client);
                if(-1 == connfd)
                    perror("accept:");
                else{
                    cout << "new connection from host:"
                         << "[" << inet_ntoa(clieaddr.sin_addr)
                         << ntohs(clieaddr.sin_port) << "]"
                         << " accepted fd: " << connfd << endl;
                }

                short int clieport = clieaddr.sin_port;
                in_addr clieip = clieaddr.sin_addr;

                if(connfd2addr.end() == connfd2addr.find(connfd)){
                    connfd2addr.insert(pair<int, pair<in_addr, short int> >(connfd, make_pair(clieip, clieport)));
                }

                ev.data.fd = connfd;
                ev.events = EPOLLIN;
                fcntl(connfd, F_SETFL, O_NONBLOCK);
                if(-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, connfd, &ev))
                    perror("epoll_ctl:");

            }
            else if(events[i].events & EPOLLIN){
                int readnum;
                if((sockfd = events[i].data.fd) < 0){
                    cout << "EPOLLIN sockfd < 0 error!\n";
                    continue;
                }
                else if((readnum = read(sockfd, buf, MAXSIZE)) < 0)
                    perror("read:");
                else if(readnum == 0){
                    cout << "fd: " << sockfd <<" has read the end of file.\t";
                    cout << "host:[" << inet_ntoa(connfd2addr[sockfd].first) 
                         << ":" << ntohs(connfd2addr[sockfd].second) << "] disconnected!\n";
                    connfd2addr.erase(connfd2addr.find(connfd));
                    close(sockfd);
                }
                else{
                    if(write(sockfd, buf, readnum) != readnum)
                        cout << "not finished one time!\n";
                    else{
                        cout << "----> fd = " << sockfd << endl;
                    }
                }
            }
        }
    }
    return 0;
}
