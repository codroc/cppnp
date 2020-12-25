#ifndef CPPNP_CHANNEL_H_
#define CPPNP_CHANNEL_H_

#include "i_channel_callback.h"
#include "eventloop.h"
#include "declare.h"
class Channel{
public:
    Channel(Eventloop *, int);
    ~Channel();

    void set_callback(IChannelCallBack *);
    Eventloop* eventloop();
    int fd();
    void set_revents(int);
    int events();
    void EnableReading();
    void DisableReading();
    void EnableWriting();
    void DisableWriting();
    void HandleEvent();
    void Update(int);
private:
    Eventloop *_pEventloop;
    int _fd;
    int _events;
    int _revents;
    IChannelCallBack *_pchannel_callback;
};

#endif //CPPNP_CHANNEL_H_
