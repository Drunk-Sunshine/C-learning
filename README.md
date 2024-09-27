本库主要存储一些日常学习的C++小代码，运行平台为Windows，编译器为MSVC

[toc]

# [线程池实现](https://github.com/0voice/cpp_backend_awsome_blog/blob/main/%E3%80%90NO.424%E3%80%91%E7%99%BE%E8%A1%8C%E4%BB%A3%E7%A0%81%E5%AE%9E%E7%8E%B0%E5%9F%BA%E4%BA%8EC%2B%2B11%E7%9A%84%E7%BA%BF%E7%A8%8B%E6%B1%A0threadpool%20%2C%20%E7%AE%80%E6%B4%81%E4%B8%94%E5%8F%AF%E5%B8%A6%E4%BB%BB%E6%84%8F%E5%A4%9A%E5%8F%82%E6%95%B0.md)

- 线程池的优势：降低重复创建销毁线程的资源消耗；提高任务处理速度；提高线程的可管理性
- 线程池组成：任务队列、线程队列、线程池类、线程池管理类以及任务类等组成部分。
- 实现思路：**管理一个任务队列，一个线程队列，然后每次取一个任务分配给一个线程去做，循环往复**
- 实现框架

  - 宏定义THREAD_POOL_H，头文件，std命名空间
  - 定义线程池容量 #define THREADPOOL_MAX_NUM 16
  - 定义线程池类 threadpool
    - 定义了一个模版类别名task(无参数，无返回)；定义一个vector的线程池；定义了一个任务队列；定义mutex的对象；定义condition_variable的对象
    - 定义两个原子变量 `atomic<bool> _run{ true }; //线程池是否执行`   `atomic<int> _idlThrNum{ 0 }; //空闲线程数量`
    - public 实现构造（添加线程数量）和析构（设置_run，唤醒所有线程检查run的状态来退出，遍历检查所有的线程任务是否完成）
    - public 实现commit方法：判断线程池是否运行；绑定future；加锁添加任务；唤醒线程执行，返回future
    - THREADPOOL_AUTO_GROW 定义一个可选项，自动增长线程池
    - 实现 addThread方法 ：循环添加线程(当不大于最大线程数量时)；加锁阻塞线程；从任务队列中拿取任务并更新
    - 如果没有定义THREADPOOL_AUTO_GROW ，将 addThread方法私有化
  - 锁mutex 保证任务的添加和移除(获取)的互斥性（_task本身是临界区）；原子量不需要加锁
  - 条件变量 _task_cv，保证获取 task 的同步性，一个 empty 的队列，线程应该等待(阻塞)；
  - `lock_guard` 和 `unique_lock`其生命周期（用{}进行保护）结束时会自动释放锁，不需要显式调用 `unlock`；相比之下，前者性能更好些，出作用域自动解锁，后者更加自由可以多次手动解锁和加锁
- 没有搞懂的几个点：

  1. 头文件不报错？future头文件包含了很多
  2. auto commit(F&& f, Args&&... args) -> future<decltype(f(args...))>，可变参数模版: https://songlee24.github.io/2014/07/22/cpp-changeable-parameter/
  3. make_shared可以减少内存分配的次数，将对象和控制块一次分配好
  4. `std::packaged_task` 是 C++11 标准库提供的用于封装可调用对象的类。它允许你将一个可调用对象（如函数、Lambda 表达式或绑定的函数）与其参数绑定在一起，并在线程中异步执行该对象，从而提供对其结果的访问。`std::packaged_task<return_type()> taskName(callable);` 或者 `std::packaged_task<return_type()> taskName(std::bind(callable, args...));`

     其中return_type是可调用对象的返回类型，对于使用 `std::bind` 创建的可调用对象，返回类型可以通过 `decltype` 来推导；callable是需要被调用的可调用对象，可能是一个函数指针、Lambda 表达式、成员函数指针或任何类型的可调用对象
  5. wait的最后一个参数为false才会阻塞
  6. move ，forward的作用：move右值；forward包含move，对左值实现转发
  7. 调用.get()获取返回值会等待future任务执行完,获取返回值；否则进程不会等待
