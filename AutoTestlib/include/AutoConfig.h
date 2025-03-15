#ifndef AUTOCONFIG_H
#define AUTOCONFIG_H

#include "Self.h"
// 标签转换
#define str(label) #label

namespace acm{
    // 配置文件标识
    enum ConfigSign{
        Allow_Path, //> 跟随路径
        AC_Path, //> AC代码路径
        Test_Path, //> 测试代码路径
        Problem_Path, //> 题目路径
    };
    // 配置转化器
    string f(ConfigSign config){
        switch(config){
        case Allow_Path:
            return str(Allow_Path);
        case AC_Path:
            return str(AC_Path);
        case Test_Path:
            return str(Test_Path);
        case Problem_Path:
            return str(Problem_Path);
        default:
            throw std::runtime_error("未知配置项");
        }
    }
}

#endif // AUTOCONFIG_H