* async_logging.cc, async_logging.h:
  * 实现异步日志，内部定义一个两个buffer，以及一个buffervector，一个后台前程专门IO，将buffervector中的数据写入到指定的文件。前台线程是调用INFO_XXX的线程，用于写入到LogStream中的buffer 
* atomic.h
  * 源于原子操作，定义了两个整数32位、64位
* blocking_queue.h
  * 生产者消费者队列，用std::deque实现，无界的，操作的时候用mutex保护，完成操作使用cond通知别的线程
* bounded_blocking_queue.h
  * 生产者消费者队列，用boost::circuit循环队列实现，类似的线程安全保护 
* CMakeLists.txt
* condition.cc, condition.h
  * 条件变量，和mutex一起使用
* copyable.h
* count_down_latch.cc, count_down_latch.h
  * 倒计时器
* current_thread.cc, current_thread.h
  * 当前线程的封装
* date.cc
* date.h
* do.py
* exception.cc
* exception.h
* file_util.cc, file_util.h
  * 
* log_file.cc
* log_file.h
* logging.cc, logging.h
  * Logger日志，搭配AsyncLogging成为异步日志
* log_stream.cc, log_stream.h
  * 内含一个buffer，存储日志数据，重载了很多operator<<，Logger的一个成员。
* mutex.h
  * 互斥锁，raii技法的使用，使用一个栈上的MutexLockGuard的自动销毁实现自动释放锁
* noncopable.h
  * 一个空的基类，copy constructor和operator=被delete，无法被复制
* process_info.cc, process_info.h
  * 进程相关信息
* singleton.h
  * 线程安全的Singleton
* string_piece.h
  * 优化字符串常量作为函数参数的时候构造string产生的开销
* thread.cc, thread.h
  * 封装了线程对象
* timestamp.cc, timestamp.h
  * UTC的时间戳
* time_zone.cc, time_zone.h
  * 时区和夏令时
* types.h
  * 基本类型的生命，dwater::string就是std::string
