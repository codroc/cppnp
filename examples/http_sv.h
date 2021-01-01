#ifndef CPPNP_HTTPSERVER_H_
#define CPPNP_HTTPSERVER_H_

#include <string>
#include <map>
#include "i_cppnp_usr.h"
#include "i_run.h"
#include "declare.h"
#include "tcp_connection.h"
#include "thread_pool.h"
#define SERVER_STRING "Server: cppnp/v0.10\r\n"
#define WWW_PATH  "/usr/local/www"
//#define MUTITHREAD

#define STDIN 0
#define STDOUT 1
#define STDERR 2

class Eventloop;
class Buffer;
class TcpConnection;
class TcpServer;
class HttpConnection;

class HttpServer: public ICppnpUsr, 
    public IRun0,
    public IRun2
{
public:
    HttpServer(Eventloop*, unsigned short);
    ~HttpServer();

    void Start();

    const string Core(HttpConnection*, const std::string&);

    bool isKeepAlive(const string &);

    virtual void OnConnection(TcpConnection *);
    virtual void OnMessage(TcpConnection *, Buffer *); 
    virtual void OnWriteComplete(TcpConnection *);

    virtual void run0();
    virtual void run2(const string&, void*);
private:
    TcpServer *_ptcp_sv;
    Eventloop *_pEventloop;
    ThreadPool _threadpool;

    int64_t _timer;
    int _index;

private:
    std::string::size_type SplitString(const std::string &, std::string::size_type, const std::string &, std::string &);
    const std::string unimplemented();
    const std::string badrequest();
    const std::string cannot_execute();
    const std::string not_found();
    const std::string execute_cgi(const std::string &, const std::string &, const std::string &, const std::string &);
    const std::string serve_file(const char *, std::string &);
    const std::string cat(FILE *);
public:
    static std::map<TcpConnection*, HttpConnection*> _HttpConnections;
};

class HttpConnection : public IRun0{
public:
    HttpConnection(TcpConnection *pConn) :
        _pConn(pConn),
        _httpProtocalVersion(-1),
        _islongconnection(false),
        _connectionCount(0),
        _isOnTiming(false)
    {}
    ~HttpConnection(){}

    void Send(const string &response){
        _pConn->Send(response);
    }
    void setLongConnection(){ _islongconnection = true; }
    void setShortConnection() { _islongconnection = false; }
    bool isLongConnection() { return _islongconnection == true; }

    void addCount() { _connectionCount++; }
    void delCount() { _connectionCount = 0; }
    int Count() { return _connectionCount; }

    bool isOnTiming() { return _isOnTiming == true; }
    bool disOnTiming() { _isOnTiming = false; }
    bool enableOnTiming() { _isOnTiming = true; }
    
    void setHttpProtocalVersion(int v) { _httpProtocalVersion = v; }
    int httpProtocalVersion() { return _httpProtocalVersion; }

    virtual void run0();

    void closeConnection() {
        _pConn->closeConnection();
        printf("Connection Closed!\n");
    }

    void setpEventloop(Eventloop *pEventloop) { _pEventloop = pEventloop; }

private:
    Eventloop *_pEventloop;
    TcpConnection *_pConn;
    int _httpProtocalVersion;// -1 表示初始化状态  0：HTTP/1.0  1：HTTP/1.1
    bool _islongconnection;

    int _connectionCount;
    bool _isOnTiming;
};

#endif // CPPNP_HTTPSERVER_H_
