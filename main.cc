#include "declare.h"
int main(int argc, char *argv[]){
    unsigned short port = 10086;
    if(argc > 1){
        port = atoi(argv[1]);
    }
    TcpServer sv_tcp(port, 5);
    sv_tcp.Start();
    return 0;
}
