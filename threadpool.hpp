#include "thread"
#include <mutex>
#include <queue>

class ThreadPool
{
public:
    // Allow only 1 threadpool at most.
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator = (const ThreadPool&) = delete;
    ThreadPool& operator = (const ThreadPool&) = delete;
    
    explicit ThreadPool(std::size_t thread_count = std::thread::hardware_concurrency())
    {}

    ~ThreadPool()
    {}

private:
};