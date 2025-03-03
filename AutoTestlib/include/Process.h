#ifndef Process_H
#define Process_H

#include "Self.h"
#include <vector>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>


namespace Process{
    // 输出管道选择
    enum TypeOut{
        OUT,ERR
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
        Args(const std::vector<string>& arguments);
        Args(const string& command);

        // 添加参数
        Args& add(const string& arg);
        // 添加多个参数
        Args& add(const std::vector<string>& arguments);
        // 设置程序名称（第一个参数）
        Args& set_program_name(const string& name);
        // 新增：解析命令行字符串
        Args& parse(const string& command_line);
        // 清除所有参数
        void clear();
        // 获取参数数量
        size_t size() const;
        // 获取C风格参数，用于execvp
        char** data();
        // 获取所有参数的复制
        std::vector<string> get_args() const;
        // 获取程序名称
        string get_program_name() const;
        // 获取指定位置的参数
        string& operator[](size_t index);
        const string& operator[](size_t index) const;
    };

    // 进程类
    class Process{
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
        Process(string &cmd, const Args &args);
        // 载入命令和参数
        void load(const string &cmd, const Args &args);
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
}


#endif // Process_H