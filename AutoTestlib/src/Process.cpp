#include "Process.h"
#include "sysapi.h"
#include <sstream>
#include <sys/resource.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

namespace process{
    // Args类实现
    Args::Args(){}

    Args::Args(const std::vector<string> &arguments): _args(arguments){
        prepare_c_args();
    }

    Args::Args(const string &command){
        parse(command);
        prepare_c_args();
    }


    Args &Args::add(const string &arg){
        _args.push_back(arg);
        prepare_c_args();
        return *this;
    }

    Args &Args::add(const std::vector<string> &arguments){
        _args.insert(_args.end(),arguments.begin(),arguments.end());
        prepare_c_args();
        return *this;
    }

    Args &Args::set_program_name(const string &name){
        if(_args.empty()){
            _args.push_back(name);
        }
        else{
            _args[0]=name;
        }
        prepare_c_args();
        return *this;
    }

    void Args::clear(){
        _args.clear();
        _c_args.clear();
    }

    size_t Args::size() const{
        return _args.size();
    }

    char **Args::data(){
        prepare_c_args();
        return _c_args.data();
    }

    std::vector<string> Args::get_args() const{
        return _args;
    }

    string Args::get_program_name() const{
        if(!_args.empty()){
            return _args[0];
        }
        return "";
    }

    string &Args::operator[](size_t index){
        return _args[index];
    }

    const string &Args::operator[](size_t index) const{
        return _args[index];
    }

    void Args::prepare_c_args(){
        _c_args.clear();
        for(const auto &arg:_args){
            _c_args.push_back(const_cast<char *>(arg.c_str()));
        }
        _c_args.push_back(nullptr); // execvp需要以nullptr结尾
    }

    // 新增：解析命令行字符串
    Args &Args::parse(const string &command_line){
        enum State{ NORMAL,IN_QUOTE,IN_DQUOTE };
        State state=NORMAL;
        string current_arg;
        bool escaped=false;

        _args.clear(); // 清除旧的参数

        for(char c:command_line){
            if(escaped){
                // 处理转义字符 - 把原始的反斜杠和字符都添加到参数中
                current_arg+='\\';
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
                    current_arg+=c;  // 将单引号添加到参数中
                    state=IN_QUOTE;
                }
                else if(c=='\"'){
                    current_arg+=c;  // 将双引号添加到参数中
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
                if(c=='\\'){
                    escaped=true;  // 允许在单引号内转义
                }
                else{
                    current_arg+=c;  // 将所有字符添加到参数中，包括单引号
                    if(c=='\''&&!escaped){
                        state=NORMAL;
                    }
                }
                break;

            case IN_DQUOTE:
                if(c=='\\'){
                    escaped=true;  // 允许在双引号内转义
                }
                else{
                    current_arg+=c;  // 将所有字符添加到参数中，包括双引号
                    if(c=='\"'&&!escaped){
                        state=NORMAL;
                    }
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

    // 计时器类
    Timer::~Timer(){
        stop();
    }

    void Timer::start(int timeout_ms,std::function<void()> callback){
        stop();
        running=true;
        thread=std::thread([this,timeout_ms,callback](){
            // 使用 sleep_for 等待指定时间
            std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
            if(running){
                callback();
            }
            });
    }

    void Timer::stop(){
        running=false;
        if(thread.joinable()){
            thread.join();
        }
    }

    // 进程类
    Process &Process::set_timeout(int timeout_ms){
        _timer.start(timeout_ms,[this](){
            if(is_running()){
                kill(SIGKILL);
                _status=TIMEOUT;
            }
            });
        return *this;
    }

    Process &Process::cancel_timeout(){
        _timer.stop();
        return *this;
    }

    Process &Process::set_memout(int memout_mb){
        _memsize=memout_mb;
        return *this;
    }

    Process &Process::cancel_memout(){
        _memsize=0;
        return *this;
    }

    void Process::init_pipe(){
        // 创建管道
        if(_stdin.is_closed()||_stdout.is_closed()||_stderr.is_closed()){
            throw std::runtime_error(name+":管道创建错误！");
        }
    }

    void Process::launch(const char arg[],char *args[]){
        _pid=fork();
        // 子进程
        if(_pid==0){
            // 设置环境变量
            for(const auto &[name,value]:_env_vars){
                setenv(name.c_str(),value.c_str(),1);
            }

            // 限制内存大小
            if(_memsize!=0){
                struct rlimit rl;
                rl.rlim_cur=_memsize*1024*1024; // 软限制
                rl.rlim_max=_memsize*1024*1024; // 硬限制

                if(setrlimit(RLIMIT_AS,&rl)==-1){
                    perror("setrlimit failed");
                    exit(EXIT_FAILURE);
                }
            }

            // 设置管道
            _stdin.set_type(PIPE_READ);
            _stdout.set_type(PIPE_WRITE);
            _stderr.set_type(PIPE_WRITE);

            // 输入输出重定向
            _stdin.redirect(STDIN_FILENO);
            _stdout.redirect(STDOUT_FILENO);
            _stderr.redirect(STDERR_FILENO);

            // 运行子程序
            execvp(arg,args);
            exit(EXIT_FAILURE);
        }
        else if(_pid<0){
            _status=ERROR;
            throw std::runtime_error(name+":子程序运行失败！");
        }
        _status=RUNNING;
        _stdin.set_type(PIPE_WRITE);
        _stdout.set_type(PIPE_READ);
        _stderr.set_type(PIPE_READ);
    }
    Process::Process(){}

    Process::Process(const string &path,const Args &args): _args(args){
        _path=path;
        if(args.size()>0){
            name=args.get_program_name();
        }
    }

    void Process::load(const string &path,const Args &args){
        _path=path;
        _args=args;
        if(args.size()>0){
            name=args.get_program_name();
        }
    }

    void Process::start(){
        init_pipe();
        launch(_path.c_str(),_args.data());
    }

    JudgeCode Process::wait(){
        int status;
        waitpid(_pid,&status,0);
        _exit_code=status;
        _pid=-1;
        if(_status==TIMEOUT){
            return TimeLimitEXceeded;
        }
        else if(WIFEXITED(status)){
            int temp=WEXITSTATUS(status);
            if(temp==0){
                _status=STOP;
                return Waiting;
            }
            else{
                _status=ERROR;
                return Waiting;
            }
        }
        else if(WIFSIGNALED(status)){
            int signal=WTERMSIG(status);
            _status=RE;
            switch(signal){
            case SIGSEGV:
                return RuntimeError;
                break;
            case SIGABRT||SIGKILL:
                return MemoryLimitExceeded;
                break;
            case SIGFPE:
                return FloatingPointError;
                break;
            default:
                return RuntimeError;
            }
        }
        else{
            _status=RE;
            return RuntimeError;
        }
    }

    int Process::get_exit_code() const{
        return _exit_code;
    }

    Process &Process::write(const string &data){
        if(_stdin.is_closed()){
            throw std::runtime_error(name + ":进程写入错误！");
        }

        // 使用Pipe类的write方法直接写入字符串数据
        _stdin.write(data);
        return *this;
    }

    string Process::read(TypeOut type){
        Pipe& pipe = (type == OUT) ? _stdout : _stderr;
        // 利用Pipe类的read_all方法获取所有可用数据
        return pipe.read_all();
    }

    char Process::read_char(TypeOut type){
        Pipe& pipe = (type == OUT) ? _stdout : _stderr;
        // 利用Pipe类的read_char方法
        return pipe.read_char();
    }

    string Process::read_line(TypeOut type){
        Pipe& pipe = (type == OUT) ? _stdout : _stderr;
        // 利用Pipe类的read_line方法
        string line = pipe.read_line();
        _empty = line.empty();
        return line;
    }
    string Process::getline(){
        return read_line(OUT);
    }
    char Process::getchar(){
        return read_char(OUT);
    }
    bool Process::empty(){
        return _stdout.empty();
    }

    void Process::set_block(bool status){
        // 设置所有管道的阻塞状态
        _stdin.set_blocked(status);
        _stdout.set_blocked(status);
        _stderr.set_blocked(status);
    }

    void Process::close(){
        _stdin.close();
        _stdout.close();
        _stderr.close();
    }

    bool Process::kill(int signal){
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
    bool Process::is_running(){
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

    Process &Process::operator<<(std::ostream &(*pf)(std::ostream &)){
        if(pf==static_cast<std::ostream&(*)(std::ostream &)>(std::endl)){
            write("\n");
            flush();
        }
        return *this;
    }

    Process &Process::flush(){
        return *this;
    }

    Process::~Process(){
        if(is_running()){
            kill(SIGTERM);
        }
        close();
        wait();
    }

    Process &Process::set_env(const std::string &name,const std::string &value){
        _env_vars[name]=value;
        return *this;
    }

    std::string Process::get_env(const std::string &name) const{
        auto it=_env_vars.find(name);
        if(it!=_env_vars.end()){
            return it->second;
        }
        // 尝试获取当前进程的环境变量
        const char *val=getenv(name.c_str());
        return val?val:"";
    }

    void Process::unset_env(const std::string &name){
        _env_vars.erase(name);
    }

    void Process::clear_env(){
        _env_vars.clear();
    }
}