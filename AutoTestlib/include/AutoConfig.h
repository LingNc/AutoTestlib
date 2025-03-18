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
        Global, //> 全局配置文件
        Test, //> 测试配置文件
        Allow_Path, //> 跟随路径
        AC_Path, //> AC代码路径
        Test_Path, //> 测试代码路径
        Problem_Path, //> 题目路径
        OpenAI_URL, //> OpenAI API地址
        Test_Name, //> 测试文件名称
        Floder_Number, //> 自动创建文件夹
        Attach_Global, //> 附加Prompt模式 true 附加 false 分离
        Model, //> 默认模型
        Named_Model, //> 命名模型
        Model_Config, //> 模型配置
        Prompt, //> Prompt
        Temperature, //> 温度
        Max_Token, //> 最大Token
        Top_P, //> Top P
        Tools, //> 工具
        Generators, //> 生成器
        Validators, //> 验证器
        Checkers, //> 检查器
        Interactors, //> 交互器
        System, //> 系统
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
        void save(size_t dumpNum=4);
        // 获取原数据
        json &get();
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