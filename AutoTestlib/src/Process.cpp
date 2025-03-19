#include "Process.h"
#include "sysapi.h"
#include <sstream>
#include <sys/resource.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

namespace process{
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
            if(execvp(arg,args)==-1){
                exit(EXIT_FAILURE);
            }
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
        try{
            init_pipe();
            launch(_path.c_str(),_args.data());
        }
        catch(const std::exception &e){
            // 记录错误并重新抛出
            std::cerr<<"Process启动失败: "<<e.what()<<std::endl;
            throw;
        }
    }

    Status Process::wait(){
        int status;
        waitpid(_pid,&status,0);
        _exit_code=status;
        _pid=-1;
        if(_status==TIMEOUT){
            return _status;
        }
        else if(WIFEXITED(status)){
            int temp=WEXITSTATUS(status);
            if(temp==0){
                _status=STOP;
                return _status;
            }
            else{
                _status=ERROR;
                return _status;
            }
        }
        else if(WIFSIGNALED(status)){
            int signal=WTERMSIG(status);
            _status=RE;
            return _status;
        }
        else{
            _status=RE;
            return _status;
        }
        // 停止计时
        if(_status!=RUNNING){
            _timer.stop();
        }
    }

    int Process::get_exit_code() const{
        return _exit_code;
    }

    Status Process::get_status() const{
        return _status;
    }

    Process &Process::write(const string &data){
        if(_stdin.is_closed()){
            throw std::runtime_error(name+":进程写入错误！");
        }

        // 使用Pipe类的write方法直接写入字符串数据
        _stdin.write(data);
        return *this;
    }

    string Process::read(PipeType type,size_t nbytes){
        Pipe &pipe=(type==PIPE_OUT)?_stdout:_stderr;
        return pipe.read_all(nbytes);
    }

    char Process::read_char(PipeType type){
        Pipe &pipe=(type==PIPE_OUT)?_stdout:_stderr;
        // 利用Pipe类的read_char方法
        return pipe.read_char();
    }

    string Process::read_line(PipeType type,char delimiter){
        Pipe &pipe=(type==PIPE_OUT)?_stdout:_stderr;
        // 利用Pipe类的read_line方法
        string line=pipe.read_line(delimiter);
        _empty=line.empty();
        return line;
    }
    string Process::getline(char delimiter){
        return read_line(PIPE_OUT,delimiter);
    }
    string Process::geterr(size_t nbytes){
        if(nbytes==0){
            // 行读
            string result;
            while(!_stderr.empty()){
                result+=_stderr.read_line('\n');
            }
            // 如果没有数据，返回空字符串
            result+=(result.empty()?"":"\n");
            return result;
        }
        // 字节读
        return read(PIPE_ERR,nbytes);
    }
    char Process::getchar(){
        return read_char(PIPE_OUT);
    }

    bool Process::empty(PipeType type){
        // 检查管道数据是否为空
        if(type==PIPE_OUT){
            return _stdout.empty();
        }
        else if(type==PIPE_ERR){
            return _stderr.empty();
        }
        else{
            throw std::invalid_argument(name+":无法检测对应管道是非为空！");
        }
    }

    void Process::set_block(bool status){
        // 设置所有管道的阻塞状态
        _stdin.set_blocked(status);
        _stdout.set_blocked(status);
        _stderr.set_blocked(status);
    }

    void Process::set_flush(int timeout_ms){
        _flushTime=timeout_ms;
    }

    void Process::set_buffer_size(size_t size){
        // 设置所有管道的缓冲区大小
        _stdin.set_buffer_size(size);
        _stdout.set_buffer_size(size);
        _stderr.set_buffer_size(size);
    }

    void Process::close(PipeType type){
        if(type==PIPE){
            _stdin.close();
            _stdout.close();
            _stderr.close();
        }
        else if(type==PIPE_IN){
            _stdin.close();
        }
        else if(type==PIPE_OUT){
            _stdout.close();
        }
        else if(type==PIPE_ERR){
            _stderr.close();
        }
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
                _status=wait();
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