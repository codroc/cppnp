#include <cstdlib>
#include <map>
#include <stdc-predef.h>
#include <type_traits>
#include <errno.h>

#include "tcp_connection.h"
#include "channel.h"
#include "eventloop.h"
#include "buffer.h"
#include "declare.h"
#include "mutex.h"
#include "task.h"
using namespace std;
map<int, TcpConnection*>* TcpConnection::_pmp;
TcpConnection::TcpConnection(Eventloop *pEventloop, int connfd) :
    _pEventloop(pEventloop)
{
    _pSockChannel = new Channel(_pEventloop, connfd);
    _pSockChannel->set_callback(this);
    _pSockChannel->EnableReading();

    _outputbuf = new Buffer;
    _inputbuf = new Buffer;

    _theothersideisclosed = true;
    _pacCount = 0;
}

TcpConnection::~TcpConnection(){
    delete _inputbuf;// new Buffer
    delete _outputbuf;// new Buffer
    delete _pSockChannel;// new Channel
}

/* TcpConnection 处理读写业务 */
void TcpConnection::HandleReading(int fd){
    if(fd < 0){
        printf("fd < 0 error!\n");
        return;
    }
    int readnum;
    const int kMaxBufSize = 1500;
    char buf[kMaxBufSize];
    memset(buf, 0, sizeof(buf));

    if((readnum = read(fd, buf, kMaxBufSize)) < 0){
        printf("readnum < 0 error!\n");
        // 如果 errno 不是 EAGAIN 和 EINTR，那么就是对方异常断开连接
        if(errno != EAGAIN || errno != EINTR)
            _theothersideisclosed = true;
    }
    else if(readnum == 0){
        // 对方正常调用 close 断开连接
        //printf("The other side closed!\n");
        _theothersideisclosed = true;
    }
    else{
        _inputbuf->Append(string(buf, readnum));
        _pcppnp_usr->OnMessage(this, _inputbuf);
        _pacCount++;// 用于记录还剩几个包没发出去
    }
    if(_theothersideisclosed && !_pacCount) rund();
}

void TcpConnection::HandleWriting(int fd){
    SendInMainThread();
}
void TcpConnection::SendInMainThread(){
    int n = _outputbuf->Write(_pSockChannel->fd());
    if(n < 0){
        printf("write error! Maybe turn off by other side!\n");
        _pacCount--;
        // 对方关闭链接。
    }
    else if(_outputbuf->len() > 0){
        printf("write not finished! %d bytes left!\n", _outputbuf->len());
        // 注册该 Channel 的 EPOLLOUT 事件
        EnableWriting();
    }
    else{
        DisableWriting();
        Task task(this);
        _pEventloop->QueueLoop(task);
    }
    if(_theothersideisclosed && !_pacCount) rund();
}
void TcpConnection::rund(){
    map<int, TcpConnection*>::iterator it = _pmp->find(_pSockChannel->fd());
    if(it != _pmp->end()){
        _pmp->erase(it);
        delete it->second;
    }
}
void TcpConnection::run0(){ _pcppnp_usr->OnWriteComplete(this); _pacCount--;}// 发送完成 _pacCount--
void TcpConnection::run2(const string &msg, void *param){ // 在多线程环境下必定是父线程来执行 run2
    _outputbuf->Append(msg);
    SendInMainThread(); 
}

void TcpConnection::set_usr(ICppnpUsr *usr) { _pcppnp_usr = usr; }
void TcpConnection::Send(const string &data){// 在多线程环境下必定是子线程来执行 Send
    Task task(this, data, this);
    _pEventloop->QueueLoop(task);
}
void TcpConnection::EnableReading(){ _pSockChannel->EnableReading(); }
void TcpConnection::EnableWriting(){ _pSockChannel->EnableWriting(); }
void TcpConnection::DisableWriting() { _pSockChannel->DisableWriting(); }
void TcpConnection::DisableReading() { _pSockChannel->DisableReading(); }
void TcpConnection::ConnectionEstablished() { _theothersideisclosed = false;_pcppnp_usr->OnConnection(this); }

void TcpConnection::closeConnection() { 
    DisableReading(); _theothersideisclosed = true; 
    if(_theothersideisclosed && !_pacCount) rund();
}
