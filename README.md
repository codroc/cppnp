# Version 0.06: 

这个版本将实现 Buffer 类，用于应用层面的缓冲。并且修改了 TcpConnection 类和实现。现在每一个用于通信的 TCP socket 都会包含在 TcpConnection 中，每个 TcpConnection 都会拥有一个 InputBuffer 和一个 OutputBuffer。

那么为什么需要缓冲呢？

首先将缓冲分为两类：InputBuffer 和 OutputBuffer

为什么需要 InputBuffer ？
TCP 是面向连接可靠的字节流协议，因为是字节流协议，所以没有消息边界。那么服务端收到的消息可能是2个或以上的 TCP 分节（TCP segment），也有可能连一个 TCP 分节也没有。接收方必须要处理“收到的数据尚不构成一条完整的消息”和“一次收到两条或以上消息的数据”等情况。那么服务器该怎么做呢？首先不管 TCP 分节到底发了几个，先 read 把能读的数据都读到应用层面的缓冲当中（将数据从 OS Buffer 搬到应用层 Buffer）否则会出发 EPOLLIN 事件，造成 busy-loop。那么网络库必定要应对“数据不完整”的情况，收到的数据先放到 InputBuffer 中，等构成一条完整的消息之后再通知程序的业务逻辑。这通常是 codec（编解码器） 的职责。所以在 TCP 网络编程中，网络库必须要给每个 TCP connection 配置 InputBuffer。

为什么需要 OutputBuffer ？

考虑一个场景：程序想通过 TCP 连接发送 100kB 数据，但是在 write() 调用中，OS 只接受了 80kB （受 TCP advertised windows 的控制），你肯定不想在原地等待，因为不知道会等多久（取决于对方什么时候接收数据，然后滑动 TCP 窗口）。程序应该尽快交出控制权，返回到 eventloop。这种情况下，剩余的 20kB 数据怎么办？

对于应用程序而言，它只管产生数据，它不应该关心到底数据是一次性发送还是分几次发送，这应该是由网络库来操心，程序只要调用 TcpConnection::Send() 就行了，网络库会负责到底。网络库应该接管剩余的 20kB 数据，把它保存在该 TCP connection 的 OutputBuffer 中，然后注册 EPOLLOUT 事件，一旦 socket 变得可写就立刻发送数据。当然，这第二次 write() 也不一定能完全写入 20kB，如果有剩余，网络库应该继续关注 EPOLLOUT 事件；如果写完了 20kB 数据，网络库应该停止关注 EPOLLOUT 事件，以免造成 busy-loop。所以必须要用 OutputBuffer。

引入了应用层缓冲，就不得不考虑要用多大的缓冲。
一方面我们希望减少系统调用，一次读的数据越多越划算，那么似乎应该准备一个大的缓冲区。另一方面希望减少内存的占用。如果有 10000 个并发连接，每个连接一建立就分配各 50kB 的读写缓冲区的话，将占用 1GB 的内存，而大多数时候这些缓冲区的使用率很低。

这个版本就简单地实现了一下缓冲区，并没有考虑到效率问题，下个版本或许可以改进。
