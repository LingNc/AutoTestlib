#ifndef AUTOCONFIG_H
#define AUTOCONFIG_H

#include <fstream>
#include "Self.h"
#include "json.hpp"

// 标签转换
#define str(label) #label

namespace acm{
    using nlohmann::json;
    // 配置文件标识
    enum ConfigSign{
        Allow_Path, //> 跟随路径
        AC_Path, //> AC代码路径
        Test_Path, //> 测试代码路径
        Problem_Path, //> 题目路径
    };
    // 配置类
    class AutoConfig{
        json _config;
        fs::path _filePath;
    public:
        // 构造函数
        AutoConfig();
        AutoConfig(const fs::path &file);
        // 设置路径
        void set_path(const fs::path &file);
        // 检查配置文件是否存在且不为空
        bool exist();
        // 保存到配置文件
        void save();
        // 操作符
        template<typename T>
        json &operator[](T key){
            return _config[key];
        }
    };

    // 配置转化器
    string f(ConfigSign config);
}

#endif // AUTOCONFIG_H