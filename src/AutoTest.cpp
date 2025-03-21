#include "AutoTest.h"
#include "Judge.h"
#include "fstream"

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
        if(!fs::exists(path)){
            _log.tlog("文件不存在: "+path.string(),loglib::ERROR);
            return "";
        }
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
            return "文档不存在: "+DocsName;
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
                { "tools",_tools },
                { "tool_choice","auto" },
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
    // 处理function call, 传入func calls，附带日志
    json AutoTest::handle_function(const json &func_calls){
        // 处理function call
        json result=json::array();
        for(auto &func_call:func_calls){
            json temp;
            temp["role"]="tool";
            temp["tool_call_id"]=func_call["id"];
            string funcName=func_call["function"]["name"];
            json funcArgs=func_call["function"]["arguments"];
            if(funcName=="get_docs"){
                if(check_func_call(funcArgs,funcName).empty()){
                    string docsName=funcArgs["DocsName"];
                    temp["content"]=get_docs(docsName);
                    _testlog.tlog(_setting[f(Model)].get<string>()+"调用了函数: "+funcName+"参数: "+funcArgs.dump());
                }
            }
            else{
                _testlog.tlog(_setting[f(Model)].get<string>()+"使用了未知的函数: "+funcName,loglib::ERROR);
                // 获取函数列表
                string funcList;
                for(auto &tool:_tools){
                    funcList+=tool["function"]["name"].get<string>()+", ";
                }
                temp["content"]="你使用了未知函数: "+funcName+"应该使用的函数包括: "+funcList;
            }
            // 添加到对话列表
            result.push_back(temp);
        }
        return result;
    }
    // 处理function call 参数名指定错误
    // 如果正常参数返回值应该是一个空字符串，不正常会返回给Ai错误信息
    string AutoTest::check_func_call(const json &funcArgs,string &funcName){
        // 处理function call 参数名指定错误
        string content;
        // 找不到参数，参数传入错误
        if(funcArgs.find("DocsName")==funcArgs.end()){
            _testlog.tlog(
                _setting[f(Model)].get<string>()+
                "使用了未知的参数名: "+
                funcArgs.dump()
                ,loglib::ERROR);
            content=
                "你使用了函数"+
                funcName+
                "的未知参数: "+
                funcArgs.dump()+
                "应该传入参数是: DocsName,并指定参数可选的值";
        }
        return content;
    }
    // 获取题目名称
    string AutoTest::get_problem_name(string name){
        // 获取题目名称
        if(name.empty()){
            _log.tlog("正在自动命名");
            json prompt=json::array({
                { "role","user" },
                { "content",_prompt["askname"]+_problem }
                });
            json result=chat(prompt,Named_Model);
            json resultData=result["choices"][0]["message"]["content"];
            string tempName=resultData["name"];
            _testlog.tlog("自动命名成功: "+tempName);
            return tempName;
        }
        return _name;
    }
    // AI，自动处理工具调用
    int AutoTest::AI(const string &prompt,json &session,ConfigSign useModel){
        json result={
            { "role","user" },
            { "content",prompt }
        };
        session.push_back(result);
        result=chat(session,useModel);
        // 循环处理一次中的function calls
        while(result["choices"][0]["finish_reason"]=="function_call"){
            json tempData=result["choices"][0]["message"];
            json tempCalls=tempData["function_calls"];
            string tempContent=tempData["content"];
            // 处理function call，并输出日志
            json toolMessage=handle_function(tempCalls);
            // 拼接历史记录
            if(!tempContent.empty()){
                session+={
                    {"role","assistant"},
                    { "content",tempContent }
                };
            }
            session+=toolMessage;
            // 发送请求
            result=chat(session);
        }
        // 读取最后一次数据
        session.push_back({
            { "role","assistant" },
            { "content",result["choices"][0]["message"]["content"] }
            });
        return 0;
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
            _openaiKey.save(tempKey);
        }
        else
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
            json tempTool={
                { "type","function" },
                { "function",{
                    { "name","get_docs" },
                { "description","获得Testlib函数库的参考文档的原始信息,用来作为重要的参考依据,有一个总的概括,和四个文档的详细描述: 包括Testlib_Total的描述,Generators数据生成器文档,Validators数据验证器文档,Checkers数据检查器文档,Interactors数据交互器文档" },
                { "parameters",{
                    { "type","object" },
                { "properties",{
                    { "DocsName",{
                        { "type","string" },
                { "description","文档的名称，有五个参数候选项可以选择: \"Total\",\"Generators\",\"Validators\",\"Checkers\",\"Interactors\"。" }
            } }
            } },
                { "required",json::array({ "DocsName" }) }
            } }
            } }
            };
            // json tempTool1=R"({
            //     "type": "function",
            //     "function": {
            //         "name": "get_docs",
            //         "description": "获得Testlib函数库的参考文档的原始信息,用来作为重要的参考依据,有一个总的概括,和四个文档的详细描述: 包括Testlib_Total的描述,Generators数据生成器文档,Validators数据验证器文档,Checkers数据检查器文档,Interactors数据交互器文档",
            //         "parameters": {
            //             "type": "object",
            //             "properties": {
            //                 "DocsName": {
            //                     "type": "string",
            //                     "description": "文档的名称，有五个参数候选项可以选择: "Total",""Generators","Validators","Checkers","Interactors"。"
            //                 }
            //             },
            //             "required": ["DocsName"]
            //         }
            //     }
            // })";
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
            // 生成测试数量，以及提供的初始化随机数值
            _config[f(DataNum)]=0;
            _config.save();
        }
    }
    // 初始化文档读取
    void AutoTest::init_docs(const fs::path &path){
        if(!fs::exists(path)){
            throw std::runtime_error("文档文件夹不存在: "+path.string());
        }
        // 初始化文档文件夹
        _log.tlog("正在读取Testlib文档");
        // 读取文档
        auto &_docs_origin=_docs["origin"];
        _docs_origin["Total"]=rfile(path/"Testlib_Total.md");
        _docs_origin[f(Generators)]=rfile(path/"Testlib_Generators.md");
        _docs_origin[f(Validators)]=rfile(path/"Testlib_Validators.md");
        _docs_origin[f(Checkers)]=rfile(path/"Testlib_Checkers.md");
        _docs_origin[f(Interactors)]=rfile(path/"Testlib_Interactors.md");
        auto &_docs_new=_docs["new"];
        _docs_new["index"]=rfile(path/"index.md");
        _docs_new["general"]=rfile(path/"general.md");
        _docs_new[f(Generators)]=rfile(path/"generator.md");
        _docs_new[f(Validators)]=rfile(path/"validator.md");
        _docs_new[f(Checkers)]=rfile(path/"checker.md");
        _docs_new[f(Interactors)]=rfile(path/"interactor.md");
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
        _prompt["system"]=rfile(path/"System.md");
        _prompt["askname"]="你是一个自动命名器,请根据题面生成一个合适的题目名称。请你的回复JSON格式为:{\"name\":\"题目名称\"}。题面：";
    }
    // 初始化系统提示词
    void AutoTest::init_system(){
        // 初始化系统提示词
        _history.set_path(_basePath/"history.json");
        if(!_history.exist()){
            auto &history=_history.get();
            history=json::array();
            history.push_back({
                { "role","system" },
                { "content",_prompt["system"] }
                });
            _history.save();
        }

    }
    // 构造函数
    AutoTest::AutoTest(const string &name)
        :_setting(_path/"config.json"),
        _openaiKey(_path/"openai.key"),
        _name(name),
        _log(_path){
        // 设置日志总配置
        _log.set_logName("AutoTest.log");
        _log.tlog("AutoTest开始运行");
        // 配置文件初始化
        init_config();
        // 注册key
        init_key();
        // 默认文档读取
        init_docs(_path/"docs");
        // 默认prompt读取
        init_prompt(_path/"prompt");
    }
    // 设置配置文件
    void AutoTest::config(const string key,const string value,ConfigSign target){
        if(target==Global){
            _setting[key]=value;
            _log.tlog(
                "配置项: "+key+
                " 设置为: "+value);
        }
        else{
            _config[key]=value;
            _testlog.tlog(
                "配置项: "+key+
                " 设置为: "+value);
        }
    }
    void AutoTest::config(ConfigSign key,const string value,ConfigSign target){
        config(f(key),value,target);
    }
    void AutoTest::config(const string key,ConfigSign value,ConfigSign target){
        config(key,f(value),target);
    }
    void AutoTest::config(ConfigSign key,ConfigSign value,ConfigSign target){
        config(f(key),f(value),target);
    }

    // 设置测试文件名字
    bool AutoTest::set_name(const string &name){
        if(name.empty()){
            _log.tlog("测试文件夹名称为空",loglib::WARNING);
            return false;
        }
        _name=name;
        return true;
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
        // 设置cph路径
        set_cph(path.parent_path()/".cph");
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
    bool AutoTest::set_cph(const fs::path &path){
        // 检查文件夹是否存在
        if(!fs::exists(path)){
            _log.tlog("cph文件夹不存在: "+path.string(),loglib::WARNING);
            return false;
        }
        _cph=path;
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
        // 初始化历史记录
        init_system();
        // 初始化错误样例集合
        _WAdatas.set_path(_basePath/"WAdatas.json");
        if(!_WAdatas.exist()){
            _log.tlog("正在初始化错误样例集合");
            _WAdatas.get()=json::array();
            _WAdatas.save();
        }
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
        // 初始化系统提示词
        init_system();
        // 读入错误样例集合
        _WAdatas.set_path(_basePath/"WAdatas.json");
        if(!_WAdatas.exist()){
            _log.tlog("错误样例集合不存在,正在重建",loglib::WARNING);
            _WAdatas.get()=json::array();
            _WAdatas.save();
        }
        _log.tlog("载入"+_name+"成功");
        _testlog.tlog("重新载入成功");
        return true;
    }
    // 测试工具生成编译
    bool AutoTest::make(ConfigSign name,json &session){
        string nameStr;
        switch(name){
        case Generators:
            nameStr="数据生成器";
            break;
        case Validators:
            nameStr="数据验证器";
            break;
        case Checkers:
            nameStr="数据检查器";
            break;
        case Interactors:
            nameStr="数据交互器";
            break;
        default:
            _testlog.tlog("未知生成器类型",loglib::ERROR);
            return false;
        }
        string prompt;
        _testlog.tlog("正在生成"+nameStr);
        prompt=_prompt[f(name)];
        // 处理请求
        AI(prompt,session);
        _testlog.tlog(nameStr+"生成成功");
        // 读取数据
        json result;
        try{
            result=json::parse(string(session.back()["content"]));
        }
        catch(const json::exception &e){
            _testlog.tlog("JSON解析失败: "+string(e.what()),loglib::ERROR);
            return false;
        }
        string code=result["code"];
        // 写入文件
        string fileName=f(name);
        wfile(_basePath/string(fileName+".cpp"),code);
        // 编译文件
        _testlog.tlog("正在编译"+nameStr);
        process::Args args("g++");
        args.add(fileName+".cpp").add("-o").add(fileName);
        process::Process proc("/bin/g++",args);
        proc.start();
        process::Status status=proc.wait();
        return status==process::STOP;
    }
    // 生成测试工具
    AutoTest &AutoTest::gen(){
        // 初始化提示词
        json &session=_history.get();
        session.push_back({
            { "role","user" },
            { "content","完整题面为: "+_problem },
            { "role","user" },
            { "content","AC代码为: "+_ACCode }
            });
        // 保存
        _history.save();
        bool temp;
        // 数据生成器
        temp=make(Generators,session);
        if(temp){
            _history.save();
            _testlog.tlog("数据生成器生成成功");
        }
        else{
            _testlog.tlog("数据生成器生成失败",loglib::ERROR);
            return *this;
        }
        // 数据校验器
        temp=make(Validators,session);
        if(temp){
            _history.save();
            _testlog.tlog("数据校验器生成成功");
        }
        else{
            _testlog.tlog("数据校验器生成失败",loglib::ERROR);
            return *this;
        }
        // 数据检查器
        temp=make(Checkers,session);
        if(temp){
            _history.save();
            _testlog.tlog("数据检查器生成成功");
        }
        else{
            _testlog.tlog("数据检查器生成失败",loglib::ERROR);
            return *this;
        }
        return *this;
    }
    // 运行测试
    AutoTest::Exit AutoTest::run(ConfigSign name){
        // 运行测试
        string nameStr;
        fs::path runfile=_basePath;
        // 统一创建所需文件夹
        std::vector<fs::path> dataDirs={
            _basePath/"inData",
            _basePath/"outData",
            _basePath/"acData"
        };

        for(const auto &dir:dataDirs){
            if(!fs::exists(dir)){
                try{
                    fs::create_directories(dir);
                }
                catch(const fs::filesystem_error &e){
                    _testlog.tlog("创建目录失败: "+dir.string()+" - "+e.what(),loglib::ERROR);
                    Exit res;
                    res.status=process::ERROR;
                    return res;
                }
            }
        }

        process::Args args;
        process::Process proc;
        // 返回值
        Exit res;
        string dataName=_config[f(NowData)].get<string>();
        switch(name){
        case Generators:{
            nameStr="数据生成器";
            runfile/=f(name);
            // 读取计数
            int num=_config[f(DataNum)];
            num++;
            _config[f(DataNum)]=num;
            // 更新文件
            _config[f(NowData)]="data"+std::to_string(num);
            _config.save();
            // 设置路径
            args.add(f(name)).add(std::to_string(num)).add(">").add(dataDirs[0]/(dataName+".in"));
            proc.load(runfile,args);
            _testlog.tlog("正在运行"+nameStr);
            proc.start();
            // 等待运行结束
            res.status=proc.wait();;
            res.exit_code=proc.get_exit_code();
            break;
        }
        case Validators:{
            nameStr="数据验证器";
            runfile/=f(name);
            args.add(f(name)).add("<").add(dataDirs[1]/(dataName+".in"));
            proc.load(runfile,args);
            _testlog.tlog("正在运行"+nameStr);
            proc.start();
            // 等待运行结束
            res.status=proc.wait();
            res.exit_code=proc.get_exit_code();
            break;
        }
        case Checkers:{
            nameStr="数据检查器";
            runfile/=f(name);
            args.add(f(name)).add(dataDirs[0]/(dataName+".in")).add(dataDirs[1]/(dataName+".out")).add(dataDirs[2]/(dataName+".out"));
            proc.load(runfile,args);
            _testlog.tlog("正在运行"+nameStr);
            proc.start();
            // 等待运行结束
            res.status=proc.wait();
            res.exit_code=proc.get_exit_code();
            break;
        }
        case Interactors:
            nameStr="数据交互器";
            runfile/=f(name);
            args.add(f(name));
            proc.load(runfile,args);
            _testlog.tlog("正在运行"+nameStr);
            proc.start();
            // 等待运行结束
            res.status=proc.wait();
            res.exit_code=proc.get_exit_code();
            break;
        case AC_Code:
            // 运行AC代码
            nameStr="AC代码";
            runfile=_ACfile;
            args.add(_ACfile).add("<").add(dataDirs[0]/(dataName+".in")).add(">").add(dataDirs[1]/(dataName+".out"));
            proc.load(runfile,args);
            _testlog.tlog("正在运行"+nameStr);
            proc.set_timeout(_config[f(TimeLimit)]);
            proc.set_memout(_config[f(MemLimit)]);
            proc.start();
            // 等待运行结束
            res.status=proc.wait();
            res.exit_code=proc.get_exit_code();
            break;
        case Test_Code:
            // 运行测试代码
            nameStr="测试代码";
            runfile=_testfile;
            args.add(_testfile).add("<").add(dataDirs[0]/(dataName+".in")).add(">").add(dataDirs[1]/(dataName+".out"));
            proc.load(runfile,args);
            _testlog.tlog("正在运行"+nameStr);
            proc.set_timeout(_config[f(TimeLimit)]);
            proc.set_memout(_config[f(MemLimit)]);
            proc.start();
            // 等待运行结束
            res.status=proc.wait();
            res.exit_code=proc.get_exit_code();
            break;
        default:
            _log.tlog("未知运行文件: "+f(name),loglib::ERROR);
            throw std::runtime_error("未知运行文件: "+f(name));
        }
        return res;
    }
    // 开始自动对拍
    bool AutoTest::start(){
        // 检测是否已经编译和生成
        if(fs::exists(f(Generators))&&fs::exists(f(Validators))&&fs::exists(f(Checkers))){
            _log.tlog("测试文件已经编译,开始自动对拍");
        }
        else{
            _log.tlog("测试文件不存在,请先编译",loglib::ERROR);
            return false;
        }
        // 开始运行
        // 循环验证数据直到找到不一致的数据
        int error_nums=0;
        while(true){
            int num=_config[f(DataNum)];
            _testlog.tlog("正在运行第"+std::to_string(num)+"个测试点");
            // 生成数据并检查数据是否符合要求
            Exit res=run(Generators);
            if(res.status==process::STOP){
                _testlog.tlog("数据生成器运行成功");
            }
            else{
                _testlog.tlog("数据生成器运行失败",loglib::ERROR);
                return false;
            }
            // 运行数据验证器
            res=run(Validators);
            if(res.status==process::STOP){
                _testlog.tlog("数据验证成功");
            }
            else if(res.status==process::ERROR){
                _testlog.tlog("本次数据生成不符合要求，正在重新生成",loglib::WARNING);
                // 重置计数
                auto &temp=_config[f(DataNum)].get_ref<json::number_integer_t&>();
                temp-=1;
                _config.save();
                // 重新生成本次数据
                continue;
            }
            else{
                _testlog.tlog("数据验证器运行失败",loglib::ERROR);
                return false;
            }
            // 运行Test代码获得对应输出
            res=run(Test_Code);
            if(res.status==process::STOP){
                _testlog.tlog("测试代码运行成功");
                JudgeCode temp=judge(res.status,res.exit_code);
                _config[f(JudgeStatus)]=f(temp);
                _config.save();
            }
            else{
                _testlog.tlog("测试代码运行失败",loglib::ERROR);
                return false;
            }
            // 运行AC代码
            res=run(AC_Code);
            if(res.status==process::STOP){
                _testlog.tlog("AC代码运行成功");
                JudgeCode temp=judge(res.status,res.exit_code);
                if(temp!=Waiting){
                    _testlog.tlog("AC代码出现问题, 状态: "+f(temp),loglib::ERROR);
                    return false;
                }
            }
            else{
                _testlog.tlog("AC代码运行失败",loglib::ERROR);
                return false;
            }
            // 如果已经判题
            if(_config[f(JudgeStatus)]!=f(Waiting)){
                error_nums+=1;
                _testlog.tlog("第"+std::to_string(num)+"个测试点,状态: "+string(_config[f(JudgeStatus)]));
                // 把当前样例加入错误集合
                add_WAdatas();
                return true;
            }
            // 运行数据检查器
            res=run(Checkers);
            if(res.status==process::STOP){
                _config[f(JudgeStatus)]=f(Accept);
                _testlog.tlog(string(_config[f(NowData)])+": "+f(Accept));
                return true;
            }
            else if(res.status==process::ERROR){
                // 获取非零状态码
                if(WIFEXITED(res.exit_code)){
                    int actual_code=WEXITSTATUS(res.exit_code);
                    if(actual_code==1){
                        _config[f(JudgeStatus)]=f(WrongAnswer);
                    }
                    else if(actual_code==2){
                        _config[f(JudgeStatus)]=f(PresentationError);
                    }
                    else{
                        _config[f(JudgeStatus)]=f(RuntimeError);
                    }
                    _testlog.tlog("第"+std::to_string(num)+"个测试点,状态: "+string(_config[f(JudgeStatus)]));
                    // 当前样例添加到错误集合
                    add_WAdatas();
                    return true;
                }
                else
                    return false;
            }
            else{
                _testlog.tlog("数据检查器运行失败",loglib::ERROR);
                return false;
            }
        }
    }
    // 添加错误集合
    void AutoTest::add_WAdatas(){
        string dataName=_config[f(NowData)];
        string in=rfile(_basePath/"inData"/(dataName+".in"));
        string out=rfile(_basePath/"acData"/(dataName+".out"));
        // 添加到错误样例集合
        json temp={
            { "in",in },
            { "out",out }
        };
        _WAdatas.get().push_back(temp);
        _WAdatas.save();
    }
    void AutoTest::add_to_cph(){
        // 如果cph路径被赋值才会执行
        if(_cph=="."||_cph.empty()){
            return;
        }
        // 在cph文件夹中找到包含name的文件
        std::vector<fs::path> matched_files;

        try{
            // 遍历CPH文件夹中的所有文件
            for(const auto &entry:fs::directory_iterator(_cph)){
                // 检查是否是文件且文件名包含指定的字符串
                if(fs::is_regular_file(entry)&&entry.path().filename().string().find(_name)!=string::npos){
                    matched_files.push_back(entry.path());
                    _testlog.tlog("找到匹配的CPH文件: "+entry.path().string());
                }
            }

            if(matched_files.empty()){
                _testlog.tlog("CPH: 未找到包含 "+_name+" 的CPH文件",loglib::WARNING);
                return;
            }

            // 对找到的文件执行操作...
            // 例如，可以将 in 和 out 添加到这些文件中
            for(const auto &file:matched_files){
                _testlog.tlog("CPH: 处理文件: "+file.string());
                // 具体的处理逻辑...
            }
        }
        catch(const std::exception &e){
            _testlog.tlog("CPH: 查找文件时出错: "+string(e.what()),loglib::ERROR);
        }
        // 以json的方式打开这个文件
        fs::path first_file=matched_files[0];
        json cph_json;
        // 读取json文件
        try{
            // 读取JSON文件内容
            string file_content=rfile(first_file);
            // 解析JSON内容
            cph_json=json::parse(file_content);
            // 添加测试数据
            if(cph_json.contains("tests")){
                // 读取错误样例集合最后一个
                json temp=_WAdatas.get().back();

                // 创建新的测试用例
                json new_test={
                    { "id",cph_json["tests"].size()+1 },
                    { "input",temp["in"] },
                    { "output",temp["out"] }
                };
                // 添加到测试用例列表
                cph_json["tests"].push_back(new_test);
                // 写回文件
                wfile(first_file,cph_json.dump());

                _testlog.tlog("成功添加测试用例到CPH文件");
            }
            else{
                _testlog.tlog("CPH文件格式不正确，没有tests字段",loglib::WARNING);
            }
        }
        catch(const json::exception &e){
            _testlog.tlog("CPH: JSON解析失败: "+string(e.what()),loglib::ERROR);
        }
        catch(const std::exception &e){
            _testlog.tlog("CPH: 读取或写入文件失败: "+string(e.what()),loglib::ERROR);
        }
    }
    // 析构函数
    AutoTest::~AutoTest(){
        _log.tlog("AutoTest结束运行");
        // 保存历史记录
        _history.save();
        // 保存配置文件
        _setting.save();
        _config.save();
        // 保存错误样例集合
        _WAdatas.save();
    }
};