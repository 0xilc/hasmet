#pragma once

#include "logging.h"
#include <chrono>

namespace hasmet {
class ScopedTimer{
public:
    ScopedTimer(const std::string& name): name_(name), start_(std::chrono::high_resolution_clock::now()) {}

    ~ScopedTimer() {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_);
        LOG_INFO(name_ + " took " + std::to_string(duration.count()) + " ms.");
    }
private:
    std::string name_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};
} // namespace hasmet

#define SCOPED_TIMER(name) hasmet::ScopedTimer _scoped_timer_##__LINE__(name)

#define TIMER_START(timerName) \
    auto _timer_start_##timerName = std::chrono::high_resolution_clock::now()

#define TIMER_STOP(timerName) \
do { \
    auto _timer_end_##timerName = std::chrono::high_resolution_clock::now(); \
    auto _duration_##timerName = std::chrono::duration_cast<std::chrono::milliseconds>(_timer_end_##timerName - _timer_start_##timerName); \
    LOG_INFO(std::string(#timerName) + " took " + std::to_string(_duration_##timerName.count()) + " ms."); \
} while(0)
