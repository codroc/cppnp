#include "echo_sv.h"
#include "eventloop.h"
#include "tcp_connection.h"
#include "tcp_server.h"

EchoServer::EchoServer(Eventloop *pEventloop, unsigned short port=80){
    _ptcp_sv = new TcpServer(port, pEventloop);
    _pEventloop = pEventloop;
    _pEventloop->set_connection_usr(this);
}

EchoServer::~EchoServer(){
    delete _ptcp_sv;
}

void EchoServer::Start(){ _ptcp_sv->Start(); }

void EchoServer::OnConnection(TcpConnection*){
    cout << "OnConnection!\n";
}
const string EchoServer::Core(const string &data){  
    string response;
    response = data;
    return response;
}
void EchoServer::OnMessage(TcpConnection *pConn, int fd, const string &data){
    cout << "OnMessage\n";
    const string response = Core(data);
    pConn->Send(fd, response);
}
