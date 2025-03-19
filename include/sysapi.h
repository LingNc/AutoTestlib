#ifndef SYSAPI_H
#define SYSAPI_H

// 操作系统检测
#if defined(_WIN32) || defined(_WIN64)
    #define OS_WINDOWS 1
    #include <windows.h>
    #include <io.h>
    #include <process.h>
    typedef HANDLE PipeHandle;
    typedef PROCESS_INFORMATION ProcessInfo;
#elif defined(__linux__) || defined(__APPLE__)
    #define OS_UNIX 1
    #include <unistd.h>
    #include <sys/wait.h>
    #include <sys/types.h>
    #include <sys/time.h>
    #include <signal.h>
    #include <fcntl.h>
    typedef int PipeHandle;
    typedef pid_t ProcessInfo;
#else
    #error "不支持的操作系统"
#endif

#include <string>
#include <vector>
#include <memory>
#include "Self.h"

namespace process {
    // 通用句柄类型
    #if defined(OS_WINDOWS)
        typedef HANDLE Handle;
        const Handle INVALID_HANDLE_VALUE = (HANDLE)-1;
    #else
        typedef int Handle;
        const Handle INVALID_HANDLE_VALUE = -1;
    #endif

    // 常见错误码封装
    enum SysError {
        SYS_OK = 0,
        SYS_ERROR = -1,
        SYS_TIMEOUT = -2,
        SYS_ACCESS_DENIED = -3,
        SYS_NOT_FOUND = -4,
        SYS_INVALID_HANDLE = -5,
        SYS_INTERRUPTED = -6,
        SYS_WOULD_BLOCK = -7
    };

    // 系统类
    class System {
    public:
        // 构造函数
        System() {}
        // 析构函数
        ~System() {}

        // 进程控制
        static ProcessInfo fork_process();
        static int execute(const char* path, char* const args[]);
        static int kill_process(ProcessInfo pid, int signal);
        static int wait_process(ProcessInfo pid, int* status, int options = 0);
        static int set_process_limit(int resource, int64_t soft_limit, int64_t hard_limit);

        // 管道操作
        static int create_pipe(PipeHandle pipe_fds[2]);
        static int set_non_blocking(PipeHandle fd, bool non_blocking);
        static int dup_handle(PipeHandle old_fd, PipeHandle new_fd);
        static int close_handle(PipeHandle handle);
        static int64_t read_pipe(PipeHandle fd, void* buffer, size_t size);
        static int64_t write_pipe(PipeHandle fd, const void* buffer, size_t size);

        // 环境变量
        static int set_env(const std::string& name, const std::string& value, bool overwrite = true);
        static std::string get_env(const std::string& name);
        static int unset_env(const std::string& name);

        // 错误处理
        static int get_last_error();
        static std::string error_to_string(int error_code);

        // 系统信息
        static bool is_windows() {
            #if defined(OS_WINDOWS)
                return true;
            #else
                return false;
            #endif
        }
    };
}

#endif // SYSAPI_H