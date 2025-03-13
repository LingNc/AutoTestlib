#ifndef Process_H
#define Process_H

#include "Self.h"
#include "Timer.h"
#include "Args.h"
#include "JudgeSign.h"
#include "Pipe.h"
#include <iostream>
#include <sstream>
#include <map>


namespace process{
    // 进程类
    // 程序状态
    enum Status{ RUNNING,STOP,ERROR,TIMEOUT,MEMOUT,RE };
    class Process{
        // 计时器
        Timer _timer;
        // 参数
        Args _args;
        // 系统接口
        System _sys;
        // 当前进程状态 原子变量
        std::atomic<Status> _status=STOP;
        // 管道
        Pipe _stdin,_stdout,_stderr;
        // 子进程信息传递控制管道
        Pipe _child_message;
        // pid
        pid_t _pid=-1;
        // 路径
        string _path;
        string name="Process";
        // 缓冲区
        string _buffer[2];
        // 环境变量存储
        std::map<string,string> _env_vars;
        // 内存限制
        int _memsize=0;
        // 时间超限
        int _timelimit=0;
        // 输出是否空
        bool _empty=true;
        // 是否启用颜色
        bool _enable_color=false;
        // 退出状态
        int _exit_code=-1;
        // 判断状态
        JudgeCode _code=Waiting;
        // 缓冲区大小
        int _buffer_size=4096;
        // 非阻塞超时
        int _flushTime=100;
        // 初始化管道
        void init_pipe();
        // 创建子进程并初始化
        void launch(const char arg[],char *args[]);
        // 开始计时是否超时
        void start_timer();
        // 读字符
        char read_char(PipeType type);
        // 读取一行
        string read_line(PipeType type,char delimiter='\n');
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
        // 获得退出状态
        int get_exit_code() const;
        // 读取数据
        string read(PipeType type=PIPE_OUT,size_t nbytes=0);
        // 写入数据
        Process &write(const string &data);
        // 读一行
        string getline(char delimiter='\n');
        // 读错误
        string geterr(size_t nbytes=0);
        // 读字符
        char getchar();
        // 刷新输入
        Process &flush();
        // 关闭所有管道
        void close(PipeType type=PIPE);
        // 终止进程
        bool kill(int signal=SIGKILL);
        // 流是否为空
        bool empty(PipeType type=PIPE_OUT);
        // 设置阻塞状态
        void set_block(bool status);
        // 设置非阻塞超时时间
        void set_flush(int timeout_ms);
        // 设置管道缓冲区大小
        void set_buffer_size(size_t size);
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
        Process &set_memout(int memout_mb);
        // 取消内存限制
        Process &cancel_memout();

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
            ss<<read_line(PIPE_OUT);
            ss>>data;
            return *this;
        }

        ~Process();
    };
}


#endif // Process_H