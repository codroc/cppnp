#include <map>

#include "channel.h"
#include "eventloop.h"
#include "tcp_connection.h"
#include "declare.h"
using namespace std;
TcpConnection::TcpConnection(Eventloop *pEventloop) :
    _pEventloop(pEventloop), _pmp(NULL)
{}

TcpConnection::~TcpConnection(){}

/* TcpConnection 处理读写业务 */
void TcpConnection::Method(int fd){
    std::cout << "fd = " << fd << std::endl;
    if(fd < 0){
        std::cout << "fd < 0 error!\n";
        return;
    }
    int readnum;
    const int kMaxBufSize = 1500;
    char buf[kMaxBufSize];
    memset(buf, 0, sizeof(buf));

    if((readnum = read(fd, buf, kMaxBufSize)) < 0){
        std::cout << "readnum < 0 error!\n";
        map<int,Channel*>::iterator it = (*_pmp).find(fd);
        if((*_pmp).end() != it){
            _pEventloop->Update(it->second, EP_DEL);
            delete it->second;
            (*_pmp).erase(it);
        }
        close(fd);
    }
    else if(readnum == 0){
        std::cout << "has read the end of the file!\n";
        map<int,Channel*>::iterator it = (*_pmp).find(fd);
        if((*_pmp).end() != it){
            _pEventloop->Update(it->second, EP_DEL);
            delete it->second;
            (*_pmp).erase(it);
        }
        close(fd);
    }
    else{
        string data(buf, readnum);
        _pcppnp_usr->OnMessage(this, fd, data);
    }
}

void TcpConnection::set_usr(ICppnpUsr *usr) { _pcppnp_usr = usr; }
void TcpConnection::Send(int fd, const string &data){
    int n = ::write(fd, data.c_str(), data.size());
    if(n != static_cast<int>(data.size()))
        cout << "write error! " << data.size() - n << " bytes left!\n";
}
