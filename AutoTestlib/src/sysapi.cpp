#include "sysapi.h"
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>

namespace process{

// 文件操作
Handle System::open_file(const std::string& path, int mode, int permissions) {
    Handle fd = ::open(path.c_str(), mode, permissions);
    return fd;
}

int System::close_file(Handle fd) {
    if (fd < 0) return SYS_INVALID_HANDLE;
    return ::close(fd);
}

int64_t System::read_file(Handle fd, void* buffer, size_t size) {
    if (fd < 0) return SYS_INVALID_HANDLE;
    return ::read(fd, buffer, size);
}

int64_t System::write_file(Handle fd, const void* buffer, size_t size) {
    if (fd < 0) return SYS_INVALID_HANDLE;
    return ::write(fd, buffer, size);
}

int64_t System::seek_file(Handle fd, int64_t offset, int whence) {
    if (fd < 0) return SYS_INVALID_HANDLE;
    return ::lseek(fd, offset, whence);
}

bool System::file_exists(const std::string& path) {
    struct stat sb;
    return stat(path.c_str(), &sb) == 0;
}

// 进程控制
int System::fork_process() {
#if defined(OS_LINUX)
    return ::fork();
#else
    return SYS_ERROR; // Not supported on Windows
#endif
}

int System::execute(const char* path, char* const args[]) {
#if defined(OS_LINUX)
    return ::execvp(path, args);
#else
    return SYS_ERROR; // Needs different implementation on Windows
#endif
}

int System::kill_process(int pid, int signal) {
#if defined(OS_LINUX)
    return ::kill(pid, signal);
#else
    return SYS_ERROR; // Needs different implementation on Windows
#endif
}

int System::wait_process(int pid, int* status, int options) {
#if defined(OS_LINUX)
    return ::waitpid(pid, status, options);
#else
    return SYS_ERROR; // Needs different implementation on Windows
#endif
}

int System::set_process_limit(int resource, int64_t soft_limit, int64_t hard_limit) {
#if defined(OS_LINUX)
    struct rlimit rl;
    rl.rlim_cur = soft_limit;
    rl.rlim_max = hard_limit;
    return ::setrlimit(resource, &rl);
#else
    return SYS_ERROR; // Needs different implementation on Windows
#endif
}

// 管道操作
int System::create_pipe(Handle pipe_fds[2]) {
#if defined(OS_LINUX)
    return ::pipe(pipe_fds);
#else
    return SYS_ERROR; // Needs different implementation on Windows
#endif
}

int System::set_non_blocking(Handle fd, bool non_blocking) {
    if (fd < 0) return SYS_INVALID_HANDLE;

    int flags = ::fcntl(fd, F_GETFL);
    if (flags == -1) return SYS_ERROR;

    if (non_blocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }

    return ::fcntl(fd, F_SETFL, flags);
}

int System::dup_handle(Handle old_fd, Handle new_fd) {
    if (old_fd < 0) return SYS_INVALID_HANDLE;
    return ::dup2(old_fd, new_fd);
}

// 环境变量
int System::set_env(const std::string& name, const std::string& value, bool overwrite) {
#if defined(OS_LINUX)
    return ::setenv(name.c_str(), value.c_str(), overwrite ? 1 : 0);
#else
    // Windows implementation would use SetEnvironmentVariable
    if (!overwrite && get_env(name).length() > 0) {
        return 0;
    }
    std::string env_str = name + "=" + value;
    return ::putenv(strdup(env_str.c_str()));
#endif
}

std::string System::get_env(const std::string& name) {
    const char* val = ::getenv(name.c_str());
    return val ? std::string(val) : std::string();
}

int System::unset_env(const std::string& name) {
#if defined(OS_LINUX)
    return ::unsetenv(name.c_str());
#else
    // Windows implementation would use SetEnvironmentVariable with NULL
    std::string env_str = name + "=";
    return ::putenv(strdup(env_str.c_str()));
#endif
}

// 错误处理
int System::get_last_error() {
    return errno;
}

std::string System::error_to_string(int error_code) {
    return std::string(strerror(error_code));
}

} // namespace process