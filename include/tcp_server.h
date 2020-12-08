#ifndef CPPNP_TCP_SERVER_H_
#define CPPNP_TCP_SERVER_H_ 

#include <map>
using namespace std;

#include "i_acceptor_callback.h"
class Eventloop;
class Channel;
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
