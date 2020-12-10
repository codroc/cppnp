#ifndef CPPNP_DECLARE_H_
#define CPPNP_DECLARE_H_

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

#include <iostream>
#include <string>
#include <vector>

#define BACKLOG 5
#define MAXEVENTS 200

const int kMaxAppBufSize = 40000;// 40K

using namespace std;
enum {EP_ADD = 0, EP_DEL, EP_MOD};
void perror(std::string msg, int);

#endif //CPPNP_DECLARE_H_
