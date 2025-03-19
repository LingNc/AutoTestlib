#include "Timer.h"

namespace process{
    // 计时器类
    Timer::~Timer(){
        stop();
    }

    void Timer::start(int timeout_ms,std::function<void()> callback){
        stop();
        // 正确设置运行标志
        running=true;
        thread=std::thread([this,timeout_ms,callback](){
            std::unique_lock<std::mutex> lock(mutex);
            // 使用条件变量的wait_for，可随时被唤醒
            if(cv.wait_for(lock,std::chrono::milliseconds(timeout_ms),
                [this]{ return !running; })){
                // 如果被提前唤醒且running=false，直接退出
                return;
            }
            // 超时且仍在运行，执行回调
            if(running){
                callback();
            }
            });
    }

    void Timer::stop(){
        {
            std::lock_guard<std::mutex> lock(mutex);
            running=false;
        }
        // 唤醒等待的线程
        cv.notify_all();

        // 仍然需要join，但现在线程会立即退出
        if(thread.joinable()){
            thread.join();
        }
    }
}