#ifndef CPPNP_TCP_SERVER_H_
#define CPPNP_TCP_SERVER_H_ 

#include "i_acceptor_callback.h"
#include "eventloop.h"
#include "acceptor.h"
#include "communicator.h"
#include "channel.h"
#include <map>
using namespace std;
class TcpServer : public IAcceptorCallBack{
public:
    TcpServer(unsigned short, Eventloop*);
    ~TcpServer();

    virtual int Connect();
    void Start();
private:
    Eventloop *_pEventloop;
    unsigned short _port;

    Channel *_pListen;
    map<int, Channel* > mp;
};

#endif  // CPPNP_TCP_SERVER_H_
