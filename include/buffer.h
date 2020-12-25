#ifndef CPPNP_BUFFER_H_
#define CPPNP_BUFFER_H_ 
#include <string>
#include "mutex.h"
class Buffer{
public:
    Buffer();
    ~Buffer();

    bool IsEmpty();
    const char* str();
    void Append(const std::string&);
    int Write(int);
    std::string ReadAsString();
    int len();
private:
//    Mutex _mutex;
    std::string _buf;
};

#endif // CPPNP_BUFFER_H_
