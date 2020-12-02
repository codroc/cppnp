#ifndef CPPNP_CHANNEL_H_
#define CPPNP_CHANNEL_H_ 

#include <string>
#include "call_back.h"
class Channel{
public:
    Channel(int, int);
    ~Channel();

    void perror(std::string);
    int sockfd();
    void EnableReading();
    void Update();

    void set_revents(int);
    void set_callback(ConcreteHandler *);
    void HandleEvent();
private:
    int _epollfd;
    int _sockfd;
 
    int _Revents;
    int _event;
    ConcreteHandler *_callback;
};

#endif // CPPNP_CHANNEL_H_
