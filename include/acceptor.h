#ifndef CPPNP_ACCEPTOR_H_
#define CPPNP_ACCEPTOR_H_ 

#include "i_channel_callback.h"
#include "i_acceptor_callback.h"
class Channel;
class Eventloop;
class Acceptor : public IChannelCallBack{
public:
    Acceptor(Eventloop*, unsigned short);
    ~Acceptor();

    virtual void HandleReading(int);
    virtual void HandleWriting(int);
    void set_callback(IAcceptorCallBack *);
    int listenfd();
    void EnableReading();
private:
    Eventloop *_pEventloop;

    int CreateAndListen();
    int _listenfd;
    int _port;
    IAcceptorCallBack *_pcallback;
    Channel *_paccept_channel;
};

#endif // CPPNP_ACCEPTOR_H_
