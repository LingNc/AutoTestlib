#ifndef TIMER_H
#define TIMER_H

#include "Self.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>

namespace process{
    // 计时器
    class Timer{
    private:
        std::thread thread;
        std::atomic<bool> running{ false };
        std::mutex mutex;
        std::condition_variable cv;
    public:
        Timer()=default;
        ~Timer();

        void start(int timeout_ms,std::function<void()> callback);
        void stop();
    };
}

#endif // TIMER_H