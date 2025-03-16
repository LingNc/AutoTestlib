#include "AutoTest.h"

namespace acm{
    void AutoTest::wfile(const fs::path &path,const string &code){
        // 检查路径上的所有目录是否存在
        if(!fs::exists(path.parent_path())){
            fs::create_directories(path.parent_path());
        }
        std::ofstream file(path);
        if(!file.is_open()){
            throw std::runtime_error("无法打开文件: "+path.string());
        }
        file<<code;
        file.close();
    }
    // 读取文件
    string AutoTest::rfile(const fs::path &path){
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
        _GeneratePrompt=rfile(path/"GeneratePrompt.md");
        _ValidatePrompt=rfile(path/"ValidatePrompt.md");
        _CheckPrompt=rfile(path/"CheckPrompt.md");
    }
    // 更改密钥
    void AutoTest::set_key(const string &key){
        if(key.empty()){
            std::cout<<"请输入OpenAi密钥: ";
            string tempKey;
            std::cin>>tempKey;
            if(tempKey.empty()){
                throw std::runtime_error("密钥不能为空");
            }
        }
        _openaiKey.save(key);
        _log.tlog("密钥注册成功");
    }
    // 注册key
    void AutoTest::init_key(){
        if(!_openaiKey.exist()){
            _log.tlog("密钥文件不存在，正在初始化密钥文件",log::WARNING);
            set_key();
        }
    }
    // 配置文件初始化
    void AutoTest::init_config(){
        if(!_setting.exist()){
            _log.tlog("配置文件不存在，正在初始化配置文件",log::WARNING);
            // 默认跟随测试文件路径
            _setting[f(Allow_Path)]=Test_Path;
            // 默认OpenAI API地址
            _setting[f(OpenAI_URL)]="https://api.openai.com/v1";
            _setting.save();
        }
    }
    // 构造函数
    AutoTest::AutoTest(const string &name)
        : _name(name),_openaiKey(_path/"openai.key"),
        _setting(_path/"config.json"),_log(_path){
        // 设置日志总配置
        _log.set_logName("AutoTest.log");
        _log.tlog("AutoTest开始运行");
        // 配置文件初始化
        init_config();
        // 注册key
        init_key();
    }
    // 设置配置文件
    void AutoTest::config(ConfigSign config,ConfigSign value){
        _setting[f(config)]=value;
        _log.tlog("配置项: "+f(config)+" 设置为: "+f(value));
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
        _problemfile=path;
        _problem=rfile(path);
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
        _testfile=path;
        _testCode=rfile(path);
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
        if(path.extension()!=".cpp"){
            throw std::runtime_error("AC代码文件格式错误: "+path.string());
        }
        _ACfile=path;
        _ACCode=rfile(path);
        if(_ACCode.empty()){
            throw std::runtime_error("AC代码不能为空");
        }
    }
    // 完整性验证
    bool AutoTest::full_check(){
        // 验证配置完整性
        string temp="完整性验证失败: ";
        if(_problem.empty()){
            _log.tlog(temp+"题目为空",log::ERROR);
            return false;
        }
        if(_testCode.empty()){
            _log.tlog(temp+"测试代码为空",log::ERROR);
            return false;
        }
        if(_ACCode.empty()){
            _log.tlog(temp+"AC代码为空",log::ERROR);
            return false;
        }
        if(_name.empty()){
            _log.tlog("测试文件夹名称为空,将自动命名",log::WARNING);
            return false;
        }
        if(_setting[f(OpenAI_URL)]=="https://api.openai.com/v1"){
            _log.tlog("OpenAI API为指定，将使用默认地址",log::WARNING);
            return false;
        }
        return true;
    }
    // 设定测试文件夹路径
    void AutoTest::set_basePath(){
        // 设定测试文件夹路径
        ConfigSign AllowConfig=_setting[f(Allow_Path)];
        switch(AllowConfig){
        case AC_Path:
            if(!_ACfile.empty()){
                _basePath=_ACfile.parent_path()/_name;
            }
            break;
        case Test_Path:
            if(!_testfile.empty()){
                _basePath=_testfile.parent_path()/_name;
            }
            break;
        case Problem_Path:
            if(!_problemfile.empty()){
                _basePath=_problemfile.parent_path()/_name;
            }
            break;
        default:
            throw std::runtime_error("未知配置项");
        }
        // 未指定一个路径进行附加
        if(_basePath=="."){
            _log.tlog("未指定路径进行附加,使用默认路径",log::WARNING);
            // _basePath=fs::current_path()/fs::path(_name);
        }
        // 检查路径上的所有目录是否存在
        if(!fs::exists(_basePath)){
            fs::create_directories(_basePath);
        }
    }
    // 初始化配置文件夹
    bool AutoTest::init(){
        // 完整性验证
        if(!full_check()){
            _log.log("完整性验证失败",log::ERROR);
            return false;
        }
        _log.tlog("完整性验证成功");
        // 为测试文件夹命名
        if(_name.empty()){
            _log.tlog("未设定名称,开始为测试文件夹命名",log::WARNING);
            _name=get_problem_name(_problem);
        }
        // 设定测试文件夹路径
        set_basePath();
        // 重设需要的文件路径
        _problemfile=_basePath/"problem.md";
        _testfile=_basePath/"test.cpp";
        _ACfile=_basePath/"AC.cpp";
        // 写入文件
        wfile(_problemfile,_problem);
        wfile(_testfile,_testCode);
        wfile(_ACfile,_ACCode);
        // 写入配置文件
        _config.set_path(_basePath/"config.json");
        _config[f(Test_Name)]=_name;
        _config.save();
        // 初始化AI
        _AI=std::make_unique<openai::OpenAI>(_openaiKey.get(),"",true,_setting[f(OpenAI_URL)]);
        return true;
    }
    // 开始自动对拍
    AutoTest &AutoTest::start(){
        // 检查路径上的所有目录是否存在
        if(!fs::exists(_basePath)){
            fs::create_directories(_basePath);
        }
        // 加载Prompt
        load_prompt();
        // 生成数据生成器

    }
};