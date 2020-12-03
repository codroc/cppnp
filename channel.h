#ifndef CPPNP_CHANNEL_H_
#define CPPNP_CHANNEL_H_

#include "i_channel_callback.h"
class Channel{
public:
    Channel(int, int);
    ~Channel();

    void set_callback(IChannelCallBack *);
    int epollfd();
    int sockfd();
    void set_revents(int);
    void EnableReading();
    void HandleEvent();
private:
    void update();
    int _epollfd;
    int _sockfd;
    int _events;
    int _revents;
    IChannelCallBack *_pchannel_callback;
};

#endif //CPPNP_CHANNEL_H_
