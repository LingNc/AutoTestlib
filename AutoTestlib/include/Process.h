#include "Slef.h"
#include <vector>
#include <unistd.h>
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
    string name;
    // 初始化管道
    void init_pipe(){
        if(pipe(_stdin)==-1
            ||pipe(_stdout)==-1
            ||pipe(_stderr)==-1){
            throw std::runtime_error(name+":管道创建错误！");
        }
    }

    // 创建子进程并初始化
    void launch(const char arg[],char *args[]){
        _pid=fork();
        // 子进程
        if(_pid==0){
            // 输入输出重定向
            dup2(_stdin[0],STDIN_FILENO);
            dup2(_stdout[1],STDOUT_FILENO);
            dup2(_stderr[1],STDERR_FILENO);

            // 关闭管道
            close_pipe(0);

            // 运行子程序
            execvp(arg,args);
            exit(EXIT_FAILURE);
        }
        else if(_pid<0){
            throw std::runtime_error(name+":子程序运行失败！");
        }
        close_pipe(1);
    }
    // 关闭特定的管道 0子进程
    void close_pipe(bool flag){
        close(_stdin[flag]);
        close(_stdout[~flag]);
        close(_stderr[~flag]);
    }
    // STL转换
    void save_args(std::vector<string> &args){
        for(auto &it:args){
            // 去除const属性
            _args.push_back(const_cast<char *>(it.c_str()));
        }
        _args.push_back(nullptr);
    }
public:
    Process(){}
    Process(string &cmd,std::vector<string> &args){
        load(cmd,args);
    }
    Process(fs::path &cmd,std::vector<string> &args){
        load(cmd,args);
    }
    void load(fs::path cmd,std::vector<string> &args){
        load(cmd.string(),args);
    }
    void load(string cmd,std::vector<string> &args){
        _path=cmd;
        save_args(args);
    }
};