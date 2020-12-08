#include "declare.h"

void perror(std::string msg, int fd){
    std::cout << "fd = " << fd << "\t" << msg << "\t" << strerror(errno) << std::endl;
    exit(0);
}
