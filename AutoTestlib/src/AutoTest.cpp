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
        AutoTest::AutoTest(const string &name)
            : _name(name),_log("AutoTest.log"){
        }
            // 设置题目
        void AutoTest::set_problem(const string &problem){
            if(problem.empty()){
                throw std::runtime_error("题目不能为空");
            }
            if(_name.empty()){
                _name=get_problem_name(problem);

            }
            _problem=problem;
        }
        // 从路径获取题目
        void AutoTest::set_problem(const fs::path &path) {
            _problem=read_file(path);
            set_problem(_problem);
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
            if(_name.empty()){
                _name=path.stem().string();
                _basePath=path.parent_path()/_name;
            }
            _testCode=read_file(path);
        }
        // 设置AC代码
        void AutoTest::set_ACCode(const string &code) {
            _ACCode=code;
        }
        // 从路径获取AC代码
        void AutoTest::set_ACCode(const fs::path &path) {
            _ACCode=read_file(path);
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