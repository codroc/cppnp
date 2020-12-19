#include <map>
#include <type_traits>

#include "tcp_connection.h"
#include "channel.h"
#include "eventloop.h"
#include "buffer.h"
#include "declare.h"
using namespace std;
map<int, TcpConnection*>* TcpConnection::_pmp;
TcpConnection::TcpConnection(Eventloop *pEventloop, int connfd) :
    _pEventloop(pEventloop)
{
    _pSockChannel = new Channel(_pEventloop, connfd);
    _pSockChannel->set_callback(this);
    _pSockChannel->EnableReading();

//    if(_pmp->end() == _pmp->find(connfd)){
//        _pmp->insert(pair<int, Channel* >(connfd, pChannel_tmp));
//    }
    _outputbuf = new Buffer;
    _inputbuf = new Buffer;
}

TcpConnection::~TcpConnection(){
    delete _pSockChannel;    
    delete _outputbuf;
    delete _inputbuf;
}

/* TcpConnection 处理读写业务 */
void TcpConnection::HandleReading(int fd){
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

        map<int, TcpConnection*>::iterator it = _pmp->find(fd);
        if(it != _pmp->end())
            _pmp->erase(it);
        delete it->second;
        close(fd);
    }
    else if(readnum == 0){
        std::cout << "has read the end of the file!\n";

        map<int, TcpConnection*>::iterator it = _pmp->find(fd);
        if(it != _pmp->end())
            _pmp->erase(it);
        delete it->second;
        close(fd);
    }
    else{
        _inputbuf->Append(buf, readnum);
        _pcppnp_usr->OnMessage(this, _inputbuf);
    }
}

void TcpConnection::HandleWriting(int fd){
    int n = _outputbuf->Write(_pSockChannel->fd());
    if(n < 0)
        cout << "write error!\n";
    else if(_outputbuf->len() > 0){
        cout << "write not finished! " << _outputbuf->len() << " bytes left!\n";
        // 注册该 Channel 的 EPOLLOUT 事件
        EnableWriting();
    }
    else{
        DisableWriting();
        _pEventloop->QueueLoop(this, NULL);
    }
}

void TcpConnection::run(void *param){ _pcppnp_usr->OnWriteComplete(this); }

void TcpConnection::set_usr(ICppnpUsr *usr) { _pcppnp_usr = usr; }
void TcpConnection::Send(const string &data){
    const char *tmp = data.c_str();
    int data_len = static_cast<int>(data.size());

    _outputbuf->Append(tmp, data_len);
    HandleWriting(_pSockChannel->fd());
}
void TcpConnection::EnableReading(){ _pSockChannel->EnableReading(); }
void TcpConnection::EnableWriting(){ _pSockChannel->EnableWriting(); }
void TcpConnection::DisableWriting() { _pSockChannel->DisableWriting(); }
void TcpConnection::ConnectionEstablished() { _pcppnp_usr->OnConnection(this); }
