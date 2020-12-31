#include <signal.h>
#include <iostream> 
#include <vector>

#include "declare.h"
#include "eventloop.h"
#include "http_sv.h"

using namespace std;
const unsigned short port = 10000;
int main(){
    signal(SIGPIPE, SIG_IGN);
    Eventloop loop;
//    cout << sizeof(Eventloop) <<endl;
//    cout << sizeof(vector<int>) << endl;
    HttpServer myhttpsv(&loop, port);
    myhttpsv.Start();
    loop.Loop();
    return 0;
}
