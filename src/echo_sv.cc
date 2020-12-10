#include "echo_sv.h"
#include "eventloop.h"
#include "tcp_connection.h"
#include "tcp_server.h"
#include "buffer.h"

EchoServer::EchoServer(Eventloop *pEventloop, unsigned short port=80){
    _ptcp_sv = new TcpServer(port, pEventloop);
    _ptcp_sv->set_usr(this);
    _pEventloop = pEventloop;
}

EchoServer::~EchoServer(){
    delete _ptcp_sv;
}

void EchoServer::Start(){ _ptcp_sv->Start(); }

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
