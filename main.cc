#include "declare.h"
#include "tcp_server.h"
const unsigned short port = 10086;
int main(){
    TcpServer sv_tcp(port);
    sv_tcp.Start();
    return 0;
}
