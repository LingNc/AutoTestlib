#ifndef ARGS_H
#define ARGS_H

#include "Self.h"
#include <vector>

namespace process{
    // 参数类
    class Args{
    private:
        // 存储参数的容器
        std::vector<string> _args;
        // 转换后的C风格参数，用于execvp
        std::vector<char *> _c_args;

        // 转换函数，将string参数转换为C风格参数
        void prepare_c_args();

    public:
        // 构造函数
        Args();
        // 读取参数列表
        Args(const std::vector<string> &arguments);
        // 读取命令行，或者程序名
        Args(const string &command);

        // 添加参数
        Args &add(const string &arg);
        // 添加多个参数
        Args &add(const std::vector<string> &arguments);
        // 设置程序名称（第一个参数）
        Args &set_program_name(const string &name);
        // 新增：解析命令行字符串
        Args &parse(const string &command_line);
        // 清除所有参数
        void clear();
        // 获取参数数量
        size_t size() const;
        // 获取C风格参数，用于execvp
        char **data();
        // 获取所有参数的复制
        std::vector<string> get_args() const;
        // 获取程序名称
        string get_program_name() const;
        // 获取指定位置的参数
        string &operator[](size_t index);
        const string &operator[](size_t index) const;
    };
}

#endif // ARGS_H