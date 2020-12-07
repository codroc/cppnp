#ifndef CPPNP_COMMUNICATOR_H_
#define CPPNP_COMMUNICATOR_H_

#include "declare.h"
#include "i_channel_callback.h"
#include "channel.h"
#include "eventloop.h"

#include <map>
using namespace std;
class Communicator : public IChannelCallBack{
public:
    Communicator(Eventloop *);
    ~Communicator();

    virtual void Method(int);
    map<int, Channel* > *_pmp;
private:
    Eventloop *_pEventloop;
};

#endif // CPPNP_COMMUNICATOR_H_
