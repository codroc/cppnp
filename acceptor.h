#ifndef CPPNP_ACCEPTOR_H_
#define CPPNP_ACCEPTOR_H_ 

#include "i_channel_callback.h"
#include "i_acceptor_callback.h"
#include "channel.h"
class Acceptor : public IChannelCallBack{
public:
    Acceptor(int, unsigned short);
    ~Acceptor();

    virtual void Method(int);
    void set_callback(IAcceptorCallBack *);
    int listenfd();
private:
    int CreateAndListen();
    int _epollfd;
    int _listenfd;
    int _port;
    IAcceptorCallBack *_pcallback;
    Channel *_paccept_channel;
};

#endif // CPPNP_ACCEPTOR_H_
