#include "buffer.h"

#include "declare.h"
Buffer::Buffer(){
    _len = 0;
    _pBuf = new char[kMaxAppBufSize];
    memset(_pBuf, 0, kMaxAppBufSize);
}
Buffer::~Buffer(){
    delete _pBuf;
}

const char* Buffer::str(){ return _pBuf; }
bool Buffer::IsEmpty() { return _len == 0; }
void Buffer::Append(const char *p, int n){
    memcpy(_pBuf + _len, p, n);
    _len += n;
}
int Buffer::Write(int fd){
    int num = ::write(fd, _pBuf, _len);
    _len -= num;
    memcpy(_pBuf, _pBuf + num, _len);
    return num;
}

int Buffer::len(){ return _len; }
