# Version 0.02: 在 Version 0.01 的基础上用 C++ 改写

引入 Channel 类将 socket 给封装起来，并将与 socket 相关的方法放入类中。
现在要管理 Channel 的生命周期了，这次改写导致了内存泄漏，之后再解决。

下个 Version 的目标是将接受连接和读写解耦出来。
