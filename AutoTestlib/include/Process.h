#ifndef Process_H
#define Process_H

#include "Self.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <map>
#include <chrono>
#include <thread>
#include <atomic>
#include <functional>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "JudgeSign.h"


namespace process{
    // 计时器
    class Timer{
    private:
        std::thread thread;
        std::atomic<bool> running{ false };

    public:
        Timer()=default;
        ~Timer();

        void start(int timeout_ms,std::function<void()> callback);
        void stop();
    };

    // 参数类
    class Args{
    private:
        // 存储参数的容器
        std::vector<string> _args;
        // 转换后的C风格参数，用于execvp
        std::vector<char *> _c_args;

        // 转换函数，将string参数转换为C风格参数
        void prepare_c_args();

    public:
        // 构造函数
        Args();
        // 读取参数列表
        Args(const std::vector<string> &arguments);
        // 读取命令行，或者程序名
        Args(const string &command);

        // 添加参数
        Args &add(const string &arg);
        // 添加多个参数
        Args &add(const std::vector<string> &arguments);
        // 设置程序名称（第一个参数）
        Args &set_program_name(const string &name);
        // 新增：解析命令行字符串
        Args &parse(const string &command_line);
        // 清除所有参数
        void clear();
        // 获取参数数量
        size_t size() const;
        // 获取C风格参数，用于execvp
        char **data();
        // 获取所有参数的复制
        std::vector<string> get_args() const;
        // 获取程序名称
        string get_program_name() const;
        // 获取指定位置的参数
        string &operator[](size_t index);
        const string &operator[](size_t index) const;
    };

    // 进程类
    // 程序状态
    enum Status{ RUNNING,STOP,ERROR,TIMEOUT,MEMOUT,RE };
    // 输出管道选择
    enum TypeOut{ OUT,ERR };
    class Process{
        // 当前进程状态
        Status _status=STOP;
        // 管道
        int _stdin[2]={ -1,-1 };
        int _stdout[2]={ -1,-1 };
        int _stderr[2]={ -1,-1 };
        // pid
        pid_t _pid=-1;
        // 路径和参数
        string _path;
        Args _args;
        string name="Process";
        // 缓冲区
        string _buffer[2];
        // 环境变量存储
        std::map<string,string> _env_vars;
        // 计时器
        Timer _timer;
        // 内存限制
        int _memsize=0;
        // 输出是否空
        bool _empty=true;
        // 是否是阻塞状态
        bool _blocked=true;
        // 是否启用颜色
        bool _enable_color=false;
        // 退出状态
        int _exit_code=-1;
        // 缓冲区大小
        int _buffer_size=4096;
        // 初始化管道
        void init_pipe();
        // 创建子进程并初始化
        void launch(const char arg[],char *args[]);
        // 关闭特定的管道 0子进程
        void close_pipe(bool flag);
        // STL转换
        void save_args(std::vector<string> &args);
        // 写入数据
        Process &write(const string &data);
        // 读字符
        char read_char(TypeOut OUT);
        // 读取一行
        string read_line(TypeOut type);
    public:
        // 构造函数
        Process();
        // 载入文件路径和参数
        Process(const string &path,const Args &args);
        // 载入命令和参数
        void load(const string &path,const Args &args);
        // 启动子进程
        void start();
        // 等待子进程结束
        JudgeCode wait();
        // 读取数据
        string read(TypeOut type);
        // 读一行
        string getline();
        // 读字符
        char getc();
        // 刷新输入
        Process &flush();
        // 关闭所有管道
        void close();
        // 终止进程
        bool kill(int signal=SIGKILL);
        // 流是否为空
        bool empty();
        // 设置阻塞状态
        void set_block(bool status);
        // 设置环境变量
        Process &set_env(const std::string &name,const std::string &value);
        // 获取环境变量
        string get_env(const std::string &name) const;
        // 清除特定环境变量
        void unset_env(const std::string &name);
        // 清除所有设置的环境变量
        void clear_env();
        // 检查进程是否在运行
        bool is_running();

        // 设置超时
        Process &set_timeout(int timeout_ms);
        // 取消超时
        Process &cancel_timeout();

        // 设置内存限制
        bool set_memout(int memout_mb);
        // 取消内存限制
        bool cancel_memout();

        // 重载运算符
        template<typename T>
        Process &operator<<(const T &data){
            std::stringstream ss;
            ss<<data;
            return write(ss.str());
        }
        Process &operator<<(std::ostream &(*pf)(std::ostream &));

        template<typename T>
        Process &operator>>(T &data){
            std::stringstream ss;
            ss<<read_line(OUT);
            ss>>data;
            return *this;
        }

        ~Process();
    };
}


#endif // Process_H