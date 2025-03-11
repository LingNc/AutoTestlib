#include "sysapi.h"

#include <string.h>
#include <stdlib.h>

#if defined(OS_UNIX)
    #include <sys/resource.h>
    #include <sys/stat.h>
    #include <errno.h>
#elif defined(OS_WINDOWS)
    #include <string>
    #pragma comment(lib, "kernel32.lib")
    #pragma comment(lib, "user32.lib")
    #define NOMINMAX 1
    #include <windows.h>
    #include <processthreadsapi.h>
    #include <shellapi.h>
    #undef NOMINMAX

    // 映射 Unix 信号到 Windows 终止码
    #define SIGKILL 9
    #define SIGTERM 15
#endif

namespace process{
// 进程控制
ProcessInfo System::fork_process() {
#if defined(OS_UNIX)
    return ::fork();
#else
    // Windows 不支持 fork，返回一个无效的进程信息
    return { 0 };
#endif
}

int System::execute(const char* path, char* const args[]) {
#if defined(OS_UNIX)
    return ::execvp(path, args);
#else
    // Windows 使用 _spawnvp 函数
    // 由于参数类型不同，需要转换 args
    _execvp(path, args);
    return -1; // 如果 _execvp 成功，不会返回到这里
#endif
}

int System::kill_process(ProcessInfo pid, int signal) {
#if defined(OS_UNIX)
    return ::kill(pid, signal);
#else
    if (signal == SIGKILL || signal == SIGTERM) {
        if (TerminateProcess(pid.hProcess, 1)) {
            return 0;
        }
    }
    return -1;
#endif
}

int System::wait_process(ProcessInfo pid, int* status, int options) {
#if defined(OS_UNIX)
    return ::waitpid(pid, status, options);
#else
    DWORD exit_code = 0;
    if (WaitForSingleObject(pid.hProcess, options == WNOHANG ? 0 : INFINITE) == WAIT_OBJECT_0) {
        GetExitCodeProcess(pid.hProcess, &exit_code);
        if (status) *status = exit_code;
        CloseHandle(pid.hProcess);
        CloseHandle(pid.hThread);
        return pid.dwProcessId;
    }
    return 0;
#endif
}

int System::set_process_limit(int resource, int64_t soft_limit, int64_t hard_limit) {
#if defined(OS_UNIX)
    struct rlimit rl;
    rl.rlim_cur = soft_limit;
    rl.rlim_max = hard_limit;
    return ::setrlimit(resource, &rl);
#else
    // Windows 没有直接等价的函数，需要单独实现
    // 对于内存限制，可以使用 Job 对象
    return -1;
#endif
}

// 管道操作
int System::create_pipe(PipeHandle pipe_fds[2]) {
#if defined(OS_UNIX)
    return ::pipe(pipe_fds);
#else
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    if (CreatePipe(&pipe_fds[0], &pipe_fds[1], &sa, 0)) {
        return 0;
    }
    return -1;
#endif
}

int System::set_non_blocking(PipeHandle fd, bool non_blocking) {
#if defined(OS_UNIX)
    if (fd < 0) return SYS_INVALID_HANDLE;

    int flags = ::fcntl(fd, F_GETFL);
    if (flags == -1) return SYS_ERROR;

    if (non_blocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }

    return ::fcntl(fd, F_SETFL, flags);
#else
    DWORD mode = non_blocking ? PIPE_NOWAIT : PIPE_WAIT;
    if (SetNamedPipeHandleState(fd, &mode, NULL, NULL)) {
        return 0;
    }
    return -1;
#endif
}

int System::dup_handle(PipeHandle old_fd, PipeHandle new_fd) {
#if defined(OS_UNIX)
    if (old_fd < 0) return SYS_INVALID_HANDLE;
    return ::dup2(old_fd, new_fd);
#else
    if (old_fd == INVALID_HANDLE_VALUE) return SYS_INVALID_HANDLE;

    HANDLE process = GetCurrentProcess();
    BOOL result = DuplicateHandle(
        process, old_fd,
        process, &new_fd,
        0, TRUE, DUPLICATE_SAME_ACCESS
    );

    return result ? 0 : -1;
#endif
}

int System::close_handle(PipeHandle handle) {
#if defined(OS_UNIX)
    return ::close(handle);
#else
    return CloseHandle(handle) ? 0 : -1;
#endif
}

int64_t System::read_pipe(PipeHandle fd, void* buffer, size_t size) {
#if defined(OS_UNIX)
    return ::read(fd, buffer, size);
#else
    DWORD bytes_read = 0;
    if (ReadFile(fd, buffer, size, &bytes_read, NULL)) {
        return bytes_read;
    }
    return -1;
#endif
}

int64_t System::write_pipe(PipeHandle fd, const void* buffer, size_t size) {
#if defined(OS_UNIX)
    return ::write(fd, buffer, size);
#else
    DWORD bytes_written = 0;
    if (WriteFile(fd, buffer, size, &bytes_written, NULL)) {
        return bytes_written;
    }
    return -1;
#endif
}

// 环境变量
int System::set_env(const std::string& name, const std::string& value, bool overwrite) {
#if defined(OS_UNIX)
    return ::setenv(name.c_str(), value.c_str(), overwrite ? 1 : 0);
#else
    if (!overwrite) {
        char* existing = getenv(name.c_str());
        if (existing != nullptr) {
            return 0;
        }
    }
    // Windows 使用 SetEnvironmentVariable
    return SetEnvironmentVariableA(name.c_str(), value.c_str()) ? 0 : -1;
#endif
}

std::string System::get_env(const std::string& name) {
#if defined(OS_UNIX) || defined(OS_WINDOWS)
    const char* val = ::getenv(name.c_str());
    return val ? std::string(val) : std::string();
#else
    char buffer[32767]; // Windows 环境变量最大长度
    DWORD size = GetEnvironmentVariableA(name.c_str(), buffer, sizeof(buffer));
    if (size > 0 && size < sizeof(buffer)) {
        return std::string(buffer);
    }
    return std::string();
#endif
}

int System::unset_env(const std::string& name) {
#if defined(OS_UNIX)
    return ::unsetenv(name.c_str());
#else
    // Windows 通过设置为 NULL 来取消环境变量
    return SetEnvironmentVariableA(name.c_str(), NULL) ? 0 : -1;
#endif
}

// 错误处理
int System::get_last_error() {
#if defined(OS_UNIX)
    return errno;
#else
    return GetLastError();
#endif
}

std::string System::error_to_string(int error_code) {
#if defined(OS_UNIX)
    return std::string(strerror(error_code));
#else
    char buffer[1024];
    FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        buffer, sizeof(buffer), NULL
    );
    return std::string(buffer);
#endif
}

} // namespace process