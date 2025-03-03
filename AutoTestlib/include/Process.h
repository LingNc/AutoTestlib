#include "Slef.h"
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
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
    // 退出状态
    int exit_code=-1;
    // 缓冲区大小
    int buffer_size=4096;
    // 输出管道选择
    enum TypeOut{
        OUT,
        ERR
    };
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
        ::close(_stdin[flag]);
        ::close(_stdout[~flag]);
        ::close(_stderr[~flag]);
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
        start();
    }
    Process(fs::path &cmd,std::vector<string> &args){
        load(cmd,args);
        start();
    }
    void load(fs::path cmd,std::vector<string> &args){
        load(cmd.string(),args);
    }
    void load(string cmd,std::vector<string> &args){
        _path=cmd;
        save_args(args);
    }
    void start(){
        init_pipe();
        launch(_path.c_str(),_args.data());
    }
    int wait(){
        waitpid(_pid,&exit_code,0);
        return WEXITSTATUS(exit_code);
    }
    Process &write(const string &data){
        if(_stdin[1]!=-1){
            if(::write(_stdin[1],data.c_str(),data.size())==-1){
                throw std::runtime_error(name+":进程读取错误！");
            }
        }
        return *this;
    }
    string read(TypeOut type){
        int stdpipe;
        if(type==OUT){
            stdpipe=_stdout[0];
        }
        else{
            stdpipe=_stderr[0];
        }
        if(stdpipe==-1)
            return "";

        char buffer[buffer_size];
        string result;
        ssize_t nbytes;
        while(nbytes=::read(stdpipe,buffer,buffer_size-1)){
            buffer[nbytes]='\0';
            result+=buffer;
        }
        return result;
    }
    // 关闭子进程的输入
    void close(){
        if(_stdin[1]!=-1){
            ::close(_stdin[1]);
            _stdin[1]=-1;
        }
    }
    // 重载运算符
    template<typename T>
    Process &operator<<(const T &data){
        std::stringstream ss;
        ss<<data;
        return write(ss.str());
    }
    Process &operator>>(string &output){
        output=read(OUT);
        return *this;
    }
    ~Process(){
        close_pipe(1);
    }

};