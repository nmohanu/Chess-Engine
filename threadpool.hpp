#include "transposition_table.hpp"
#include "thread"
#include <mutex>
#include <queue>
#include <functional>
#include <memory>
#include <condition_variable>
#include <chrono>

class ThreadPool
{
public:
    // Allow only 1 threadpool at most.
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator = (const ThreadPool&) = delete;
    
    explicit ThreadPool(std::size_t thread_count = std::thread::hardware_concurrency())
    {
        if(!thread_count) throw std::invalid_argument("no threads available");
        for(auto i = 0; i < thread_count; i++)
        {
            threads.push_back(std::thread([this]()
            {
                while(true)
                {
                    task_pointer work(nullptr);
                    {
                        std::unique_lock guard(m_queue_lock);
                        m_condition.wait(guard, [this]() {
                            return m_check;
                        });
                        m_check = false;
                        work = std::move(m_queue.front());
                        m_queue.pop();
                    }

                    if(!work)
                    {
                        m_queue.push(task_pointer{nullptr});
                        break;
                    }
                    (*work)();
                }
            }));
        }
    }

    ~ThreadPool()
    {
        stop();
        for(auto& t : threads)
        {
            t.join();
        }
    }

    using task_t = std::function<float()>;


    void do_task(task_t task_item, int, int, int, int&, Position*, Move&, bool)
    {
        auto task_p = std::make_unique<task_t>(std::move(task_item));
        {
            std::unique_lock guard(m_queue_lock);
            m_queue.push(std::move(task_p));
            m_check = true;
        }

        m_condition.notify_one();
    }

    void stop()
    {
        std::unique_lock guard(m_queue_lock);
        m_queue.push(std::move(task_pointer{nullptr}));
        m_check = true;
        m_condition.notify_one();
    }

private:
    using task_pointer = std::unique_ptr<task_t>;
    using task_queue = std::queue<task_pointer>;

    task_queue m_queue;
    std::mutex m_queue_lock;
    std::condition_variable m_condition;
    bool m_check = false;

    using threads_t = std::vector<std::thread>;
    threads_t threads;
};