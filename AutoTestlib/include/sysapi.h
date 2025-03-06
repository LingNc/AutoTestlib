#ifndef SYSAPI_H
#define SYSAPI_H
#if defined(_WIN32)
#define <windows.h>
    #define OS_WINDOWS 1
#elif defined(__linux__)
    #include <unistd.h>
    #define OS_LINUX 1
#else
    #error "Unsupported operating system"
#endif
#include <fcntl.h>
#include "Self.h"



namespace process{
    // 管道
    typedef int Handle;
    // 系统类
    class System{
        // 阻塞信号
        int _flags;
        // 是否被阻塞
        int _isBlocked=true;
        // 在非阻塞模式下的数据超时时间
        int _pipeTime=100;
    public:
        // 构造函数
        System(){}
        // 析构函数
        ~System(){}
        // 设置阻塞模式
        void set_blocked(bool isblocked);
        // 设置非阻塞的读取超时时间
        void set_ptime(int pipeTime);
        // 是否阻塞开始
        void start_blocked(Handle _pipe);
        // 是否阻塞结束
        void close_blocked(Handle _pipe);
        // 读取数据
        string read(Handle _pipe);
        string read(Handle _pipe,char _end_char);
        string read(Handle _pipe,int _read_size);
    };
}

#endif // SYSAPI_H