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
    // 获取文档
    string AutoTest::get_docs(const string &DocsName){
        if(_docs.find(DocsName)==_docs.end()){
            throw std::runtime_error("文档不存在: "+DocsName);
        }
        return _docs[DocsName];
    }
    // 新增工具
    void AutoTest::add_tool(const json &tool){
        _tools.push_back(tool);
    }
    // 对话
    json AutoTest::chat(const json &prompt,ConfigSign type){
        if(type==Model){
            // 发送请求
            return _AI.chat.create({
                { "model",_setting[f(type)] },
                { "messages",prompt },
                { "tool",_tools },
                { "response_format",{ "type","json_object" } },
                _setting[f(Model_Config)]
                });
        }
        else{
            return _AI.chat.create({
                { "model",_setting[f(type)] },
                { "messages",prompt },
                { "response_format",{ "type","json_object" } },
                _setting[f(Model_Config)]
                });
        }

    }
    // 处理function call, 传入message
    json AutoTest::handle_function(const json &func_calls){
        // 处理function call
        json result=json::array();
        for(auto &func_call:func_calls){
            json temp;
            string funcName=func_call["function"]["name"];
            json funcArgs=func_call["function"]["arguments"];
            if(funcName=="get_docs"){
                string docsName=funcArgs["DocsName"];
                result[funcName]=get_docs(docsName);
            }
            else{
                _log.tlog(_setting[f(Model)]+"使用了未知的函数: "+funcName,loglib::ERROR);
                result[funcName]=json::object();
                result[funcName]["error"]="未知函数调用";
            }
        }
        return result;

    }
    // 获取题目名称
    string AutoTest::get_problem_name(string name){
        // 获取题目名称
        if(name.empty()){
            _log.tlog("正在自动命名");
            json prompt={
                { "role","user" },
                { "content",_prompt["askname"].dump()+_problem }
            };
            json result=chat(prompt,Named_Model);
            json resultData=result["choices"][0]["message"]["content"];
            string tempName=resultData["name"];
            _log.tlog("自动命名成功: "+tempName);
            return tempName;
        }
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
            _log.tlog("密钥文件不存在，正在初始化密钥文件",loglib::WARNING);
            set_key();
        }
    }
    // 配置文件初始化
    void AutoTest::init_config(){
        if(!_setting.exist()){
            _log.tlog("配置文件不存在，正在初始化配置文件",loglib::WARNING);
            // 默认跟随测试文件路径
            _setting[f(Allow_Path)]=Test_Path;
            // 默认OpenAI API地址
            _setting[f(OpenAI_URL)]="https://api.openai.com/v1";
            // 初始化默认文件夹计数
            _setting[f(Floder_Number)]=0;
            // 模型默认设置
            _setting[f(Model_Config)]={
                { "temperature",0.7 },
                { "max_tokens",4096 },
                { "top_p",1 }
            };
            // 默认工具
            _setting[f(Tools)]=json::array();
            json tempTool=R"({
                "type": "function",
                "function": {
                    "name": "get_docs",
                    "description": "获得Testlib函数库的参考文档的原始信息,用来作为重要的参考依据,有一个总的概括,和四个文档的详细描述: 包括Testlib_Total的描述,Generators数据生成器文档,Validators数据验证器文档,Checkers数据检查器文档,Interactors数据交互器文档",
                    "parameters": {
                        "type": "object",
                        "properties": {
                            "DocsName": {
                                "type": "string",
                                "description": "文档的名称，有五个参数候选项可以选择: "Total",""Generators","Validators","Checkers","Interactors"。
                            }
                        },
                        "required": ["DocsName"]
                    },
                }
            })";
            _setting[f(Tools)].push_back(tempTool);

            _setting.save();
        }
    }
    // 测试配置初始化
    void AutoTest::init_test_config(){
        _config.set_path(_basePath/"config.json");
        if(!_config.exist()){
            _log.tlog("测试配置文件不存在，正在初始化配置文件",loglib::WARNING);
            _config[f(Test_Name)]=_name;
            // 默认跟随全局配置
            _config[f(Attach_Global)]=true;
            _config.save();
        }
    }
    // 初始化文档读取
    void AutoTest::init_docs(const fs::path &path){
        if(!fs::exists(path)){
            throw std::runtime_error("文档文件夹不存在: "+path.string());
        }
        // 初始化文档文件夹
        _log.log("正在读取Testlib文档");
        // 读取文档
        _docs["Total"]=rfile(path/"Testlib_Total.md");
        _docs[f(Generators)]=rfile(path/"Testlib_Generators.md");
        _docs[f(Validators)]=rfile(path/"Testlib_Validators.md");
        _docs[f(Checkers)]=rfile(path/"Testlib_Checkers.md");
        _docs[f(Interactors)]=rfile(path/"Testlib_Interactors.md");
    }
    // 加载Prompt
    void AutoTest::init_prompt(const fs::path &path){
        if(!fs::exists(path)){
            throw std::runtime_error("Prompt文件夹不存在: "+path.string());
        }
        // 初始化Prompt文件夹
        _log.tlog("正在读取Prompt");
        // 读取Prompt
        _prompt[f(Generators)]=rfile(path/"GeneratePrompt.md");
        _prompt[f(Validators)]=rfile(path/"ValidatePrompt.md");
        _prompt[f(Checkers)]=rfile(path/"CheckPrompt.md");
        _prompt["askname"]="你是一个自动命名器,请根据题面生成一个合适的题目名称。请你的回复JSON格式为:{\"name\":\"题目名称\"}。题面：";
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
    void AutoTest::config(ConfigSign config,ConfigSign value,ConfigSign target){
        if(target==Global){
            _setting[f(config)]=value;
            _log.tlog("配置项: "+f(config)+" 设置为: "+f(value));
        }
        else{
            _config[f(config)]=value;
            _testlog.tlog("配置项: "+f(config)+" 设置为: "+f(value));
        }
    }
    // 设置测试文件名字
    bool AutoTest::set_name(const string &name){
        if(name.empty()){
            _log.tlog("测试文件夹名称为空",loglib::WARNING);
            return false;
        }
        _name=name;
    }
    // 设置题目
    bool AutoTest::set_problem(const string &problem){
        if(problem.empty()){
            _log.tlog("题目不能为空",loglib::ERROR);
            return false;
        }
        _problem=problem;
        return true;
    }
    // 从路径获取题目
    bool AutoTest::set_problem(const fs::path &path){
        if(!fs::exists(path)){
            _log.tlog("题目文件不存在: "+path.string(),loglib::ERROR);
            return false;
        }
        _problemfile=path;
        _problem=rfile(path);
        if(_problem.empty()){
            _log.tlog("题目文件为空: "+path.string(),loglib::ERROR);
            return false;
        }
        return true;
    }
    // 设置测试代码
    bool AutoTest::set_testCode(const string &code){
        if(code.empty()){
            _log.tlog("测试代码不能为空",loglib::ERROR);
            return false;
        }
        _testCode=code;
        return true;
    }
    // 从路径获取测试代码
    bool AutoTest::set_testCode(const fs::path &path){
        if(!fs::exists(path)){
            _log.tlog("测试代码文件不存在: "+path.string(),loglib::ERROR);
            return false;
        }
        if(path.extension()!=".cpp"){
            _log.tlog("测试代码文件格式错误: "+path.string(),loglib::ERROR);
            return false;
        }
        _testfile=path;
        _testCode=rfile(path);
        if(_testCode.empty()){
            _log.tlog("测试代码文件为空: "+path.string(),loglib::ERROR);
            return false;
        }
        return true;
    }
    // 设置AC代码
    bool AutoTest::set_ACCode(const string &code){
        if(code.empty()){
            _log.tlog("AC代码不能为空",loglib::ERROR);
            return false;
        }
        _ACCode=code;
        return true;
    }
    // 从路径获取AC代码
    bool AutoTest::set_ACCode(const fs::path &path){
        if(!fs::exists(path)){
            _log.tlog("AC代码文件不存在: "+path.string(),loglib::ERROR);
            return false;
        }
        if(path.extension()!=".cpp"){
            _log.tlog("AC代码文件格式错误: "+path.string(),loglib::ERROR);
            return false;
        }
        _ACfile=path;
        _ACCode=rfile(path);
        if(_ACCode.empty()){
            _log.tlog("AC代码文件为空: "+path.string(),loglib::ERROR);
            return false;
        }
        return true;
    }
    // 完整性验证
    bool AutoTest::full_check(){
        // 验证配置完整性
        string temp="完整性验证失败: ";
        if(_problem.empty()){
            _log.tlog(temp+"题目为空",loglib::ERROR);
            return false;
        }
        if(_testCode.empty()){
            _log.tlog(temp+"测试代码为空",loglib::ERROR);
            return false;
        }
        if(_ACCode.empty()){
            _log.tlog(temp+"AC代码为空",loglib::ERROR);
            return false;
        }
        if(_name.empty()){
            _log.tlog("测试文件夹名称为空,将自动命名",loglib::WARNING);
        }
        if(_setting[f(OpenAI_URL)]=="https://api.openai.com/v1"){
            _log.tlog("OpenAI API为指定，将使用默认地址",loglib::WARNING);
        }
        if(_setting[f(Model)].empty()){
            _log.tlog(temp+"基本模型未指定",loglib::ERROR);
            return false;
        }
        return true;
    }
    // 设定测试文件夹路径
    bool AutoTest::set_basePath(const fs::path &path){
        if(path.empty()){
            _log.tlog("测试文件夹路径为空",loglib::ERROR);
            return false;
        }
        // 如果路径为默认路径
        if(path=="."){
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
                _log.tlog("未指定路径进行附加,将自动构造路径",loglib::WARNING);
                int floderNum=_setting[f(Floder_Number)];
                _setting[f(Floder_Number)]=++floderNum;
                _basePath=_path/string("AutoTest"+std::to_string(floderNum));
            }
        }
        else{
            // 设定测试文件夹路径
            _basePath=path;
        }
        // 检查路径上的所有目录是否存在
        if(!fs::exists(_basePath)){
            fs::create_directories(_basePath);
        }
        return true;
    }
    // 初始化配置文件夹
    bool AutoTest::init(){
        // 完整性验证
        if(!full_check()){
            _log.log("完整性验证失败",loglib::ERROR);
            return false;
        }
        _log.tlog("完整性验证成功");
        // 为测试文件夹命名
        if(_name.empty()){
            _log.tlog("未设定名称,开始为测试文件夹命名",loglib::WARNING);
            _name=get_problem_name(_problem);
        }
        // 设定测试文件夹路径
        if(_basePath!="."){
            set_basePath();
        }
        // 初始化日志
        _testlog.set_logPath(_basePath);
        _testlog.set_logName(_name+".log");
        _testlog.tlog("测试日志开始运行");
        // 重设需要的文件路径
        _problemfile=_basePath/"problem.md";
        _testfile=_basePath/"test.cpp";
        _ACfile=_basePath/"AC.cpp";
        // 写入文件
        wfile(_problemfile,_problem);
        wfile(_testfile,_testCode);
        wfile(_ACfile,_ACCode);
        _testlog.tlog("文件写入成功");
        // 初始化测试配置
        init_test_config();
        // 初始化AI - 构造
        _AI.setToken(_openaiKey.get());
        _AI.setBaseUrl(_setting[f(OpenAI_URL)]);
        _log.tlog("初始化成功,文件夹在: "+_basePath.string());
        return true;
    }
    // 载入已经存在的文件夹
    bool AutoTest::load(const fs::path &path){
        if(!fs::exists(path)){
            _log.tlog("指定路径不存在,请检查路径",loglib::ERROR);
            return false;
        }
        _basePath=path;
        _problemfile=_basePath/"problem.md";
        _testfile=_basePath/"test.cpp";
        _ACfile=_basePath/"AC.cpp";
        _config.set_path(_basePath/"config.json");
        // 读取配置文件
        if(!_config.exist()){
            _log.tlog("配置文件不存在,正在重建",loglib::WARNING);
            _name=path.filename().string();
        }
        // 读取配置项
        _name=_config[f(Test_Name)];
        _config.save();
        // 读取题目
        _problem=rfile(_problemfile);
        if(_problem.empty()){
            _log.tlog("题目文件为空,请检查",loglib::ERROR);
            return false;
        }
        // 读取测试代码
        _testCode=rfile(_testfile);
        if(_testCode.empty()){
            _log.tlog("测试代码文件为空,请检查",loglib::ERROR);
            return false;
        }
        // 读取AC代码
        _ACCode=rfile(_ACfile);
        if(_ACCode.empty()){
            _log.tlog("AC代码文件为空,请检查",loglib::ERROR);
            return false;
        }
        // 读入日志文件
        _testlog.set_logPath(_basePath);
        _testlog.set_logName(_name+".log");
        // 初始化AI - 构造
        string tempURL=_setting[f(OpenAI_URL)];
        if(!_config[f(Attach_Global)]){
            _log.tlog("附加模式为false");
            _openaiKey.set_path(_basePath/"openai.key");
            init_key();
            tempURL=_config[f(OpenAI_URL)];
            if(tempURL.empty()){
                _log.tlog("OpenAI API地址为空,使用默认地址",loglib::WARNING);
                tempURL="https://api.openai.com/v1";
                _config[f(OpenAI_URL)]=tempURL;
                _config.save();
            }
        }
        _AI.setBaseUrl(tempURL);
        _AI.setToken(_openaiKey.get());
        _log.tlog("载入"+_name+"成功");
        _testlog.tlog("重新载入成功");
        return true;
    }
    // 生成数据
    AutoTest &AutoTest::generate(){
        // 生成数据
        json prompt={
            { "role","user" },
            { "content",_prompt[f(Generators)].dump()+_problem }
        };
        json result=chat(prompt);
        json resultData=result["choices"][0]["message"]["content"];
        _log.tlog("数据生成成功");
        // 读取数据
        _testCode=resultData["code"];
        _ACCode=resultData["AC"];
        // 写入文件
        wfile(_testfile,_testCode);
        wfile(_ACfile,_ACCode);
        return *this;
    }
    // 开始自动对拍
    AutoTest &AutoTest::start(){
        // 检查路径上的所有目录是否存在
        if(!fs::exists(_basePath)){
            fs::create_directories(_basePath);
        }
        // 加载Prompt
        init_prompt();
        // 生成数据生成器

    }
};