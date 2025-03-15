#ifndef ACM_AUTOTEST_H
#define ACM_AUTOTEST_H

#include <fstream>
#include <memory>
#include <filesystem>
#include "openai.hpp"
#include "json.hpp"
#include "loglib.hpp"
#include "Process.h"
#include "KeyCircle.h"
#include "AutoConfig.h"

namespace acm{
    using nlohmann::json;

    // 自动测试类
    class AutoTest{
        // 配置文件
        fs::path _path="./Config";
        AutoConfig _setting;
        // 配置文件初始化
        void init_config();
        //密钥环
        KeyCircle _openaiKey;
        // 注册key
        void init_key();
        // 测试路径配置文件
        AutoConfig _config;
        // 测试程序名称
        string _name;
        // 数据路径
        fs::path _basePath=".";
        // 题目文件
        fs::path _problemfile;
        string _problem;
        // AC代码文件
        fs::path _ACfile;
        string _ACCode;
        // 测试代码文件
        fs::path _testfile;
        string _testCode;
        // 日志文件
        log::Log _log;
        log::Log _testlog;
        // 写入文件
        void write_file(const fs::path &path,const string &code);
        // 读取文件
        string read_file(const fs::path &path);
        // prompt
        string _GeneratePrompt="";
        string _ValidatePrompt="";
        string _CheckPrompt="";
        // 加载Prompt
        void load_prompt(const fs::path &path="./config/prompt");
        // AI对象 - 智能指针
        std::unique_ptr<openai::OpenAI> _AI;
        // docs tools
        json _tools;
        // 历史记录
        json _history;
        // 对话
        string chat(const string &prompt);
        // 获取题目名称
        string get_problem_name(string name);
        // 完整性验证
        bool full_check();
        // 设定测试文件夹路径
        void set_basePath();
    public:
        // 构造函数
        AutoTest(const string &name="");
        // 设置配置文件
        void config(ConfigSign config,ConfigSign value);
        // 设置测试文件名字
        void set_name(const string &name);
        // 设置测试路径
        void set_basePath(const fs::path &path);
        // 设置题目
        void set_problem(const string &problem);
        void set_problem(const fs::path &path);
        // 设置测试代码
        void set_testCode(const string &code);
        void set_testCode(const fs::path &path);
        // 设置AC代码
        void set_ACCode(const string &code);
        void set_ACCode(const fs::path &path);
        // 载入已经存在的文件夹
        void load(const fs::path &path);
        // 初始化结构
        bool init();
        // 生成数据
        AutoTest &generate();
        // 测试数据
        AutoTest &check();
        // 开始自动对拍
        AutoTest &start();

    };
};

#endif // ACM_AUTOTEST_H