#include <iostream>
#include <vector>
#include "declare.h"
#include "eventloop.h"
#include "echo_sv.h"

using namespace std;
const unsigned short port = 10086;
int main(){
    Eventloop loop;
//    cout << sizeof(Eventloop) <<endl;
//    cout << sizeof(vector<int>) << endl;
    EchoServer myecho(&loop, port);
    myecho.Start();
    loop.Loop();
    return 0;
}
