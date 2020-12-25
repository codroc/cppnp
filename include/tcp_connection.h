#ifndef CPPNP_TCPCONNECTION_H_
#define CPPNP_TCPCONNECTION_H_

#include "declare.h"
#include "i_channel_callback.h"
#include "i_run.h"
#include "i_cppnp_usr.h"

#include <string>
#include <map>
using namespace std;
class Eventloop;
class Channel;
class Buffer;
class TcpConnection : public IChannelCallBack
            , public IRun0
            , public IRun2
{
public:
    TcpConnection(Eventloop *, int);
    ~TcpConnection();

    virtual void HandleReading(int);
    virtual void HandleWriting(int);

    virtual void run0();
    virtual void run2(const string&, void*);

    void Send(const string&);
    void set_usr(ICppnpUsr *);
    void EnableReading();
    void EnableWriting();
    void DisableWriting();

    void ConnectionEstablished();
    void rund();// 删除 TcpConnection
private:
    void SendInMainThread();
    Eventloop *_pEventloop;
    Channel *_pSockChannel;
    // 用于跟用户沟通
    ICppnpUsr *_pcppnp_usr;

    Buffer *_outputbuf;
    Buffer *_inputbuf;

    // 用于多线程处理链接断开时 TcpConnection 对象销毁所用
    bool _theothersideisclosed;
    int _pacCount;
public:
    // 用于记录已建立链接的 TcpConnection
    static map<int, TcpConnection*>* _pmp;
};


#endif // CPPNP_TCPCONNECTION_H_
