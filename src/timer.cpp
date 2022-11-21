#include "timer.h"
#include <algorithm>

void TimerManagement::timer_thread_function() {
    using namespace std::literals::chrono_literals;
    std::chrono::time_point<std::chrono::high_resolution_clock> now;
    while (running) {
        now = std::chrono::high_resolution_clock::now();

        std::vector<ManagedTimer>::iterator next;
        {
            std::lock_guard<std::mutex> lock(timers_mutex);
            for (auto &timer: timers) {
                if (now - timer.last_time > timer.interval) {
                    timer.function();
                    timer.last_time = now;
                }
            }

            next = std::max_element(timers.begin(), timers.end(), [](const ManagedTimer& a, const ManagedTimer& b){
                return (a.last_time + a.interval) < (b.last_time + b.interval);
            });
        }

        std::this_thread::sleep_until(next->last_time + next->interval);
    }
}

void TimerManagement::add_timer(const std::function<void(void)> &function, const std::chrono::milliseconds &interval) {
    std::lock_guard<std::mutex> lock(timers_mutex);
    timers.emplace_back(function, interval);
}

TimerManagement::~TimerManagement() {
    running = false;
    timer_thread.join();
}

TimerManagement::TimerManagement() : running(true), timer_thread(&TimerManagement::timer_thread_function, this) {

}

TimerManagement::ManagedTimer::ManagedTimer(std::function<void(void)> _function,
                                            const std::chrono::milliseconds &_interval)
        : function(std::move(_function)), interval(_interval) {
    last_time = std::chrono::high_resolution_clock::now();
}

