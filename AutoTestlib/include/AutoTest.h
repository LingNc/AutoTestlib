#ifndef ACM_AUTOTEST_H
#define ACM_AUTOTEST_H

#include <fstream>
#include "openai.hpp"
#include "json.hpp"
#include "Process.h"
#include "loglib.hpp"

namespace acm{
    using std::string;
    class AutoTest{
        // 测试程序名称
        string _name;
        // 数据路径
        fs::path _basePath=".";
        // AC代码路径
        fs::path _ACPath;
        string _ACCode;
        // 测试代码路径
        fs::path _testPath;
        string _testCode;
        // 日志文件
        log::Log _log;
        // 写入文件
        void write_file(const fs::path &path,const string &code);
        // 读取文件
        string read_file(const fs::path &path);

    public:
        // 构造函数
        AutoTest(){}
        AutoTest(const fs::path &testPath,const string &ACCode="");
        // 设置测试代码
        void set_TestCode(const string &name,const string &code);
        // 从路径获取测试代码
        void set_TestCode(const fs::path &path);
        // 设置AC代码
        void set_ACCode(const string &code);
        // 开始自动对拍
        AutoTest &start();

    };
};

#endif // ACM_AUTOTEST_H