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
#include <cstdio>
using namespace std;

HttpServer::HttpServer(Eventloop *pEventloop, unsigned short port=80){
    _ptcp_sv = new TcpServer(port, pEventloop);
    _ptcp_sv->set_usr(this);
    _pEventloop = pEventloop;
    _timer = -1;
    _index = 0;
}

HttpServer::~HttpServer(){
    delete _ptcp_sv;// new TcpServer
}

void HttpServer::Start(){ 
    _ptcp_sv->Start(); 
#ifdef MUTITHREAD
    _threadpool.Start(3);
#endif
}

void HttpServer::OnConnection(TcpConnection*){
    printf("OnConnection!\n");
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

string::size_type HttpServer::SplitString(const string &request, string::size_type pos_s, const string &split_char, string &ret_str){
    if(pos_s == string::npos) return string::npos;
    string::size_type pos_e = request.find(split_char, pos_s);
    if(string::npos != pos_e)
        ret_str = request.substr(pos_s, pos_e - pos_s);
    return pos_e;
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
        sprintf(length_env, "CONTENT_LENGTH=%d", query_string.size());
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
const string HttpServer::Core(const string &request){  
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

void HttpServer::OnMessage(TcpConnection *pConn, Buffer *buf){
    printf("OnMessage\n");
    string request = buf->ReadAsString();
#ifdef MUTITHREAD
    Task task(this, request, pConn);
    _threadpool.AddTask(task);
#else
    const string response = Core(request);
    pConn->Send(response);
#endif
}
void HttpServer::OnWriteComplete(TcpConnection *pConn){
    printf("WriteCompleted!\n");
}
void HttpServer::run0(){
    printf("_index = %d\n", _index);
    _index++;
    if(_index >= 1000){
        _pEventloop->cancelTimer(_timer);
        _index = 0;
        _pEventloop->Quit();
    }
}
void HttpServer::run2(const string &request, void *pCon){
    const string response = Core(request);
    static_cast<TcpConnection*>(pCon)->Send(response);
}
