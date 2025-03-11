#include "sysapi.h"
#include <string>

namespace process{
    // 管道类型枚举
    enum PipeType{ PIPE_READ=0,PIPE_WRITE=1 };

    // 管道类
    class Pipe{
    private:
        // 管道句柄
        Handle _pipe[2];
        // 管道阻塞信号
        int _flags;
        // 管道类型 1 为写，0 为读
        bool _type=PIPE_READ;
        // 管道阻塞模式
        int _isBlocked=true;
        // 缓冲区大小
        int _bufferSize=4096;
    public:
        // 构造函数,创建管道
        Pipe();
        // 析构函数,关闭管道
        ~Pipe();
        // 手动创建管道
        void create();
        // 关闭管道
        void close();
        // 设置管道阻塞模式
        void set_blocked(bool isblocked);
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
        bool empty(int timeout_ms=0);
        // 管道是否关闭
        bool is_closed();
        // 获取管道句柄
        Handle get_handle();
        Handle operator[](int index);
        // 底层接口：读取数据
        int read(char buffer[],int size);
        // 底层接口：写入数据
        int write(const char data[],int size);

        // 读取一个字符
        char read_char();
        // 读取一行数据
        std::string read_line(char delimiter='\n');
        // 读取所有可用数据
        std::string read_all();
        // 写入字符串
        void write(const std::string &data);
    };
}