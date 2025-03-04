#include "../include/Process.h"
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

namespace pc=process;

// Args类实现
pc::Args::Args(){}

pc::Args::Args(const std::vector<string> &arguments): _args(arguments){
    prepare_c_args();
}

pc::Args::Args(const string &command){
    parse(command);
    prepare_c_args();
}


pc::Args &pc::Args::add(const string &arg){
    _args.push_back(arg);
    prepare_c_args();
    return *this;
}

pc::Args &pc::Args::add(const std::vector<string> &arguments){
    _args.insert(_args.end(),arguments.begin(),arguments.end());
    prepare_c_args();
    return *this;
}

pc::Args &pc::Args::set_program_name(const string &name){
    if(_args.empty()){
        _args.push_back(name);
    }
    else{
        _args[0]=name;
    }
    prepare_c_args();
    return *this;
}

void pc::Args::clear(){
    _args.clear();
    _c_args.clear();
}

size_t pc::Args::size() const{
    return _args.size();
}

char **pc::Args::data(){
    prepare_c_args();
    return _c_args.data();
}

std::vector<string> pc::Args::get_args() const{
    return _args;
}

string pc::Args::get_program_name() const{
    if(!_args.empty()){
        return _args[0];
    }
    return "";
}

string &pc::Args::operator[](size_t index){
    return _args[index];
}

const string &pc::Args::operator[](size_t index) const{
    return _args[index];
}

void pc::Args::prepare_c_args(){
    _c_args.clear();
    for(const auto &arg:_args){
        _c_args.push_back(const_cast<char *>(arg.c_str()));
    }
    _c_args.push_back(nullptr); // execvp需要以nullptr结尾
}

// 新增：解析命令行字符串
pc::Args &pc::Args::parse(const string &command_line){
    enum State{ NORMAL,IN_QUOTE,IN_DQUOTE };
    State state=NORMAL;
    string current_arg;
    bool escaped=false;

    for(char c:command_line){
        if(escaped){
            current_arg+=c;
            escaped=false;
            continue;
        }

        switch(state){
        case NORMAL:
            if(c=='\\'){
                escaped=true;
            }
            else if(c=='\''){
                state=IN_QUOTE;
            }
            else if(c=='\"'){
                state=IN_DQUOTE;
            }
            else if(std::isspace(c)){
                if(!current_arg.empty()){
                    _args.push_back(current_arg);
                    current_arg.clear();
                }
            }
            else{
                current_arg+=c;
            }
            break;

        case IN_QUOTE:
            if(c=='\''){
                state=NORMAL;
            }
            else{
                current_arg+=c;
            }
            break;

        case IN_DQUOTE:
            if(c=='\\'){
                escaped=true;
            }
            else if(c=='\"'){
                state=NORMAL;
            }
            else{
                current_arg+=c;
            }
            break;
        }
    }

    // 处理最后一个参数
    if(!current_arg.empty()){
        _args.push_back(current_arg);
    }

    prepare_c_args();
    return *this;
}

void pc::Process::init_pipe(){
    if(pipe(_stdin)==-1
        ||pipe(_stdout)==-1
        ||pipe(_stderr)==-1){
        throw std::runtime_error(name+":管道创建错误！");
    }
}

void pc::Process::launch(const char arg[],char *args[]){
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

void pc::Process::close_pipe(bool flag){
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

// 修改save_args方法使用Args类
void pc::Process::save_args(std::vector<string> &args){
    _args.clear(); // 清除旧的参数
    _args.add(args);
}

pc::Process::Process(){}

pc::Process::Process(const string &path,const Args &args): _args(args){
    _path=path;
    if(args.size()>0){
        name=args.get_program_name();
    }
}

void pc::Process::load(const string &path,const Args &args){
    _path=path;
    _args=args;
    if(args.size()>0){
        name=args.get_program_name();
    }
}

void pc::Process::start(){
    init_pipe();
    launch(_path.c_str(),_args.data());
}

int pc::Process::wait(){
    waitpid(_pid,&exit_code,0);
    _pid=-1;
    return WEXITSTATUS(exit_code);
}

pc::Process &pc::Process::write(const string &data){
    if(_stdin[1]!=-1){
        if(::write(_stdin[1],data.c_str(),data.size())==-1){
            throw std::runtime_error(name+":进程读取错误！");
        }
    }
    return *this;
}

string pc::Process::read(TypeOut type){
    int stdpipe=(type==OUT)?_stdout[0]:_stderr[0];
    if(stdpipe==-1) return "";

    char buffer[buffer_size];
    string result;

    // 设置非阻塞模式以避免无限等待
    int flags;
    if(!_blocked){
        flags=fcntl(stdpipe,F_GETFL,0);
        fcntl(stdpipe,F_SETFL,flags|O_NONBLOCK);
    }


    int nbytes;
    while(true){
        nbytes=::read(stdpipe,buffer,buffer_size-1);
        if(nbytes<=0) break;
        buffer[nbytes]='\0';
        result+=buffer;
    }

    // 恢复阻塞模式
    if(!_blocked){
        fcntl(stdpipe,F_SETFL,flags);
    }
    _empty=(result=="")?true:false;
    return result;
}

char pc::Process::read_char(TypeOut type){
    // 获取合适的管道文件描述符
    int stdpipe=(type==OUT)?_stdout[0]:_stderr[0];
    if(stdpipe==-1) return '\0';

    char ch;
    if(::read(stdpipe,&ch,1)>0) return ch;
    else return 0;
}

string pc::Process::read_line(TypeOut type,int timeout_ms){
    // 获取合适的管道文件描述符
    int stdpipe=(type==OUT)?_stdout[0]:_stderr[0];
    if(stdpipe==-1) return "";

    fd_set read_fds;      // 定义一个文件描述符集，用于select监控
    struct timeval tv;    // 定义超时结构

    FD_ZERO(&read_fds);           // 清空文件描述符集
    FD_SET(stdpipe,&read_fds);   // 添加管道文件描述符到集合中

    // 设置超时时间
    tv.tv_sec=timeout_ms/1000;                // 秒部分
    tv.tv_usec=(timeout_ms%1000)*1000;      // 微秒部分

    string line;
    char c;

    // 使用select检查是否有数据可读，带超时
    if(select(stdpipe+1,&read_fds,NULL,NULL,&tv)>0){
        // 有数据可读，开始读取一行数据
        while(::read(stdpipe,&c,1)>0){
            if(c=='\n') break;  // 遇到换行符结束
            line+=c;             // 将字符添加到结果中

            // 读完一个字符后，立即检查是否还有更多数据
            FD_ZERO(&read_fds);
            FD_SET(stdpipe,&read_fds);
            tv.tv_sec=0;
            tv.tv_usec=0;  // 立即返回，不等待

            // 如果没有更多即时可读的数据，就退出
            if(select(stdpipe+1,&read_fds,NULL,NULL,&tv)<=0){
                break;
            }
        }
    }
    _empty=(line=="")?true:false;
    return line;
}

char pc::Process::getc(){
    int stdpipe=_stdout[0];
    fd_set read_fds;      // 定义一个文件描述符集，用于select监控
    struct timeval tv;    // 定义超时结构

    FD_ZERO(&read_fds);           // 清空文件描述符集
    FD_SET(stdpipe,&read_fds);   // 添加管道文件描述符到集合中

    // 设置超时时间
    int timeout_ms=10;
    tv.tv_sec=timeout_ms/1000;                // 秒部分
    tv.tv_usec=(timeout_ms%1000)*1000;      // 微秒部分

    char ch=0;
    if(select(stdpipe+1,&read_fds,NULL,NULL,&tv)>0){
        ch=read_char(OUT);
    }
    return ch;
}

string pc::Process::getline(){
    return read_line(OUT,10);
}

bool pc::Process::empty(){
    return _empty;
}

void pc::Process::set_block(bool status){
    _blocked=status;
}

void pc::Process::close(){
    close_pipe(1);
    close_pipe(0);
}

bool pc::Process::kill(int signal){
    if(_pid<=0){
        // 程序已经结束
        return false;
    }
    // 发送终止信号
    int result=::kill(_pid,signal);

    if(result==0){
        // 发送成功
        if(signal==SIGKILL||signal==SIGTERM){
            // 等待进程结束
            wait();
            // 关闭管道
            close();
        }
        return true;
    }
    return false;
}

pc::Process &pc::Process::operator<<(std::ostream &(*pf)(std::ostream &)){
    if(pf==static_cast<std::ostream&(*)(std::ostream &)>(std::endl)){
        write("\n");
        flush();
    }
    return *this;
}

pc::Process &pc::Process::flush(){
    return *this;
}

pc::Process::~Process(){
    close();
    close_pipe(1);
    wait();
}

