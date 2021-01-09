#include "http_sv.h"
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "current_thread.h"
#include "eventloop.h"
#include "tcp_connection.h"
#include "tcp_server.h"
#include "buffer.h"
#include "task.h"
#include "timestamp.h"
#include <cstdio>
#include <memory>
using namespace std;

bool operator<(const weak_ptr<TcpConnection> &lhs, const weak_ptr<TcpConnection> &rhs){
    return lhs.lock().get() < rhs.lock().get();
}
map<weak_ptr<TcpConnection>, shared_ptr<HttpConnection>> HttpServer::_HttpConnections;

HttpServer::HttpServer(Eventloop *pEventloop, unsigned short port=80){
    _ptcp_sv = new TcpServer(port, pEventloop);
    _ptcp_sv->set_usr(this);
    _pEventloop = pEventloop;
    _timer = -1;
    _index = 0;
    shared_ptr<HttpServer> local_sp(this);
    _sp_self = local_sp;
}

HttpServer::~HttpServer(){
    delete _ptcp_sv;// new TcpServer
}

void HttpServer::Start(){ 
    _ptcp_sv->Start(); 
    _pEventloop->runEvery(1, this);
#ifdef MUTITHREAD
    _threadpool.Start(1);
#endif
}

void HttpServer::OnConnection(weak_ptr<TcpConnection>){
    //printf("OnConnection!\n");
}


const string HttpServer::unimplemented()
{
    char buf[1024];
    string response;

    sprintf(buf, "HTTP/1.0 501 Method Not Implemented\r\n");
    response.append(buf);
    sprintf(buf, SERVER_STRING);
    response.append(buf);
    sprintf(buf, "Content-Type: text/html\r\n");
    response.append(buf);
    sprintf(buf, "\r\n");
    response.append(buf);
    sprintf(buf, "<HTML><HEAD><TITLE>Method Not Implemented\r\n");
    response.append(buf);
    sprintf(buf, "</TITLE></HEAD>\r\n");
    response.append(buf);
    sprintf(buf, "<BODY><P>HTTP request method not supported.\r\n");
    response.append(buf);
    sprintf(buf, "</BODY></HTML>\r\n");
    response.append(buf);

    return response;
}

const string HttpServer::badrequest(){
    char buf[1024];
    string response;

    sprintf(buf, "HTTP/1.0 400 Bad Request\r\n");
    response.append(buf);
    sprintf(buf, SERVER_STRING);
    response.append(buf);
    sprintf(buf, "Content-Type: text/html\r\n");
    response.append(buf);
    sprintf(buf, "\r\n");
    response.append(buf);
    sprintf(buf, "<HTML><HEAD><TITLE>Bad Request!\r\n");
    response.append(buf);
    sprintf(buf, "</TITLE></HEAD>\r\n");
    response.append(buf);
    sprintf(buf, "<BODY><P>HTTP protocal not supported.\r\n");
    response.append(buf);
    sprintf(buf, "</BODY></HTML>\r\n");
    response.append(buf);

    return response;
}

const string HttpServer::not_found()
{
    char buf[1024];
    string response;

    sprintf(buf, "HTTP/1.0 404 NOT FOUND\r\n");
    response.append(buf);
    sprintf(buf, SERVER_STRING);
    response.append(buf);
    sprintf(buf, "Content-Type: text/html\r\n");
    response.append(buf);
    sprintf(buf, "\r\n");
    response.append(buf);
    sprintf(buf, "<HTML><TITLE>Not Found</TITLE>\r\n");
    response.append(buf);
    sprintf(buf, "<BODY><P>The server could not fulfill\r\n");
    response.append(buf);
    sprintf(buf, "your request because the resource specified\r\n");
    response.append(buf);
    sprintf(buf, "is unavailable or nonexistent.\r\n");
    response.append(buf);
    sprintf(buf, "</BODY></HTML>\r\n");
    response.append(buf);

    return response;
}
const string HttpServer::cannot_execute()
{
    char buf[1024];
    string response;

    sprintf(buf, "HTTP/1.0 500 Internal Server Error\r\n");
    response.append(buf);
    sprintf(buf, "Content-type: text/html\r\n");
    response.append(buf);
    sprintf(buf, "\r\n");
    response.append(buf);
    sprintf(buf, "<P>Error prohibited CGI execution.\r\n");
    response.append(buf);

    return response;
}

string::size_type HttpServer::SplitString(const string &request, string::size_type pos_s, const string &split_char, string &ret_str){
    if(pos_s == string::npos) return string::npos;
    string::size_type pos_e = request.find(split_char, pos_s);
    if(string::npos != pos_e)
        ret_str = request.substr(pos_s, pos_e - pos_s);
    return pos_e;
}

const string HttpServer::execute_cgi(const string &path, const string &method, const string &httpV, const string &query_string){
    string response;
    response.append(httpV);
    response.append(" 200 OK\r\n");
    response.append(SERVER_STRING);
    response.append("Content-Type: text/html\r\n");
    response.append("\r\n");

    pid_t pid;

    int cgi_input[2];
    int cgi_output[2];
    if(pipe(cgi_input) == -1)
        return cannot_execute();
    if(pipe(cgi_output) == -1)
        return cannot_execute();
    if((pid = fork()) < 0)
        return cannot_execute();
     
    if(pid == 0)// child
    {
        char method_env[255];
        char query_env[255];
        char length_env[255];

        dup2(cgi_input[0], STDIN);
        dup2(cgi_output[1], STDOUT);
        close(cgi_input[1]);
        close(cgi_output[0]);

        sprintf(method_env, "REQUEST_METHOD=%s", method.c_str());
        sprintf(query_env, "QUERY_STRING=%s", query_string.c_str());
        sprintf(length_env, "CONTENT_LENGTH=%ld", query_string.size());
        putenv(method_env);
        putenv(query_env);
        putenv(length_env);

        execl(path.c_str(), NULL);
        exit(0);
    }
    else{// father
        int status;
        close(cgi_input[0]);
        close(cgi_output[1]);

        char buf[1024];
        int num;
        while((num = read(cgi_output[0], buf, 1023)) > 0){
            string s(buf);
            response.append(s);
        }

        close(cgi_input[1]);
        close(cgi_output[0]);

        waitpid(pid, &status, 0);
    }
    return response;
}
const string HttpServer::serve_file(const char *filename, string &httpV)
{
    string response;
    FILE *resource = NULL;
    char buf[1024];

    resource = fopen(filename, "r");
    if (resource == NULL)
        return not_found();
    else
    {
        response.append(httpV);
        response.append(" 200 OK\r\n");
        response.append(SERVER_STRING);
        response.append("Content-Type: text/html\r\n");
        response.append("\r\n");
        response.append(cat(resource));
    }
    fclose(resource);
    return response;
}
const string HttpServer::cat(FILE *resource)
{
    char buf[1024];
    string response;
    fgets(buf, sizeof(buf), resource);
    while (!feof(resource))
    {
        response.append(buf);
        fgets(buf, sizeof(buf), resource);
    }

    return response;
}

// 子线程会来执行 Core 
const string HttpServer::Core(HttpConnection *HttpConn, const string &request){  
    struct stat st;
    int cgi = 0;
    string path(WWW_PATH);
    string response;
    string method, url, query_string, httpV;
    string::size_type pos_s = 0;
    string::size_type pos_e = SplitString(request, pos_s, " ", method);

    if(method != "GET" && method != "POST")
        return unimplemented();

    if(method == "POST")
        cgi = 1;

    pos_s = pos_e + 1;
    pos_e = SplitString(request, pos_s, " ", url);

    if(method == "GET"){
        string::size_type pos = url.find("?");
        if(string::npos != pos){
            cgi = 1;
            query_string = url.substr(pos + 1, url.size() - pos - 2);
            url = url.substr(0, pos);
        }
    }

    pos_s = pos_e + 1;
    pos_e = SplitString(request, pos_s, "\r\n", httpV);
    if(httpV != "HTTP/1.0" && httpV != "HTTP/1.1"){
        printf("badreques: %s\n", httpV.c_str());
        return badrequest();
    }
    else if(httpV == "HTTP/1.0"){
        HttpConn->setHttpProtocalVersion(0);
    // 查看 Connection 字段，如果存在且为 keep-alive 则设置成长链接
        if(isKeepAlive(request))
            HttpConn->setLongConnection();
        else
            HttpConn->setShortConnection();
    }
    else{// HTTP/1.1
        HttpConn->setHttpProtocalVersion(1);
        HttpConn->setLongConnection();
    }

    if(method == "POST"){
        int length = 0;
        string tmp;
        string len;
        string::size_type pos1, pos2;
        pos1 = SplitString(request, 0, "Content-Length:", tmp);
        pos2 = SplitString(request, pos1 + 15, "\r\n", len);
        length = atoi(len.c_str());

        while(pos1 != pos2){
            pos1 = pos2 + 2;
            pos2 = SplitString(request, pos1, "\r\n", tmp);
        }
        pos1 += 2;
        query_string = request.substr(pos1, request.size() - pos1 - 1);
    }
    

    path.append(url);
    if(path[path.size() - 1] == '/')
        path.append("html/index.html");
    if(stat(path.c_str(), &st) == -1)
        return not_found();
    else{
        if ((st.st_mode & S_IFMT) == S_IFDIR)
            path.append("html/index.html");
        if ((st.st_mode & S_IXUSR) ||
                (st.st_mode & S_IXGRP) ||
                (st.st_mode & S_IXOTH)    )
            cgi = 1;
        if (!cgi)
            return serve_file(path.c_str(), httpV);
        else
            return execute_cgi(path, method, httpV, query_string);
    }
}

void HttpServer::OnMessage(weak_ptr<TcpConnection> pConn, Buffer *buf){
    //printf("OnMessage\n");
    map<weak_ptr<TcpConnection>, shared_ptr<HttpConnection>>::iterator it = _HttpConnections.find(pConn);
    shared_ptr<HttpConnection> pHttpConn;
    if(it == _HttpConnections.end()){
        pHttpConn = make_shared<HttpConnection>(pConn);// 相当于 new HttpConnection
        pHttpConn->setpEventloop(_pEventloop);
        _HttpConnections.insert(pair<weak_ptr<TcpConnection>, shared_ptr<HttpConnection>> (pConn, pHttpConn));
    }
    else pHttpConn = it->second;
    pHttpConn->addCount();// 请求数 +1 
    string request = buf->ReadAsString();
#ifdef MUTITHREAD
    Task task(weak_ptr<HttpServer>(_sp_self), request, weak_ptr<HttpConnection>(pHttpConn)); // 子线程会去执行 HttpServer::run2()  第三个参数是执行 HttpConnection 的指针
    _threadpool.AddTask(task);
#else
    const string response = Core(pHttpConn.get(), request);
    pHttpConn->Send(response);
#endif
}
// 当一个相应发送完毕时，肯定会来调用 OnWriteComplete，此时如果是长连接则设置定时器，如果时短链接则断开。
void HttpServer::OnWriteComplete(weak_ptr<TcpConnection> pConn){
//    printf("WriteCompleted!\n");
    map<weak_ptr<TcpConnection>, shared_ptr<HttpConnection>>::iterator it = _HttpConnections.find(pConn);
    shared_ptr<HttpConnection> pHttpConn = it->second;
    if(!pHttpConn->isOnTiming() && pHttpConn->isLongConnection()){// 如果是长链接并且没在计时
//        printf("set 定时器\n");
        _pEventloop->runAfter(3, pHttpConn.get());
    }
    else if(!pHttpConn->isLongConnection()){
        pHttpConn->closeConnection();
        _HttpConnections.erase(it);
    }
}// 如果执行了 closeConnection 那么出这个 scope 时 HttpConnection 就会自动销毁, 此时 TcpConnection 已经销毁了！

void HttpServer::run0(){
    //printf("_index = %d\n", _index);
    _index++;
    if(_index >= 1000){
        _pEventloop->cancelTimer(_timer);
        _index = 0;
        _pEventloop->Quit();
    }
}
void HttpServer::run2(const string &request, void *pConn){
    HttpConnection *HttpConn = static_cast<HttpConnection*>(pConn);
    const string response = Core(HttpConn, request);
    HttpConn->Send(response);
}

bool HttpServer::isKeepAlive(const string &request){
    string::size_type pos = request.find("Connection:");
    if(string::npos == pos) return false;
    pos = request.find("keep-alive", pos + 1);
    if(string::npos == pos) return false;
    else return true;
}

// 定时器到时来执行, 此时对方可能已经断开连接了
void HttpConnection::run0(){
//    printf("http 定时器\n");
    disOnTiming();
    if(_pConn.expired() || Count() == 0){
        map<weak_ptr<TcpConnection>, shared_ptr<HttpConnection>>::iterator it = HttpServer::_HttpConnections.find(_pConn);
        closeConnection();// 会判断 TcpConnection 对象是否还存在 
        HttpServer::_HttpConnections.erase(it);
    }
    else{
        delCount();
        _pEventloop->runAfter(3, this);
        enableOnTiming();
    }
}
