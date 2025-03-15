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
        // 构造函数
        AutoTest::AutoTest(const fs::path &testPath,const string &ACCode=""){
            set_TestCode(testPath);
            set_ACCode(ACCode);
        }
        // 设置测试代码
        void AutoTest::set_TestCode(const string &name,const string &code) {
            _name=name;
            _basePath/=name;
            _testPath=_basePath/"test.cpp";
            _testCode=code;
            write_file(_testPath,_testCode);
        }
        // 从路径获取测试代码
        void AutoTest::set_TestCode(const fs::path &path){
            _name=path.stem().string();
            _basePath=path.parent_path()/_name;
            _testPath=_basePath/"test.cpp";
            _testCode=read_file(path);
            write_file(_testPath,_testCode);
        }
        // 设置AC代码
        void AutoTest::set_ACCode(const string &code) {
            _ACCode=code;
            _ACPath=_basePath/"AC.cpp";
            write_file(_ACPath,_ACCode);
        }
};