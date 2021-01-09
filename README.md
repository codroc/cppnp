在之前的版本中 TcpConnection 对象的析构不是一件容易的事，因为在多线程场景下会引发 race condition，某个线程在析构 TcpConnection 对象时，很难保证其他线程没有正在使用该对象。如果其他线程正在使用该对象而你去析构它，会导致无定义的行为，最好的结果就是 core dump。

本版本引入智能指针：shared_ptr<T> 和 weak_ptr<T> 来解决这一问题。同时删去之前为了实现多线程场景下的 TcpConnection 析构而引入的粗糙的代码。

