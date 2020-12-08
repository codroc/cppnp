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
class TcpConnection : public IChannelCallBack{
public:
    TcpConnection(Eventloop *);
    ~TcpConnection();

    virtual void Method(int);
    void Send(int fd, const string&);
    void set_usr(ICppnpUsr *);
    map<int, Channel* > *_pmp;
private:
    Eventloop *_pEventloop;
    ICppnpUsr *_pcppnp_usr;
};

#endif // CPPNP_TCPCONNECTION_H_
