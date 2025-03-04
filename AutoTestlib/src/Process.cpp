#include "../include/Process.h"
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

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
        // 设置环境变量
        for(const auto &[name,value]:_env_vars){
            setenv(name.c_str(),value.c_str(),1);
        }
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
        _status=ERROR;
        throw std::runtime_error(name+":子程序运行失败！");
    }
    _status=RUNNING;
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
    _status=STOP;
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

    // 设置非阻塞模式以避免无限等待
    int flags;
    if(!_blocked){
        flags=fcntl(stdpipe,F_GETFL,0);
        fcntl(stdpipe,F_SETFL,flags|O_NONBLOCK);
    }

    char ch;
    int nbytes;
    nbytes=::read(stdpipe,&ch,1);

    // 恢复阻塞模式
    if(!_blocked){
        fcntl(stdpipe,F_SETFL,flags);
    }

    if(nbytes>0) return ch;
    else return 0;
}

string pc::Process::read_line(TypeOut type){
    // 获取合适的管道文件描述符
    int stdpipe=(type==OUT)?_stdout[0]:_stderr[0];
    if(stdpipe==-1) return "";

    // 设置非阻塞模式以避免无限等待
    int flags;
    if(!_blocked){
        flags=fcntl(stdpipe,F_GETFL,0);
        fcntl(stdpipe,F_SETFL,flags|O_NONBLOCK);
    }

    string line;
    char c;
    // 有数据可读，开始读取一行数据
    while(::read(stdpipe,&c,1)>0){
        // 遇到换行符结束
        if(c=='\n') break;
        // 将字符添加到结果中
        line+=c;
    }
    _empty=(line=="")?true:false;


    // 恢复阻塞模式
    if(!_blocked){
        fcntl(stdpipe,F_SETFL,flags);
    }
    return line;
}

char pc::Process::getc(){
    return read_char(OUT);
}

string pc::Process::getline(){
    return read_line(OUT);
}

bool pc::Process::empty(){
    return _empty;
}

void pc::Process::setBlock(bool status){
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
            _status=STOP;
        }
        return true;
    }
    return false;
}

// 新增：检查进程是否在运行
bool pc::Process::is_running(){
    if(_pid<=0){
        _status=STOP;
        return false;
    }

    // 发送信号0检查进程是否存在
    int result=::kill(_pid,0);

    if(result==0){
        // 进程存在
        _status=RUNNING;
        return true;
    }
    else{
        // 检查错误类型
        if(errno==ESRCH){
            // 进程不存在
            _pid=-1;
            _status=STOP;
            return false;
        }
        else if(errno==EPERM){
            // 没有权限，但进程可能存在
            _status=RUNNING;
            return true;
        }
    }

    // 其他错误情况
    _status=ERROR;
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
    if(is_running()){
        kill(SIGTERM);
    }
    close();
    close_pipe(1);
    wait();
}

pc::Process &pc::Process::set_env(const std::string &name,const std::string &value){
    _env_vars[name]=value;
    return *this;
}

std::string pc::Process::get_env(const std::string &name) const{
    auto it=_env_vars.find(name);
    if(it!=_env_vars.end()){
        return it->second;
    }
    // 尝试获取当前进程的环境变量
    const char *val=getenv(name.c_str());
    return val?val:"";
}

void pc::Process::unset_env(const std::string &name){
    _env_vars.erase(name);
}

void pc::Process::clear_env(){
    _env_vars.clear();
}