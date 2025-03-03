#include "../include/Process.h"
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

void Process::init_pipe(){
    if(pipe(_stdin)==-1
        ||pipe(_stdout)==-1
        ||pipe(_stderr)==-1){
        throw std::runtime_error(name+":管道创建错误！");
    }
}

void Process::launch(const char arg[],char *args[]){
    _pid=fork();
    // 子进程
    if(_pid==0){
        // 输入输出重定向
        dup2(_stdin[0],STDIN_FILENO);
        dup2(_stdout[1],STDOUT_FILENO);
        dup2(_stderr[1],STDERR_FILENO);

        // 关闭管道
        close_pipe(1);

        // 运行子程序
        execvp(arg,args);
        exit(EXIT_FAILURE);
    }
    else if(_pid<0){
        throw std::runtime_error(name+":子程序运行失败！");
    }
    close_pipe(0);
}

void Process::close_pipe(bool flag){
    // 确保先关闭读端再关闭写端
    if(_stdin[flag]!=-1){
        ::close(_stdin[flag]);
        _stdin[flag]=-1;
    }
    if(_stdout[!flag]!=-1){
        ::close(_stdout[!flag]);
        _stdout[!flag]=-1;
    }
    if(_stderr[!flag]!=-1){
        ::close(_stderr[!flag]);
        _stderr[!flag]=-1;
    }
}

void Process::save_args(std::vector<string> &args){
    _args.clear(); // 清除旧的参数
    for(auto &it:args){
        // 去除const属性
        _args.push_back(const_cast<char *>(it.c_str()));
    }
    _args.push_back(nullptr);
}

Process::Process(){}

Process::Process(string &cmd,std::vector<string> &args){
    load(cmd,args);
}

Process::Process(fs::path &cmd,std::vector<string> &args){
    load(cmd,args);
}

void Process::load(fs::path cmd,std::vector<string> &args){
    load(cmd.string(),args);
}

void Process::load(string cmd,std::vector<string> &args){
    _path=cmd;
    name=args[0];
    save_args(args);
}

void Process::start(){
    init_pipe();
    launch(_path.c_str(),_args.data());
}

int Process::wait(){
    waitpid(_pid,&exit_code,0);
    return WEXITSTATUS(exit_code);
}

Process &Process::write(const string &data){
    if(_stdin[1]!=-1){
        if(::write(_stdin[1],data.c_str(),data.size())==-1){
            throw std::runtime_error(name+":进程读取错误！");
        }
    }
    return *this;
}

string Process::read(TypeOut type){
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

    // 设置非阻塞模式以避免无限等待
    int flags=fcntl(stdpipe,F_GETFL,0);
    fcntl(stdpipe,F_SETFL,flags|O_NONBLOCK);

    while((nbytes=::read(stdpipe,buffer,buffer_size-1))>0){
        buffer[nbytes]='\0';
        result+=buffer;
    }

    // 恢复阻塞模式
    fcntl(stdpipe,F_SETFL,flags);
    return result;
}

void Process::close(){
    if(_stdin[1]!=-1){
        ::close(_stdin[1]);
        _stdin[1]=-1;
    }
}

Process &Process::operator>>(string &output){
    output=read(OUT);
    return *this;
}

Process &Process::operator<<(std::ostream &(*pf)(std::ostream &)){
    if(pf==static_cast<std::ostream &(*)(std::ostream &)>(std::endl)){
        write("\n");
        flush();
    }
    return *this;
}

Process &Process::flush(){
    return *this;
}

Process::~Process(){
    close();
    close_pipe(1);
    wait();
}
