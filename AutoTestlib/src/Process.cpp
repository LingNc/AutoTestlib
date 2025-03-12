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
        // 正确设置运行标志
        running=true;
        thread=std::thread([this,timeout_ms,callback](){
            std::unique_lock<std::mutex> lock(mutex);
            // 使用条件变量的wait_for，可随时被唤醒
            if(cv.wait_for(lock,std::chrono::milliseconds(timeout_ms),
                [this]{ return !running; })){
                // 如果被提前唤醒且running=false，直接退出
                return;
            }
            // 超时且仍在运行，执行回调
            if(running){
                callback();
            }
            });
    }

    void Timer::stop(){
        {
            std::lock_guard<std::mutex> lock(mutex);
            running=false;
        }
        // 唤醒等待的线程
        cv.notify_all();

        // 仍然需要join，但现在线程会立即退出
        if(thread.joinable()){
            thread.join();
        }
    }

    // 进程类
    Process &Process::set_timeout(int timeout_ms){
        if(timeout_ms<=0){
            throw std::invalid_argument(name+":超时设置错误！");
        }
        _timelimit=timeout_ms;
        return *this;
    }

    Process &Process::cancel_timeout(){
        _timer.stop();
        _timelimit=0;
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

    void Process::start_timer(){
        if(_timelimit>0){
            _timer.stop();  // 确保停止任何现有的计时器
            _timer.start(_timelimit,[this](){
                if(is_running()){
                    _status=TIMEOUT;  // 设置状态为超时
                    ::kill(_pid,SIGKILL);      // 发送终止信号
                }
                });
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

            // 通讯 进程开始
            _child_message.set_type(PIPE_WRITE);
            _child_message<<"Start"<<std::endl;

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
        _child_message.set_type(PIPE_READ);
        // 接受子进程的开始信号
        // if(!_child_message.is_blocked()){
        //     _child_message.set_blocked(true);
        // }
        string start_signal=_child_message.read_line('\n');
        // if(!_child_message.is_blocked()){
        //     _child_message.set_blocked(false);
        // }
        if(start_signal!="Start"){
            _status=ERROR;
            throw std::runtime_error(name+":未获取到开始信号，子程序启动失败！");
        }
        // 开始计时
        if(_timelimit>0){
            start_timer();
        }
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

        // 首先检查是否已经标记为超时
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
                // 修复逻辑运算符错误，使用单独的case
            case SIGABRT:
                return MemoryLimitExceeded;
            case SIGKILL:
                // 如果已经标记为TIMEOUT，则返回超时错误
                if(_status==TIMEOUT){
                    return TimeLimitEXceeded;
                }
                return MemoryLimitExceeded;
            case SIGFPE:
                return FloatingPointError;
            default:
                return RuntimeError;
            }
        }
        else{
            _status=RE;
            return RuntimeError;
        }
        // 停止计时
        if(_status!=RUNNING){
            _timer.stop();
        }
    }

    int Process::get_exit_code() const{
        return _exit_code;
    }

    Process &Process::write(const string &data){
        if(_stdin.is_closed()){
            throw std::runtime_error(name+":进程写入错误！");
        }

        // 使用Pipe类的write方法直接写入字符串数据
        _stdin.write(data);
        return *this;
    }

    string Process::read(TypeOut type){
        Pipe &pipe=(type==OUT)?_stdout:_stderr;
        // 利用Pipe类的read_all方法获取所有可用数据
        return pipe.read_all();
    }

    char Process::read_char(TypeOut type){
        Pipe &pipe=(type==OUT)?_stdout:_stderr;
        // 利用Pipe类的read_char方法
        return pipe.read_char();
    }

    string Process::read_line(TypeOut type,char delimiter){
        Pipe &pipe=(type==OUT)?_stdout:_stderr;
        // 利用Pipe类的read_line方法
        string line=pipe.read_line(delimiter,_unblock_timeout);
        _empty=line.empty();
        return line;
    }
    string Process::getline(char delimiter){
        return read_line(OUT,delimiter);
    }

    char Process::getchar(){
        return read_char(OUT);
    }

    bool Process::empty(int timeout_ms){
        return _stdout.empty(timeout_ms);
    }

    void Process::set_block(bool status){
        // 设置所有管道的阻塞状态
        _stdin.set_blocked(status);
        _stdout.set_blocked(status);
        _stderr.set_blocked(status);
    }

    void Process::set_unblock_time(int timeout_ms){
        _unblock_timeout=timeout_ms;
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
                // 关闭管道
                _code=wait();
                close();
                if(_status!=TIMEOUT){
                    _status=STOP;
                }
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