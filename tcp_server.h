#ifndef CPPNP_TCP_SERVER_H_
#define CPPNP_TCP_SERVER_H_ 

#include "i_acceptor_callback.h"
#include "acceptor.h"
#include "communicator.h"
#include "channel.h"
#include <map>
using namespace std;
class TcpServer : public IAcceptorCallBack{
public:
    TcpServer(unsigned short);
    ~TcpServer();

    virtual int Connect();
    void Start();
private:
    int _epollfd;
    unsigned short _port;
    Acceptor *_pAcceptor;
    Communicator *_pComm;
    map<int, Channel* > mp;
};

#endif  // CPPNP_TCP_SERVER_H_
