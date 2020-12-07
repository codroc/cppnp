#include "declare.h"
#include "eventloop.h"
#include "tcp_server.h"

const unsigned short port = 10086;
int main(){
    Eventloop loop;
    TcpServer sv_tcp(port, &loop);
    sv_tcp.Start();
    loop.Loop();
    return 0;
}
