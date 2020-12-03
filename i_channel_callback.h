#ifndef CPPNP_I_CHANNEL_CALLBACK_H_
#define CPPNP_I_CHANNEL_CALLBACK_H_

class IChannelCallBack{
public:
    virtual void Method(int sockfd){}
};

#endif //CPPNP_I_CHANNEL_CALLBACK_H_
