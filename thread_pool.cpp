#include "thread_pool.h"

namespace utils
{
ThreadPool::~ThreadPool()
{
    if (!threads_.empty() || !task_queue_.empty() || !timer_queue_.empty())
        this->end();
}
bool ThreadPool::begin(int thread_nums/* = 1*/)
{
    if(thread_nums <= 0)
        return false;

    for(auto i = 0; i < thread_nums; ++i)
        threads_.emplace_back(std::bind(&ThreadPool::work_proc, this));

    return true;
}
void ThreadPool::end()
{
    exit_flag_.store(true, std::memory_order_relaxed);
    cv_.notify_all();

    for(auto&& thread: threads_)
    {
        if(thread.joinable())
            thread.join();
    }
    threads_.clear();

    {
        std::scoped_lock latch{mtx_};
        while (!task_queue_.empty())
            task_queue_.pop();
            
        timer_queue_.clear();
    }
    exit_flag_.store(false, std::memory_order_relaxed);
}
void ThreadPool::post_task(std::function<void()> &&task)
{
    if (nullptr == task)
        return;
    
    mtx_.lock();
    task_queue_.push(std::move(task));
    mtx_.unlock();

    cv_.notify_one();
}
void ThreadPool::post_task(const std::function<void()> &task)
{
    if (nullptr == task)
        return;

    mtx_.lock();
    task_queue_.push(task);
    mtx_.unlock();

    cv_.notify_one();
}
size_t ThreadPool::task_count()
{
    size_t count = 0;

    mtx_.lock();
    count = task_queue_.size();
    mtx_.unlock();

    return count;
}
void ThreadPool::set_timer(uint32_t id, uint32_t mill_sec, const std::function<void(uint32_t)>& on_timer)
{
    mtx_.lock();
    timer_queue_.set_timer(id, mill_sec, on_timer);
    mtx_.unlock();

    cv_.notify_one();
}
void ThreadPool::kill_timer(uint32_t id)
{
    mtx_.lock();
    timer_queue_.kill_timer(id);
    mtx_.unlock();
}
size_t ThreadPool::timer_count()
{
    size_t timer_count = 0;

    mtx_.lock();
    timer_count = timer_queue_.timer_count();
    mtx_.unlock();

    return timer_count;
}
void ThreadPool::work_proc()
{
    auto on_deal_task = [this](std::unique_lock<std::mutex>& latch)
    {
        if (task_queue_.empty())
            return;
        std::function<void()> on_task = std::move(task_queue_.front());
        task_queue_.pop();
        latch.unlock();
        on_task();
        latch.lock();
    };
    auto on_deal_timer = [this](std::unique_lock<std::mutex>& latch)
    {
        uint32_t id{0};
        std::function<void(uint32_t)> on_timer;
        if (!timer_queue_.get_timer_task(id, on_timer))
            return;

        latch.unlock();
        on_timer(id);
        latch.lock();
    };
    std::unique_lock latch{mtx_};
    while (!exit_flag_)
    {
        if (task_queue_.empty())
        {
            auto time = timer_queue_.get_wait_time();
            cv_.wait_for(latch, std::chrono::milliseconds(time));
        }
        if (exit_flag_) break;
        on_deal_timer(latch);
        if (exit_flag_) break;
        on_deal_task(latch);
    }
}
} // namespace utils
