#include "Pipe.h"
#include <stdexcept>
#include <errno.h>
#include <string.h>
#include <poll.h>

namespace process{
    // 管道类实现
    Pipe::Pipe(){
        _type=false;
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
        _type=true;
    }
    // 关闭管道
    void Pipe::close(){
        if(_pipe[0]!=-1){
            ::close(_pipe[0]);
            _pipe[0]=-1;
        }
        if(_pipe[1]!=-1){
            ::close(_pipe[1]);
            _pipe[1]=-1;
        }
    }
    // 设置阻塞模式
    void Pipe::set_blocked(bool isblocked){
        _isBlocked=isblocked;
        if(_isBlocked){
            _flags=fcntl(_pipe[_type],F_GETFL);
            fcntl(_pipe[_type],F_SETFL,_flags&~O_NONBLOCK);
        }
        else{
            _flags=fcntl(_pipe[_type],F_GETFL);
            fcntl(_pipe[_type],F_SETFL,_flags|O_NONBLOCK);
        }
    }
    // 重定向到输入输出
    void Pipe::redirect(Handle pipe){
        if(_pipe[_type]!=-1){  // 修正: 只有管道打开时才能重定向
            if(::dup2(_pipe[_type],pipe)==-1){  // _pipe[_type]是源，pipe是目标
                throw std::runtime_error("Failed to redirect pipe");
            }
        }
        else {
            throw std::runtime_error("Cannot redirect: pipe is closed");
        }
    }
    // 是否是阻塞模式
    bool Pipe::is_blocked(){
        return _isBlocked;
    }
    // 设置管道类型
    void Pipe::set_type(bool type){
        _type=type;
        ::close(_pipe[!_type]);
        _pipe[!_type]=-1;
    }
    // 设置缓冲区大小
    void Pipe::set_buffer_size(int size){
        if(size<=0){
            throw std::invalid_argument("Buffer size must be greater than 0");
        }
        _bufferSize=size;
    }
    // 获取管道类型
    bool Pipe::get_type(){
        return _type;
    }
    // 是否为空
    bool Pipe::empty(int timeout_ms){
        if(_pipe[_type]==-1){
            return true; // 管道未打开，认为是空的
        }

        struct pollfd pfd;
        pfd.fd=_pipe[_type];
        pfd.events=POLLIN;

        int ret=poll(&pfd,1,timeout_ms);

        if(ret<0){
            throw std::runtime_error("Failed to poll pipe: "+std::string(strerror(errno)));
        }

        // 修正：ret > 0 且 pfd.revents & POLLIN 表示有数据可读，因此不为空
        return !(ret > 0 && (pfd.revents & POLLIN));
    }
    // 管道是否关闭
    bool Pipe::is_closed(){
        return _pipe[_type]==-1;
    }
    // 获取管道句柄
    Handle Pipe::get_handle(){
        return _pipe[_type];
    }
    Handle Pipe::operator[](int index){
        if(index<0||index>1){
            throw std::out_of_range("Index out of range");
        }
        return _pipe[index];
    }
    // 读取指定大小的数据
    int Pipe::read(char buffer[],int size){
        int bytes_read=::read(_pipe[_type],buffer,size);
        buffer[bytes_read]='\0';
        return bytes_read;
    }
    // 写入指定大小的数据
    int Pipe::write(const char data[],int size){
        int bytes_written=::write(_pipe[_type],data,size);
        return bytes_written;
    }

    // 新增方法: 读取一个字符
    char Pipe::read_char(){
        if(_pipe[_type]==-1){
            throw std::runtime_error("Pipe is not open");
        }

        char c;
        int bytes_read=::read(_pipe[_type],&c,1);
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
    std::string Pipe::read_line(char delimiter,int timeout_ms){
        if(_pipe[_type]==-1){
            throw std::runtime_error("Pipe is not open");
        }

        std::string line;
        char c;

        while(true){
            if(_isBlocked==false){
                if(empty(timeout_ms)){
                    // 如果非阻塞模式下没有数据可读，直接返回空字符串
                    return "";
                }
            }
            int bytes_read=::read(_pipe[_type],&c,1);
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
    std::string Pipe::read_all(){
        if(_pipe[_type]==-1){
            throw std::runtime_error("Pipe is not open");
        }
        char buffer[_bufferSize];
        std::string result;

        while(true){
            int bytes_read=::read(_pipe[_type],buffer,_bufferSize-1);
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

            buffer[bytes_read]='\0';
            result+=buffer;
        }
        return result;
    }

    // 新增方法: 写入字符串
    void Pipe::write(const std::string &data){
        write(data.c_str(),data.length());
    }
}
