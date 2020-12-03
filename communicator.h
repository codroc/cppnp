#ifndef CPPNP_COMMUNICATOR_H_
#define CPPNP_COMMUNICATOR_H_

#include "i_channel_callback.h"
#include "channel.h"
#include <map>
using namespace std;
class Communicator : public IChannelCallBack{
public:
    Communicator(int);
    ~Communicator();

    virtual void Method(int);
    map<int, Channel* > *_pmp;
private:
    int _epollfd;
};

#endif // CPPNP_COMMUNICATOR_H_
