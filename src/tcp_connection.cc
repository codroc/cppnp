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
map<int, shared_ptr<TcpConnection> >* TcpConnection::_pmp;
map<int, shared_ptr<TcpConnection>>::iterator find_tcpconnection(int fd){
    return TcpConnection::_pmp->find(fd);
} 
TcpConnection::TcpConnection(Eventloop *pEventloop, int connfd) :/*{{{*/
    _pEventloop(pEventloop)
{
    _pSockChannel = new Channel(_pEventloop, connfd);
    _pSockChannel->set_callback(this);// 在构造函数期间不要把 this 指针泄漏给其他类或线程！
    _pSockChannel->EnableReading();

    _outputbuf = new Buffer;
    _inputbuf = new Buffer;

}/*}}}*/

TcpConnection::~TcpConnection(){/*{{{*/
    delete _inputbuf;// new Buffer
    delete _outputbuf;// new Buffer
    delete _pSockChannel;// new Channel
}/*}}}*/

/* TcpConnection 处理读写业务 */
void TcpConnection::HandleReading(int fd){/*{{{*/
    if(fd < 0){
        printf("fd < 0 error!\n");
        return;
    }
    int readnum;
    const int kMaxBufSize = 1500;
    char buf[kMaxBufSize];
    memset(buf, 0, sizeof(buf));

    if((readnum = read(fd, buf, kMaxBufSize)) < 0){
        // 如果 errno 不是 EAGAIN 和 EINTR，那么就是对方异常断开连接
        if(errno != EAGAIN || errno != EINTR){
            auto it = ::find_tcpconnection(_pSockChannel->fd());
            if(_pmp->end() != it)
                _pmp->erase(it);
        }
    }
    else if(readnum == 0){
        auto it = ::find_tcpconnection(_pSockChannel->fd());
        if(_pmp->end() != it)
            _pmp->erase(it);
    }
    else{
        _inputbuf->Append(string(buf, readnum));
        _pcppnp_usr->OnMessage(weak_ptr<TcpConnection>(::find_tcpconnection(_pSockChannel->fd())->second), _inputbuf);
    }
}/*}}}*/

void TcpConnection::HandleWriting(int fd){
    SendInMainThread();
}
void TcpConnection::SendInMainThread(){/*{{{*/
    int n = _outputbuf->Write(_pSockChannel->fd());
    if(n < 0){
        printf("write error! Maybe turn off by other side!\n");
        auto it = ::find_tcpconnection(_pSockChannel->fd());
        if(_pmp->end() != it)
            _pmp->erase(it);
        // 对方关闭链接。
    }
    else if(_outputbuf->len() > 0){
        printf("write not finished! %d bytes left!\n", _outputbuf->len());
        // 注册该 Channel 的 EPOLLOUT 事件
        EnableWriting();
    }
    else{
        DisableWriting();
        Task task(weak_ptr<TcpConnection>(::find_tcpconnection(_pSockChannel->fd())->second));
        _pEventloop->QueueLoop(task);
    }
}/*}}}*/
void TcpConnection::run0(){ 
    _pcppnp_usr->OnWriteComplete(weak_ptr<TcpConnection>(::find_tcpconnection(_pSockChannel->fd())->second));
} 
void TcpConnection::run2(const string &msg, void *param){ // 在多线程环境下必定是父线程来执行 run2
    _outputbuf->Append(msg);
    SendInMainThread(); 
}

void TcpConnection::set_usr(ICppnpUsr *usr) { _pcppnp_usr = usr; }
void TcpConnection::Send(const string &data){// 在多线程环境下必定是子线程来执行 Send
    Task task(weak_ptr<TcpConnection>(::find_tcpconnection(_pSockChannel->fd())->second), data, weak_ptr<TcpConnection>(::find_tcpconnection(_pSockChannel->fd())->second));
    _pEventloop->QueueLoop(task);
}
void TcpConnection::EnableReading(){ _pSockChannel->EnableReading(); }
void TcpConnection::EnableWriting(){ _pSockChannel->EnableWriting(); }
void TcpConnection::DisableWriting() { _pSockChannel->DisableWriting(); }
void TcpConnection::ConnectionEstablished() { 
    _pcppnp_usr->OnConnection(weak_ptr<TcpConnection>(::find_tcpconnection(_pSockChannel->fd())->second)); 
}

// 主动关闭连接
void TcpConnection::closeConnection() { 
    auto it = ::find_tcpconnection(_pSockChannel->fd());
    if(_pmp->end() != it)
        _pmp->erase(it);// 销毁 shared_ptr 对象，计数为 0 时销毁 TcpConnection 对象
}
