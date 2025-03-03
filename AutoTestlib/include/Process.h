#ifndef PROCESS_H
#define PROCESS_H

#include "Self.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


// 输出管道选择
enum TypeOut{
    OUT,ERR
};
class Process{
    // 管道
    int _stdin[2]={ -1,-1 };
    int _stdout[2]={ -1,-1 };
    int _stderr[2]={ -1,-1 };
    // pid
    pid_t _pid=-1;
    // 路径和参数
    string _path;
    std::vector<char *> _args;
    string name = "Process";
    // 退出状态
    int exit_code=-1;
    // 缓冲区大小
    int buffer_size=4096;
    // 初始化管道
    void init_pipe();

    // 创建子进程并初始化
    void launch(const char arg[],char *args[]);

    // 关闭特定的管道 0子进程
    void close_pipe(bool flag);

    // STL转换
    void save_args(std::vector<string> &args);

public:
    Process();
    Process(string &cmd,std::vector<string> &args);
    Process(fs::path &cmd,std::vector<string> &args);

    void load(fs::path cmd,std::vector<string> &args);
    void load(string cmd,std::vector<string> &args);
    // 启动子进程
    void start();
    // 等待子进程结束
    int wait();
    // 写入数据
    Process &write(const string &data);
    // 读取数据
    string read(TypeOut type);
    // 刷新输入
    Process &flush();
    // 关闭子进程的输入
    void close();

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
        ss<<read(OUT);
        ss>>data;
        return *this;
    }
    Process &operator>>(string &output);

    ~Process();
};


#endif // PROCESS_H