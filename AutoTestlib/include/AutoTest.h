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
        // 测试配置初始化
        void init_test_config();
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
        loglib::Log _log;
        loglib::Log _testlog;
        // 写入文件
        void wfile(const fs::path &path,const string &code);
        // 读取文件
        string rfile(const fs::path &path);
        // prompt
        json _prompt;
        // 加载Prompt
        void init_prompt(const fs::path &path="./config/prompt");
        // AI对象
        openai::OpenAI _AI;
        // docs tools
        json _tools;
        // 历史记录
        AutoConfig _history;
        // 对话内容
        json _content;
        // 对话
        json chat(const json &prompt,ConfigSign type=Model);
        // 获取题目名称
        string get_problem_name(string name);
        // 文档文件夹
        fs::path _docsPath;
        // 文档内容
        json _docs;
        // 初始化文档读取
        void init_docs();
        // 获取文档
        string get_docs(const string &DocsName);
        // 完整性验证
        bool full_check();
    public:
        // 构造函数
        AutoTest(const string &name="");
        // 新增AI工具
        void add_tool(const json &tool);
        // 设置配置文件
        void config(ConfigSign config,ConfigSign value,ConfigSign target=Test);
        // 更改密钥
        void set_key(const string &key="");
        // 设置测试文件名字
        bool set_name(const string &name);
        // 设置测试路径
        bool set_basePath(const fs::path &path=".");
        // 设置题目
        bool set_problem(const string &problem);
        bool set_problem(const fs::path &path);
        // 设置测试代码
        bool set_testCode(const string &code);
        bool set_testCode(const fs::path &path);
        // 设置AC代码
        bool set_ACCode(const string &code);
        bool set_ACCode(const fs::path &path);
        // 载入已经存在的文件夹
        bool load(const fs::path &path);
        // 初始化结构
        bool init();
        // 生成数据
        AutoTest &generate();
        // 测试数据
        AutoTest &check();
        // 开始自动对拍
        AutoTest &start();
        // 析构函数
        ~AutoTest();
    };
};

#endif // ACM_AUTOTEST_H