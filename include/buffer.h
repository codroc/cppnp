#ifndef CPPNP_BUFFER_H_
#define CPPNP_BUFFER_H_ 

class Buffer{
public:
    Buffer();
    ~Buffer();

    bool IsEmpty();
    const char* str();
    void Append(const char*, int);
    int Write(int);
    int len();
private:
    int _len;
    char *_pBuf;
};

#endif // CPPNP_BUFFER_H_
