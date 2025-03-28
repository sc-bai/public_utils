#include "timer_queue.h"
#include <chrono>
#include <algorithm>
#include <utils/system.h>

namespace utils
{
void TimerQueue::set_timer(
    uint32_t id,
    uint32_t mill_sec,
    const std::function<void(uint32_t)> &on_timer)
{
    kill_timer(id);
    Timer timer;
    timer.id = id;
    timer.start_time = utils::get_tickout();
    timer.interval = mill_sec;
    timer.on_timer = on_timer;

    timer_list_.insert(std::move(timer));
}
void TimerQueue::kill_timer(uint32_t id)
{
    auto iter = std::find_if(timer_list_.begin(), timer_list_.end(), 
        [id](const Timer& elem){
            return id == elem.id;
    });
    if (iter != timer_list_.cend())
        timer_list_.erase(iter);
}
bool TimerQueue::get_timer_task(uint32_t &id, std::function<void(uint32_t)> &on_timer)
{
    if (timer_list_.empty())
        return false;

    auto iter = timer_list_.begin();
    auto end_time = iter->start_time + iter->interval;
    uint64_t curr_time = utils::get_tickout();
    if (curr_time < end_time)
        return false;

    auto timer_node = timer_list_.extract(iter);
    auto& timer = timer_node.value();

    id = timer.id;
    on_timer = timer.on_timer;
    timer.start_time += timer.interval;

    timer_list_.insert(std::move(timer_node));
    return true;
}
bool TimerQueue::empty()
{
    return timer_list_.empty();
}
size_t TimerQueue::timer_count()
{
    return timer_list_.size();
}
uint64_t TimerQueue::get_wait_time()
{
    constexpr uint32_t kWaitTimeInterval = 7 * 86400 * 1000;
    if(timer_list_.empty())
        return kWaitTimeInterval;

    const auto& timer = *(timer_list_.cbegin());
    auto end_time = timer.start_time + timer.interval;
    uint64_t curr_time = utils::get_tickout();
    return curr_time <= end_time ? end_time - curr_time : 0;
}
void TimerQueue::clear()
{
    timer_list_.clear();
}
} // namespace utils
