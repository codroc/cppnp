#ifndef CPPNP_TCPCONNECTION_H_
#define CPPNP_TCPCONNECTION_H_

#include "declare.h"
#include "i_channel_callback.h"
#include "i_cppnp_usr.h"

#include <string>
#include <map>
using namespace std;
class Eventloop;
class Channel;
class Buffer;
class TcpConnection : public IChannelCallBack{
public:
    TcpConnection(Eventloop *, int);
    ~TcpConnection();

    virtual void HandleReading(int);
    virtual void HandleWriting(int);
    void Send(const string&);
    void set_usr(ICppnpUsr *);
    void EnableReading();
    void EnableWriting();
    void DisableWriting();
private:
    Eventloop *_pEventloop;
    Channel *_pChannel;
    // 用于跟用户沟通
    ICppnpUsr *_pcppnp_usr;

    Buffer *_outputbuf;
    Buffer *_inputbuf;

public:
    static map<int, TcpConnection*>* _pmp;
};


#endif // CPPNP_TCPCONNECTION_H_
