#include "echo_sv.h"
#include "eventloop.h"
#include "tcp_connection.h"
#include "tcp_server.h"
#include "buffer.h"

EchoServer::EchoServer(Eventloop *pEventloop, unsigned short port=80){
    _ptcp_sv = new TcpServer(port, pEventloop);
    _ptcp_sv->set_usr(this);
    _pEventloop = pEventloop;
    _timer = -1;
    _index = 0;
}

EchoServer::~EchoServer(){
    delete _ptcp_sv;
}

void EchoServer::Start(){ 
    _ptcp_sv->Start(); 
    _timer = _pEventloop->runEvery(1, this);
}

void EchoServer::OnConnection(TcpConnection*){
    cout << "OnConnection!\n";
}
const string EchoServer::Core(Buffer *buf){  
    string response;
    response = string (buf->str());
    return response;
}
void EchoServer::OnMessage(TcpConnection *pConn, Buffer *buf){
    cout << "OnMessage\n";
    const string response = Core(buf);
    pConn->Send(response);
}
void EchoServer::OnWriteComplete(TcpConnection *pConn){
    cout << "WriteCompleted!" << endl;
}
// param: 指向 Timer 的指针
void EchoServer::run(void *param){
    cout << "_index = " << _index << endl;
    _index++;
    if(_index >= 1000){
        _pEventloop->cancelTimer(_timer);
        _index = 0;
    }
}
