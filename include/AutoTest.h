#ifndef ACM_AUTOTEST_H
#define ACM_AUTOTEST_H

#include <fstream>
#include <memory>
#include <filesystem>
#include <unordered_map>
#include "openai.hpp"
#include "json.hpp"
#include "loglib.hpp"
#include "Process.h"
#include "KeyCircle.h"
#include "AutoConfig.h"

namespace acm{
    using nlohmann::json;
    typedef struct Config{

    }Config;
    // 自动测试类
    class AutoTest{
        // 配置文件
        fs::path _path="./config";
        AutoConfig _setting;
        // 配置文件初始化
        void init_config();
        //密钥环
        KeyCircle _openaiKey;
        // 注册key
        void init_key();
        // 测试路径配置文件
        AutoConfig _config;
        // 运行时配置文件
        std::unordered_map<string,int> _temp_num;
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
        std::unordered_map<string,string> _prompt;
        // 加载Prompt
        void init_prompt(const fs::path &path="./config/prompt");
        // AI对象
        openai::OpenAI _AI;
        // docs tools
        json _tools;
        // 初始化工具
        bool init_tools(const fs::path &path="./config/tools");
        // 历史记录
        AutoConfig _history;
        // 初始化系统提示词,以及初始化历史记录
        void init_system();
        // 对话
        json chat(const json &prompt,ConfigSign type=Model);
        // 获取题目名称
        string get_problem_name();
        // 处理function call
        json handle_function(const json &func_calls);
        // 处理function call 参数名指定错误
        string check_func_call(const json &funcArgs,string &funcName);
        // 完全Ai，可以自动处理工具调用，可传入可选参数
        int AI(const string &prompt,json &session,ConfigSign useModel=Model);
        // 文档内容
        json _docs;
        // 初始化文档读取
        void init_docs(const fs::path &path="./config/docs");
        // 获取文档
        string get_docs(const string &DocsName,const string &DocsType);
        // 完整性验证
        bool full_check();
        // 错误样例集合
        AutoConfig _WAdatas;
        // 添加当前样例到错误集合
        void add_WAdatas();
        // cph路径
        fs::path _cph=".";
        // 设置cph路径
        bool set_cph(const fs::path &path);
        // 修改源文件目录下.cph配置，将错误样例自动加入
        void add_to_cph();
        // 数据存储文件夹
        std::vector<fs::path> _dataDirs;
        // 数据文件夹访问
        enum DataFloder{ inData,outData,acData };
        // 初始化其余配置
        bool init_temp();
    public:
        // 构造函数
        AutoTest(const string &name="");
        // 新增AI工具
        void add_tool(const json &tool);
        // 设置配置文件
        void config(ConfigSign key,ConfigSign value,ConfigSign target=Test);
        void config(const string key,const string value,ConfigSign target=Test);
        void config(ConfigSign key,const string value,ConfigSign target=Test);
        void config(const string key,ConfigSign value,ConfigSign target=Test);
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
        // 测试工具生成编译
        bool make(ConfigSign name,json &session);
        // 生成测试工具
        AutoTest &ai_gen();
        // 退出状态
        struct Exit{
            process::Status status;
            int exit_code;
        };
        // 进行测试
        Exit run(ConfigSign name,process::Args args);
        // 生成数据
        bool generate_data();
        // 测试数据
        bool test_data();
        // 开始自动对拍
        bool start();
        // 析构函数
        ~AutoTest();
    };
};

#endif // ACM_AUTOTEST_H