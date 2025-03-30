#ifndef AUTOCONFIG_H
#define AUTOCONFIG_H

#include <fstream>
#include <any>
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
        General, //> 综述
        Generators, //> 生成器
        Validators, //> 验证器
        Checkers, //> 检查器
        Interactors, //> 交互器
        System, //> 系统
        DataNum, //> 数据计数
        NowData, //> 现在的数据位置
        NowTest, //> 现在的测试位置
        AC_Code, //> AC代码
        Test_Code, //> 测试代码
        TimeLimit, //> 时间限制
        MemLimit, //> 内存限制
        ErrorLimit, //> 在达到错误数量之后自动退出
        JudgeStatus, //> 判题状态
        Special, // > 特例
        Edge, // > 边界
    };
    // 配置类
    class AutoConfig{
        std::any _tempData;
        json _data;
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
        // 保存
        template<typename T>
        void save(T config){
            sync<T>();
            save();
        }
        // 获取JSON数据
        json &value();
        // 同步数据
        template<typename T>
        void sync(){
            _data=std::any_cast<T>(_tempData);
        }
        // 操作符
        template<typename T>
        json &operator[](T key){
            // _tempData=_data.get<T>();
            return _data[key];
        }
        // =
        template<typename T>
        void operator=(T value){
            _tempData=value;
            _data=value;
        }
        // 获取指定类型原数据的引用
        template<typename T>
        T &get(){
            return std::any_cast<T &>(_tempData);
        }
    };

    // 配置转化器
    string f(ConfigSign config);
}

#endif // AUTOCONFIG_H