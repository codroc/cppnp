# 

将原来 **Channel** 对 *Event* 的处理方法进行细分， **Acceptor** 用来专门接受新的连接；**Communicator** 用来在建立的连接上进行通信。


思路是这样的：

1. 构建一个 TcpServer 类，该类拥有 Start 方法，在 main 函数中只需要执行该方法就可以启动服务器。
2. 每个 TcpServer 对象拥有一个 Acceptor(即对 listenfd 进行封装)，专门用来处理新的连接。
3. 每当 epoll_wait 返回，就循环处理每个事件（新的连接或读写），通过 Channel 的 HandleEvent 接口来处理事件(实现多态)。

整个代码的框架如下图所示：

![]()
