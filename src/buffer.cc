#include "buffer.h"
#include <errno.h>
#include <string.h>
#include "current_thread.h"
#include "declare.h"
using namespace std;
Buffer::Buffer(){}
Buffer::~Buffer(){}

const char* Buffer::str(){ return _buf.c_str(); }
bool Buffer::IsEmpty() { return _buf.size() == 0; }
void Buffer::Append(const string &str){
    _buf.append(str);
}
int Buffer::Write(int fd){
    int num = ::write(fd, _buf.c_str(), _buf.size());
    if(num < 0){
        printf("num = %d :%s\n", num, strerror(errno));
    }
    else{
        if(num > _buf.size())   printf("num > _buf.size()\n");
        else
            _buf = _buf.substr(num, _buf.size());
    }
    return num;
}
string Buffer::ReadAsString(){
    string s = string(_buf.c_str(), len());
    _buf = _buf.substr(_buf.size(), _buf.size());
    return s;
}
int Buffer::len(){ return _buf.size(); }
