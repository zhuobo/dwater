# Web server dwater
![logo](./screenshots/logo.png)
dwater is a high-performance multi-threaded network library based on Reactor mode.

## 技术特点
* 非阻塞 I/O + I/O 复用的 Reactor 模式
* 异步日志，实时记录服务器程序运行状态
* TCP 连接缓冲区，简化网络库使用者对接受和发送数据的操作
* 使用多线程技术、并使用线程池避免频繁创建销毁线程带来的开销
* 使用智能指针、RAII 机制减少程序可能出现的内存泄漏
