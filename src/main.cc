#include "declare.h"
#include "eventloop.h"
#include "echo_sv.h"

const unsigned short port = 10086;
int main(){
    Eventloop loop;
    EchoServer myecho(&loop, port);
    myecho.Start();
    loop.Loop();
    return 0;
}
