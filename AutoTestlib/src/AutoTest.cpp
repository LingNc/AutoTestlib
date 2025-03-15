#include "AutoTest.h"

namespace acm{
        void AutoTest::write_file(const fs::path &path,const string &code){
            // 检查路径上的所有目录是否存在
            if (!fs::exists(path.parent_path())) {
                fs::create_directories(path.parent_path());
            }
            std::ofstream file(path);
            if (!file.is_open()) {
                throw std::runtime_error("无法打开文件: " + path.string());
            }
            file << code;
            file.close();
        }
        // 读取文件
        string AutoTest::read_file(const fs::path &path){
            std::ifstream file(path);
            if(!file.is_open()){
                throw std::runtime_error("无法打开文件: "+path.string());
            }
            string code((std::istreambuf_iterator<char>(file)),std::istreambuf_iterator<char>());
            file.close();
            return code;
        }
        // 加载Prompt
        void AutoTest::load_prompt(const fs::path &path){
            if(!fs::exists(path)){
                throw std::runtime_error("Prompt文件夹不存在: "+path.string());
            }
            _GeneratePrompt=read_file(path/"GeneratePrompt.md");
            _ValidatePrompt=read_file(path/"ValidatePrompt.md");
            _CheckPrompt=read_file(path/"CheckPrompt.md");
        }
        // 构造函数
        AutoTest::AutoTest(const string &name,const fs::path &basePath)
            : _name(name),_basePath(basePath),_log("AutoTest.log"){
        }
        // 设置题目
        void AutoTest::set_problem(const string &problem){
            if(problem.empty()){
                throw std::runtime_error("题目不能为空");
            }
            _problem=problem;
        }
        // 从路径获取题目
        void AutoTest::set_problem(const fs::path &path){
            if(!fs::exists(path)){
                throw std::runtime_error("题目文件不存在: "+path.string());
            }
            _problem=read_file(path);
            if(_problem.empty()){
                throw std::runtime_error("题目不能为空");
            }
        }
        // 设置测试代码
        void AutoTest::set_testCode(const string &code){
            if(code.empty()){
                throw std::runtime_error("测试代码不能为空");
            }
            _testCode=code;
        }
        // 从路径获取测试代码
        void AutoTest::set_testCode(const fs::path &path){
            if(!fs::exists(path)){
                throw std::runtime_error("测试代码文件不存在: "+path.string());
            }
            if(path.extension()!=".cpp"){
                throw std::runtime_error("测试代码文件格式错误: "+path.string());
            }
            _testCode=read_file(path);
            if(_testCode.empty()){
                throw std::runtime_error("测试代码不能为空");
            }
        }
        // 设置AC代码
        void AutoTest::set_ACCode(const string &code){
            if(code.empty()){
                throw std::runtime_error("AC代码不能为空");
            }
            _ACCode=code;
        }
        // 从路径获取AC代码
        void AutoTest::set_ACCode(const fs::path &path){
            if(!fs::exists(path)){
                throw std::runtime_error("AC代码文件不存在: "+path.string());
            }
            _ACCode=read_file(path);
            if(_ACCode.empty()){
                throw std::runtime_error("AC代码不能为空");
            }
        }
        // 初始化配置文件夹
        bool AutoTest::init(){

            if(!fs::exists(_basePath)){
                fs::create_directories(_basePath);
            }
            _problemfile=_basePath/"problem.txt";
            _testfile=_basePath/"test.cpp";
            _ACfile=_basePath/"AC.cpp";
            return true;
        }
        // 开始自动对拍
        AutoTest &AutoTest::start() {
            // 检查路径上的所有目录是否存在
            if (!fs::exists(_basePath)) {
                fs::create_directories(_basePath);
            }
            // 加载Prompt
            load_prompt();
            // 生成数据生成器

        }
};