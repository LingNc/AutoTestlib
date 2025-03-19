#include "Pipe.h"
#include <stdexcept>
#include <errno.h>
#include <string.h>
#include <poll.h>

namespace process{
    // 单位转换函数实现
    size_t GB(size_t size){
        return size*1024*1024*1024;
    }
    size_t MB(size_t size){
        return size*1024*1024;
    }
    size_t KB(size_t size){
        return size*1024;
    }
    // 管道类实现
    Pipe::Pipe(){
        _pipeType=false;
        _isBlocked=true;
        create();
    }
    Pipe::~Pipe(){
        close();
    }
    // 创建管道
    void Pipe::create(){
        if(::pipe(_pipe)==-1){
            throw std::runtime_error("Failed to create pipe");
        }
        _pipeType=true;
    }
    // 重新创建管道
    void Pipe::recreate(){
        close();
        _pipeType=false;
        set_blocked(true);
        _bufferSize=KB(4);
        create();
    }
    // 关闭管道
    void Pipe::close(){
        if(!is_closed(PIPE_READ)){
            ::close(_pipe[PIPE_READ]);
        }
        if(!is_closed(PIPE_WRITE)){
            ::close(_pipe[PIPE_WRITE]);
        }
    }
    // 设置阻塞模式
    void Pipe::set_blocked(bool isblocked){
        _isBlocked=isblocked;
        if(_isBlocked){
            _flags=fcntl(_pipe[_pipeType],F_GETFL);
            fcntl(_pipe[_pipeType],F_SETFL,_flags&~O_NONBLOCK);
        }
        else{
            _flags=fcntl(_pipe[_pipeType],F_GETFL);
            fcntl(_pipe[_pipeType],F_SETFL,_flags|O_NONBLOCK);
        }
    }
    // 设置非缓冲检查时间
    void Pipe::set_flush(size_t timeout_ms){
        if(timeout_ms<=0){
            throw std::invalid_argument("非缓冲检查时间必须大于 0");
        }
        _flushTime=timeout_ms;
    }
    // 重定向到输入输出
    void Pipe::redirect(Handle pipe){
        if(_pipe[_pipeType]!=-1){  // 修正: 只有管道打开时才能重定向
            if(::dup2(_pipe[_pipeType],pipe)==-1){  // _pipe[_pipeType]是源，pipe是目标
                throw std::runtime_error("Failed to redirect pipe");
            }
        }
        else{
            throw std::runtime_error("Cannot redirect: pipe is closed");
        }
    }
    // 是否是阻塞模式
    bool Pipe::is_blocked(){
        return _isBlocked;
    }
    // 设置管道类型
    void Pipe::set_type(bool type,bool autoClose){
        _pipeType=type;
        if(autoClose){
            // 关闭另一个管道
            ::close(_pipe[!_pipeType]);
        }
    }
    // 设置缓冲区大小
    void Pipe::set_buffer_size(int size){
        if(size<=0){
            throw std::invalid_argument("Buffer size must be greater than 0");
        }
        // 设置管道缓冲区大小
        fcntl(_pipe[_pipeType],F_SETPIPE_SZ,size);
        _bufferSize=size;
    }
    // 获取管道类型
    bool Pipe::get_type(){
        return _pipeType;
    }
    // 是否为空
    bool Pipe::empty(){
        if(is_closed()){
            return true; // 管道未打开，认为是空的
        }

        struct pollfd pfd;
        pfd.fd=_pipe[_pipeType];
        pfd.events=POLLIN;

        int ret=poll(&pfd,1,_flushTime);

        if(ret<0){
            throw std::runtime_error("Failed to poll pipe: "+std::string(strerror(errno)));
        }

        // 修正：ret > 0 且 pfd.revents & POLLIN 表示有数据可读，因此不为空
        return !(ret>0&&(pfd.revents&POLLIN));
    }
    // 管道是否关闭
    bool Pipe::is_closed(PipeType type){
        auto nowPipe=(type==PIPE)?_pipe[_pipeType]:_pipe[type];
        int flags=fcntl(nowPipe,F_GETFD);
        if(flags==-1&&errno==EBADF){
            return true; // 管道已关闭
        }
        return false;
    }
    // 获取管道句柄
    Handle Pipe::get_handle(){
        return _pipe[_pipeType];
    }
    Handle Pipe::operator[](int index){
        if(index<0||index>1){
            throw std::out_of_range("Index out of range");
        }
        return _pipe[index];
    }
    // 读取指定大小的数据
    ssize_t Pipe::read(void *buffer,size_t size){
        if(is_closed(PIPE_READ)){
            throw std::runtime_error("管道已关闭，无法读取数据");
        }
        return ::read(_pipe[PIPE_READ],buffer,size);
    }
    // 写入指定大小的数据
    ssize_t Pipe::write(const void *data,size_t size){
        if(is_closed(PIPE_WRITE)){
            throw std::runtime_error("管道已关闭，无法写入数据");
        }
        return ::write(_pipe[PIPE_WRITE],data,size);
    }

    // 新增方法: 读取一个字符
    char Pipe::read_char(){
        if(_pipeType==PIPE_NO){
            throw std::runtime_error("管道未被初始化为特定模式！");
        }
        char c;
        int bytes_read=read(&c,1);
        if(bytes_read<0){
            if(errno==EAGAIN||errno==EWOULDBLOCK){
                return '\0'; // 非阻塞模式下没有数据
            }
            throw std::runtime_error("Failed to read from pipe: "+std::string(strerror(errno)));
        }
        else if(bytes_read==0){
            return '\0'; // 管道已关闭
        }
        return c;
    }

    // 新增方法: 读取一行数据
    std::string Pipe::read_line(char delimiter){
        if(_pipeType==PIPE_NO){
            throw std::runtime_error("管道未被初始化为特定模式！");
        }
        std::string line;
        char c;

        while(true){
            if(_isBlocked==false&&empty()){
                // 如果非阻塞模式下没有数据可读，直接返回空字符串
                return "";
            }
            int bytes_read=read(&c,1);
            if(bytes_read<0){
                if(errno==EAGAIN||errno==EWOULDBLOCK){
                    // 没有更多数据了
                    break;
                }
                throw std::runtime_error("Failed to read from pipe: "+std::string(strerror(errno)));
            }
            else if(bytes_read==0){
                // 管道已关闭
                break;
            }

            if(c==delimiter){
                // 遇到分隔符，结束读取
                break;
            }
            line+=c;
        }

        return line;
    }

    // 新增方法: 读取所有可用数据
    std::string Pipe::read_all(size_t nbytes){
        if(nbytes!=0) return read_bytes(nbytes);
        if(_pipeType==PIPE_NO){
            throw std::runtime_error("管道未被初始化为特定模式！");
        }
        if(is_closed()){
            return "";
        }
        char *buffer=new char[_bufferSize];
        if(buffer==nullptr){
            throw std::runtime_error("管道缓存区读取内存分配失败");
        }
        std::string result;

        // 记录原始阻塞状态
        bool original_blocked=_isBlocked;
        try{
            // 临时设置为非阻塞模式
            if(original_blocked){
                set_blocked(false);
            }
            while(true){
                // 无论什么模式，都先检查是否有数据
                if(empty()){
                    break;
                }
                int bytes_read=read(buffer,_bufferSize-1);
                if(bytes_read<=0){
                    if(errno==EAGAIN||errno==EWOULDBLOCK){
                        continue; // 尝试再次读取
                    }
                    break; // 其他错误或管道已关闭
                }
                buffer[bytes_read]='\0';
                result+=buffer;
            }
            // 恢复原始阻塞状态
            if(original_blocked){
                set_blocked(true);
            }
        }
        catch(...){
            // 恢复原始阻塞状态
            if(original_blocked){
                set_blocked(true);
            }
            delete[] buffer;
            throw;
        }

        delete[] buffer;
        return result;
    }

    // 实现
    std::string Pipe::read_bytes(size_t bytes){
        if(_pipeType==PIPE_NO){
            throw std::runtime_error("管道未被初始化为特定模式！");
        }

        std::string result;
        result.reserve(bytes);
        size_t total_read=0;
        char buffer[_bufferSize]; // 读取缓冲区

        while(total_read<bytes){
            // 检查是否有数据可读
            if(_isBlocked==false&&empty()){
                break; // 非阻塞模式下没有更多数据
            }

            // 计算本次读取大小
            size_t to_read=std::min(bytes-total_read,sizeof(buffer));
            ssize_t read_bytes=read(buffer,to_read);

            if(read_bytes<=0){
                if(errno==EAGAIN||errno==EWOULDBLOCK){
                    continue; // 非阻塞模式下暂时没有数据
                }
                break; // 出错或管道已关闭
            }

            result.append(buffer,read_bytes);
            total_read+=read_bytes;
        }

        return result;
    }

    // 新增方法: 写入字符串
    void Pipe::write(const std::string &data){
        write(data.c_str(),data.length());
    }
    // 重载运算符
    Pipe &Pipe::operator<<(std::ostream &(*pf)(std::ostream &)){
        if(pf==static_cast<std::ostream&(*)(std::ostream &)>(std::endl)){
            write("\n");
        }
        return *this;
    }
}
