#include "communicator.h"
#include "declare.h"
#include <map>
using namespace std;
Communicator::Communicator(Eventloop *pEventloop) :
    _pEventloop(pEventloop), _pmp(NULL)
{}

Communicator::~Communicator(){}

/* Communicator 处理读写业务 */
void Communicator::Method(int fd){
    std::cout << "fd = " << fd << std::endl;
    if(fd < 0){
        std::cout << "fd < 0 error!\n";
        return;
    }
    int readnum;
    const int kMaxBufSize = 1500;
    char buf[kMaxBufSize];
    memset(buf, 0, sizeof(buf));

    if((readnum = read(fd, buf, kMaxBufSize)) < 0){
        std::cout << "readnum < 0 error!\n";
        map<int,Channel*>::iterator it = (*_pmp).find(fd);
        if((*_pmp).end() != it){
            _pEventloop->Update(it->second, EP_DEL);
            delete it->second;
            (*_pmp).erase(it);
        }
        close(fd);
    }
    else if(readnum == 0){
        std::cout << "has read the end of the file!\n";
        map<int,Channel*>::iterator it = (*_pmp).find(fd);
        if((*_pmp).end() != it){
            _pEventloop->Update(it->second, EP_DEL);
            delete it->second;
            (*_pmp).erase(it);
        }
        close(fd);
    }
    else{
        if(write(fd, buf, readnum) != readnum)
            std::cout << "not finished at one time!\n";
    }
}
