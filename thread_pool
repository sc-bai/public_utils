#ifndef UTILS_THREAD_POOL_H_
#define UTILS_THREAD_POOL_H_
#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "timer_queue.h"

namespace utils
{
class ThreadPool
{
    using TaskQueue = std::queue<std::function<void()>>;
public:
    ThreadPool() = default;
    ~ThreadPool();
    ThreadPool(ThreadPool&&) = default;
    ThreadPool& operator=(ThreadPool&&) = default;

    bool begin(int thread_nums = 1);
    void end();
    
    void post_task(std::function<void()>&& task);
    void post_task(const std::function<void()>& task);
    size_t task_count();

    void set_timer(uint32_t id, uint32_t mill_sec, const std::function<void(uint32_t)>& on_timer);
    void kill_timer(uint32_t id);
    size_t timer_count();

private:
    void work_proc();
private:
    std::atomic_bool exit_flag_{false};
    std::vector<std::thread> threads_;
    std::condition_variable cv_;
    std::mutex mtx_;
    TaskQueue task_queue_;
    TimerQueue timer_queue_;
}; 
} // utils
#endif// UTILS_THREAD_POOL_H_
