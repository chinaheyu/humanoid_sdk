#ifndef HUMANOID_SDK_TIMER_H
#define HUMANOID_SDK_TIMER_H

#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <mutex>
#include <vector>

class TimerManagement {
public:
    TimerManagement();

    ~TimerManagement();

    void add_timer(const std::function<void(void)>& function, const std::chrono::milliseconds& interval);

private:

    struct ManagedTimer {
        ManagedTimer(std::function<void(void)> _function, const std::chrono::milliseconds& _interval);
        std::function<void(void)> function;
        std::chrono::time_point<std::chrono::high_resolution_clock> last_time;
        std::chrono::milliseconds interval;
    };

    std::mutex timers_mutex;
    std::vector<ManagedTimer> timers;

    std::atomic<bool> running;
    std::thread timer_thread;
    void timer_thread_function();

};

#endif //HUMANOID_SDK_TIMER_H
