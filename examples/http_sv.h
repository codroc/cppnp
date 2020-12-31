#ifndef CPPNP_HTTPSERVER_H_
#define CPPNP_HTTPSERVER_H_

#include "i_cppnp_usr.h"
#include "i_run.h"
#include "declare.h"
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

class HttpServer: public ICppnpUsr, 
    public IRun0,
    public IRun2
{
public:
    HttpServer(Eventloop*, unsigned short);
    ~HttpServer();

    void Start();

    const string Core(const std::string&);

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
};

#endif // CPPNP_HTTPSERVER_H_
