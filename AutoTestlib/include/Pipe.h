#include "sysapi.h"
#include <string>

namespace process{
    // 管道类型枚举
    enum PipeType{
        PIPE_READ=0,
        PIPE_WRITE,
        PIPE_NO,
        PIPE,
        PIPE_IN,
        PIPE_OUT,
        PIPE_ERR
    };
    // 单位转换
    size_t GB(size_t size);
    size_t MB(size_t size);
    size_t KB(size_t size);
    // 管道类
    class Pipe{
    private:
        // 管道句柄
        Handle _pipe[2];
        // 管道阻塞信号
        int _flags;
        // 管道类型 1 为写，0 为读
        int _pipeType=PIPE_NO;
        // 管道阻塞模式
        int _isBlocked=true;
        // 缓冲区大小
        int _bufferSize=KB(4);
        // 刷新时间
        int _flushTime=100;
    public:
        // 构造函数,创建管道
        Pipe();
        // 析构函数,关闭管道
        ~Pipe();
        // 手动创建管道
        void create();
        // 关闭管道
        void close();
        // 重设管道
        void recreate();
        // 设置管道阻塞模式
        void set_blocked(bool isblocked);
        // 设置非缓冲检查时间
        void set_flush(size_t timeout_ms);
        // 重定向到输入输出
        void redirect(Handle pipe);
        // 是否是阻塞模式
        bool is_blocked();
        // 设置管道类型
        void set_type(bool type);
        // 设置缓冲区大小
        void set_buffer_size(int size);
        // 获取管道类型
        bool get_type();
        // 检查是否有数据可读
        bool empty();
        // 管道是否关闭
        bool is_closed(PipeType type=PIPE);
        // 获取管道句柄
        Handle get_handle();
        Handle operator[](int index);
        // 底层接口：读取数据
        ssize_t read(void *buffer,size_t size);
        // 底层接口：写入数据
        ssize_t write(const void *data,size_t size);
        // 读取一个字符
        char read_char();
        // 读取一行数据
        std::string read_line(char delimiter='\n');
        // 读取指定大小的数据
        std::string read_bytes(size_t bytes);
        // 读取所有可用数据
        std::string read_all(size_t nbytes=0);
        // 写入字符串
        void write(const std::string &data);
        // 重载运算符
        template<typename T>
        Pipe &operator<<(const T &data){
            std::stringstream ss;
            ss<<data;
            write(ss.str());
            return *this;
        }
        Pipe &operator<<(std::ostream &(*pf)(std::ostream &));

        template<typename T>
        Pipe &operator>>(T &data){
            std::stringstream ss;
            ss<<read_all();
            ss>>data;
            return *this;
        }
    };
}