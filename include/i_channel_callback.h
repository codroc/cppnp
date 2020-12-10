#ifndef CPPNP_I_CHANNEL_CALLBACK_H_
#define CPPNP_I_CHANNEL_CALLBACK_H_

class IChannelCallBack{
public:
    virtual void HandleReading (int sockfd)=0;
    virtual void HandleWriting (int sockfd)=0;
};

#endif //CPPNP_I_CHANNEL_CALLBACK_H_
