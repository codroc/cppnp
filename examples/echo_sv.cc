#include "echo_sv.h"
#include "current_thread.h"
#include "eventloop.h"
#include "tcp_connection.h"
#include "tcp_server.h"
#include "buffer.h"
#include "task.h"
EchoServer::EchoServer(Eventloop *pEventloop, unsigned short port=80){
    _ptcp_sv = new TcpServer(port, pEventloop);
    _ptcp_sv->set_usr(this);
    _pEventloop = pEventloop;
    _timer = -1;
    _index = 0;
}

EchoServer::~EchoServer(){
    delete _ptcp_sv;// new TcpServer
}

void EchoServer::Start(){ 
    _ptcp_sv->Start(); 
    _timer = _pEventloop->runEvery(1, this);
#ifdef MUTITHREAD
    _threadpool.Start(3);
#endif
}

void EchoServer::OnConnection(TcpConnection*){
    printf("OnConnection!\n");
}
const string EchoServer::Core(Buffer *buf){  
    string response = buf->ReadAsString();
    return response;
}
void EchoServer::OnMessage(TcpConnection *pConn, Buffer *buf){
    printf("OnMessage\n");
    const string response = Core(buf);
#ifdef MUTITHREAD
    Task task(this, response, pConn);
    _threadpool.AddTask(task);
#else
    printf("#######################fib = %ld tid = %d\n",EchoServer::Fib(30), CurrentThread::tid());
    pConn->Send(response);
#endif
}
void EchoServer::OnWriteComplete(TcpConnection *pConn){
    printf("WriteCompleted!\n");
}
void EchoServer::run0(){
    printf("_index = %d\n", _index);
    _index++;
    if(_index >= 1000){
        _pEventloop->cancelTimer(_timer);
        _index = 0;
        _pEventloop->Quit();
    }
}
long EchoServer::Fib(int n){
    return (n == 1 || n == 2) ? 1 : (EchoServer::Fib(n - 1) + EchoServer::Fib(n - 2));
}
void EchoServer::run2(const string &msg, void *pCon){
    printf("#######################fib = %ld tid = %d\n",EchoServer::Fib(30), CurrentThread::tid());
    static_cast<TcpConnection*>(pCon)->Send(msg);
}
