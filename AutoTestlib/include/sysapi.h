#ifndef SYSAPI_H
#define SYSAPI_H
#if defined(_WIN32)
#include <windows.h>
#define OS_WINDOWS 1
#elif defined(__linux__)
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#define OS_LINUX 1
#else
#error "Unsupported operating system"
#endif
#include <fcntl.h>
#include <errno.h>
#include <string>
#include <vector>
#include <memory>
#include "Self.h"

namespace process{
    // 句柄
    typedef int Handle;

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

    // 文件操作权限
    enum FileMode {
        READ_ONLY = O_RDONLY,
        WRITE_ONLY = O_WRONLY,
        READ_WRITE = O_RDWR,
        APPEND = O_APPEND,
        CREATE = O_CREAT,
        TRUNCATE = O_TRUNC
    };

    // 系统类
    class System{
    public:
        // 构造函数
        System(){}
        // 析构函数
        ~System(){}

        // 文件操作
        static Handle open_file(const std::string& path, int mode, int permissions = 0644);
        static int close_file(Handle fd);
        static int64_t read_file(Handle fd, void* buffer, size_t size);
        static int64_t write_file(Handle fd, const void* buffer, size_t size);
        static int64_t seek_file(Handle fd, int64_t offset, int whence);
        static bool file_exists(const std::string& path);

        // 进程控制
        static int fork_process();
        static int execute(const char* path, char* const args[]);
        static int kill_process(int pid, int signal);
        static int wait_process(int pid, int* status, int options = 0);
        static int set_process_limit(int resource, int64_t soft_limit, int64_t hard_limit);

        // 管道操作
        static int create_pipe(Handle pipe_fds[2]);
        static int set_non_blocking(Handle fd, bool non_blocking);
        static int dup_handle(Handle old_fd, Handle new_fd);

        // 环境变量
        static int set_env(const std::string& name, const std::string& value, bool overwrite = true);
        static std::string get_env(const std::string& name);
        static int unset_env(const std::string& name);

        // 错误处理
        static int get_last_error();
        static std::string error_to_string(int error_code);
    };
}

#endif // SYSAPI_H