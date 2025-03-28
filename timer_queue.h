#ifndef UTILS_TIME_QUEUE_H_
#define UTILS_TIME_QUEUE_H_
#include <set>
#include <functional>

namespace utils
{
class TimerQueue
{
using on_timer_func_t = std::function<void(uint32_t)>;
public:
    TimerQueue() = default;
    TimerQueue(TimerQueue&&) = default;
    TimerQueue& operator=(TimerQueue&&) = default;

    void set_timer(
        uint32_t id, 
        uint32_t mill_sec, 
        const  std::function<void(uint32_t)>& on_timer);
    void kill_timer(uint32_t id);
    bool get_timer_task(uint32_t& id, std::function<void(uint32_t)>& on_timer);
    bool empty();
    size_t timer_count();
    uint64_t get_wait_time();
    void clear();
private:
    struct Timer
    {
        uint32_t id{0};
        uint64_t interval{0};
        uint64_t start_time{0};
        on_timer_func_t on_timer;
        bool operator <(const Timer& rhs) const
        {
            auto end_time = start_time + interval;
            auto rhs_end_time = rhs.start_time + rhs.interval;
            return ((end_time < rhs_end_time) ||
                    (end_time == rhs_end_time && id < rhs.id));
        } 
    };
    std::set<Timer> timer_list_;
};
} // namespace utils
#endif// UTILS_TIME_QUEUE_H_
