# Version 0.01：使用 linux API (epoll) 实现一个 echo toy.

#

## UNIX 网络编程基础介绍：

&emsp;&emsp;**个人认为，网络编程的本质还是进程间通信，只是通信区域跨越了一个网络罢了，通信的方式是使用套接字。**

*重要的数据结构:*

```c++
struct sockaddr_in{
    short int sin_family;           // 1. AF_INET    2. PF_INET
    unsigned short int sin_port;    // 端口号
    struct in_addr sin_addr;        // ip 地址
    unsigned char sin_zero[8];      // 无意义
};

struct in_addr{
    in_addr_t s_addr;
};
```

*服务器端的 API 调用：*

socket API 用于创建一个套接字，这个套接字常常是监听套接字

```c++
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
        Return fd on success, or -1 on error
```

bind API 用于将 socket 产生的套接字与服务器 ip:port 进行绑定

```c++
#include <sys/socket.h>

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
        Return 0 on success, or -1 on error
```

listen API 用于指定监听队列的长度，因为考虑到大量连接请求的情况，会有大量请求到达 listenfd，此时要求确定一个队列长度

```c++
#include <sys/socket.h>

int listen(int sockfd, int backlog);
        Return 0 on success, or -1 on error 
```

accept API 用于产生一个服务套接字

```c++
#include <sys/socket.h>
int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
        Return fd on success, or -1 on error 
```

其中 addr 指向的是一个需要由 accept API 填写的结构，表明了客户端的 ip:port 

#


## linux API: epoll

epoll - I/O event notification facility

epoll (event poll) 可以检查多个文件描述符上的 I/O 就绪状态。epoll API 的主要优点如下：
- 当检查大量的文件描述符时，epoll 性能的延展性比 select 和 poll 高很多
- epoll API 既支持水平触发又支持边缘触发。select 和 poll 仅支持水平触发而 信号驱动I/O 仅支持边缘触发

性能表现上 信号驱动I/O 与 epoll 相近，但是 epoll 具有以下 信号驱动I/O 不具备的优势：
- 可以避免复杂的信号处理流程（比如信号队列溢出时的处理流程，因为实时信号可以排队，但队长有限）
- 灵活性高，可以指定我们希望检查的事件（例如检查文件描述符的读就绪，或写就绪，或两者都检查）

epoll 一共就 3 个接口：
- 1. int epoll_create(int size);
- 2. int epoll_ctl(int epfd, int op, int fd, struct epoll_event *ev);
- 3. int epoll_wait(int epfd, struct epoll_event *evlist, int maxevents, int timeout);

```c++
#include <sys/epoll.h>

int epoll_create(int size);
        Return fd on success, or -1 on error 

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *ev);
        Return 0 on success, or -1 on error 

int epoll_wait(int epfd, struct epoll_event *evlist, int maxevents, int timeout);
        Return number of ready fd, 0 on timeout, or -1 on error 
```

1. int epoll_create(int size);

创建一个epoll的句柄，size用来告诉内核这个监听的数目一共有多大。这个参数不同于select()中的第一个参数，给出最大监听的fd+1的值。需要注意的是，当创建好epoll句柄后，它就是会占用一个fd值，在linux下如果查看/proc/进程id/fd/，是能够看到这个fd的，所以在使用完epoll后，必须调用close()关闭，否则可能导致fd被耗尽。

注意：size参数只是告诉内核这个 epoll对象会处理的事件大致数目，而不是能够处理的事件的最大个数。在 Linux最新的一些内核版本的实现中，这个 size参数没有任何意义。

2. int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);

epoll的事件注册函数，epoll_ctl向 epoll对象中添加、修改或者删除感兴趣的事件，epoll_wait方法返回的事件必然是通过 epoll_ctl添加到 epoll中的。

第一个参数是epoll_create()的返回值，第二个参数表示动作，用三个宏来表示：

EPOLL_CTL_ADD：注册新的fd到epfd中；

EPOLL_CTL_MOD：修改已经注册的fd的监听事件；

EPOLL_CTL_DEL：从epfd中删除一个fd；


第三个参数是需要监听的fd，第四个参数是告诉内核需要监听什么事，struct epoll_event结构如下：
```c++
typedef union epoll_data {
    void *ptr;
    int fd;
    __uint32_t u32;
    __uint64_t u64;
} epoll_data_t;

struct epoll_event {
    __uint32_t events; /* Epoll events */
    epoll_data_t data; /* User data variable */
```

events可以是以下几个宏的集合：

EPOLLIN ：表示对应的文件描述符可以读（包括对端SOCKET正常关闭）；

EPOLLOUT：表示对应的文件描述符可以写；

EPOLLPRI：表示对应的文件描述符有紧急的数据可读（这里应该表示有带外数据到来）；

EPOLLERR：表示对应的文件描述符发生错误；

EPOLLHUP：表示对应的文件描述符被挂断；

EPOLLET： 将EPOLL设为边缘触发(Edge Triggered)模式，这是相对于水平触发(Level Triggered)来说的。

EPOLLONESHOT：只监听一次事件，当监听完这次事件之后，如果还需要继续监听这个socket的话，需要再次把这个socket加入到EPOLL队列里


3. int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout);

等待事件的产生，类似于select()调用。参数events用来从内核得到事件的集合，maxevents告之内核这个events有多大，这个 maxevents的值不能大于创建epoll_create()时的size，参数timeout是超时时间（毫秒，0会立即返回，-1将不确定，也有说法说是永久阻塞）。

第1个参数 epfd是 epoll的描述符。

第2个参数 events则是分配好的 epoll_event结构体数组，epoll将会把发生的事件复制到 events数组中（events不可以是空指针，内核只负责把数据复制到这个 events数组中，不会去帮助我们在用户态中分配内存。内核这种做法效率很高）。

第3个参数 maxevents表示本次可以返回的最大事件数目，通常 maxevents参数与预分配的events数组的大小是相等的。

第4个参数 timeout表示在没有检测到事件发生时最多等待的时间（单位为毫秒），如果 timeout为0，则表示 epoll_wait在 rdllist链表中为空，立刻返回，不会等待。


#

##3. int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout);

等待事件的产生，类似于select()调用。参数events用来从内核得到事件的集合，maxevents告之内核这个events有多大，这个 maxevents的值不能大于创建epoll_create()时的size，参数timeout是超时时间（毫秒，0会立即返回，-1将不确定，也有说法说是永久阻塞）。

第1个参数 epfd是 epoll的描述符。

第2个参数 events则是分配好的 epoll_event结构体数组，epoll将会把发生的事件复制到 events数组中（events不可以是空指针，内核只负责把数据复制到这个 events数组中，不会去帮助我们在用户态中分配内存。内核这种做法效率很高）。

第3个参数 maxevents表示本次可以返回的最大事件数目，通常 maxevents参数与预分配的events数组的大小是相等的。

第4个参数 timeout表示在没有检测到事件发生时最多等待的时间（单位为毫秒），如果 timeout为0，则表示 epoll_wait在 rdllist链表中为空，立刻返回，不会等待。


#

##3. int epoll_wait(int epfd, struct epoll_event * events, int maxevents, int timeout);

等待事件的产生，类似于select()调用。参数events用来从内核得到事件的集合，maxevents告之内核这个events有多大，这个 maxevents的值不能大于创建epoll_create()时的size，参数timeout是超时时间（毫秒，0会立即返回，-1将不确定，也有说法说是永久阻塞）。

第1个参数 epfd是 epoll的描述符。

第2个参数 events则是分配好的 epoll_event结构体数组，epoll将会把发生的事件复制到 events数组中（events不可以是空指针，内核只负责把数据复制到这个 events数组中，不会去帮助我们在用户态中分配内存。内核这种做法效率很高）。

第3个参数 maxevents表示本次可以返回的最大事件数目，通常 maxevents参数与预分配的events数组的大小是相等的。

第4个参数 timeout表示在没有检测到事件发生时最多等待的时间（单位为毫秒），如果 timeout为0，则表示 epoll_wait在 rdllist链表中为空，立刻返回，不会等待。


#

## 另外一些在网络编程时用到的函数

- char* inet_ntoa(struct in_addr);
- in_addr_t inet_addr(const char *cp);
- uint32_t htonl(uint32_t hostlong);
- uint16_t htons(uint16_t hostshort);
- int fcntl(int fd, int cmd, ... /* arg */ );

&emsp;&emsp;The  htonl function converts the unsigned int **hostlong** from host byte order to network byte order.
&emsp;&emsp;The  htons function converts the unsigned int **hostshort** from host byte order to network byte order.
&emsp;&emsp;The  inet_ntoa function 将返回一个点分十进制的 ip 地址字符串，该字符串保存在静态内存中，即初始化数据段中。
&emsp;&emsp;The  inet_addr function 将会将点分十进制的 ip 地址字符串转换成无符号长整形 (in_addr_t)。
&emsp;&emsp;The  fcntl function performs one of the operations on the open file descriptor fd.  The operation is determined by cmd. 具体请看 man 手册。

#

## 程序总体框架设计

1. 将 socket、bind、listen 放在一个函数里面，用于创建一个监听套接字。

2. 通过 epoll 来不断查询是否有可以进行 I/O 的套接字。

3. 当监听套接字读就绪时，说明有连接接入，则通过 accept 产生一个服务套接字来对连接进行服务。

4. 当服务套接字读写就绪时，进行相应的 I/O 操作

![](/home/codroc/图片/pic1.png)


