#include <random>
#include "AutoTest.h"
#include "AutoJson.h"
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
    string AutoTest::get_docs(const string &DocsName,const string &DocsType){
        if(_docs.find(DocsType)==_docs.end()){
            return "文档类型不存在: "+DocsType;
        }
        if(_docs[DocsType].find(DocsName)==_docs[DocsType].end()){
            return "文档不存在: "+DocsName;
        }
        if(DocsType.empty()){
            return _docs["new"][DocsName];
        }
        return _docs[DocsType][DocsName];
    }
    // 新增工具
    void AutoTest::add_tool(const ns::Request::Tool &tool){
        _tools.push_back(tool);
    }
    // 对话
    json AutoTest::chat(const json &prompt,ConfigSign type){
        // 选择日志记录者
        auto templog=(type==Named_Model)?_log:_testlog;
        json temp={};
        if(type==Model){
            temp["tools"]=_tools;
            temp["tool_choice"]=_config["tool_choice"];
        }
        string modelName;
        // 命名模型不存在则使用原始模型
        if(_setting[f(type)].empty()){
            if(type==Named_Model){
                modelName=_setting[f(Model)].get<string>();
            }
            else{
                templog.tlog("模型不存在",loglib::ERROR);
            }
        }
        else{
            modelName=_setting[f(type)].get<string>();
        }
        // 更新配置
        temp.update({
            { "model",modelName },
            { "messages",prompt },
            { "response_format",{ { "type","json_object" } } }
            });
        // 添加默认配置
        temp.update(_setting[f(Model_Config)]);
        // 格式化
        json requireData=temp;
        // std::cout<<requireData.dump(4)<<std::endl;
        // 验证结果是否正确
        json result;
        try{
            result=_AI.chat.create(requireData);
        }
        catch(std::exception &e){
            templog.tlog("OpenAI请求出现异常: "+string(e.what()),loglib::ERROR);
            return result;
        }
        if(result.type()!=json::value_t::object){
            templog.tlog("出现错误,返回结果: "+result.dump(),loglib::ERROR);
        }
        // 并不是正确的返回
        if(result.find("id")==result.end()){
            if(result.is_null()){
                templog.tlog("请求失败,返回数据为空",loglib::ERROR);
                return result;
            }
            int errorCode=0;
            if(result.contains("code")){
                errorCode=result["code"];
            }
            string message=result["message"];
            string theData=result["data"].dump();
            templog.tlog(
                "请求失败,错误代码: "+std::to_string(errorCode)+
                ",错误信息: "+message+
                ",返回数据: "+theData
                ,loglib::ERROR);
            // 检查是否有核心项缺失
            // Use the Response structure to check for missing fields
            ns::Request temp=result;

            // Log specific missing fields in order of importance
            if(result.find("error")!=result.end()){
                string errorType=result["error"]["type"];
                string errorMsg=result["error"]["message"];
                templog.tlog("API错误: "+errorType+" - "+errorMsg,loglib::ERROR);
            }
            else if(temp.model.empty()){
                templog.tlog("模型名称缺失",loglib::ERROR);
            }
            else if(temp.messages.empty()){
                templog.tlog("消息内容缺失",loglib::ERROR);
            }
            else if(!temp.response_format.has_value()&&type==Model){
                templog.tlog("返回格式缺失",loglib::ERROR);
            }
            else if(!temp.tools.has_value()&&type==Model){
                templog.tlog("工具列表缺失",loglib::ERROR);
            }
            else if(!temp.tool_choice.has_value()&&type==Model){
                templog.tlog("工具选择缺失",loglib::ERROR);
            }
            else{
                // Log the complete request data for debugging
                templog.tlog("完整请求数据："+requireData.dump(),loglib::DEBUG);
            }
        }
        return result;
    }

    // 流式对话
    void AutoTest::chat_stream(const json &prompt,std::function<void(const json &)> messageCallback,ConfigSign type){
        // 选择日志记录者
        auto templog=(type==Named_Model)?_log:_testlog;
        json temp={};
        if(type==Model){
            temp.update({
                { "tools",_tools },
                { "tool_choice","auto" },
                });
        }

        string modelName;
        // 命名模型不存在则使用原始模型
        if(_setting[f(type)].empty()){
            if(type==Named_Model){
                modelName=_setting[f(Model)].get<string>();
            }
            else{
                templog.tlog("模型不存在",loglib::ERROR);
                return;
            }
        }
        else{
            modelName=_setting[f(type)].get<string>();
        }

        temp.update({
            { "model",modelName },
            { "messages",prompt },
            { "stream",true }  // 启用流式传输
            });

        // 更新配置，但不包含response_format（流式传输不支持json格式）
        json configCopy=_setting[f(Model_Config)];
        if(configCopy.contains("response_format")){
            configCopy.erase("response_format");
        }
        temp.update(configCopy);

        // 使用流式API
        _AI.chat.stream(temp,[&templog,messageCallback](const std::string &chunk){
            try{
                if(chunk=="[DONE]"){
                    return false; // 结束流
                }
                // 除去前面的 "data:" 字符串
                if(chunk.substr(0,5)!="data:"){
                    templog.tlog("流式数据格式错误: "+chunk,loglib::ERROR);
                    return false; // 停止流
                }
                std::string jsonString=chunk.substr(5);
                // 使用AutoOpen解析返回的JSON数据
                ns::Response response=json::parse(jsonString);

                // 处理每个增量消息
                if(!response.choices.empty()&&!response.choices[0].message.content.empty()&&messageCallback){
                    // 直接访问message.content而不是使用response["choices"][0]["delta"]
                    json delta={
                        { "content",response.choices[0].message.content }
                    };
                    messageCallback(delta);
                }

                return true; // 继续接收数据
            }
            catch(const json::exception &e){
                templog.tlog("解析流式响应失败: "+std::string(e.what()),loglib::ERROR);
                return false; // 遇到解析错误，停止流
            }
            });
    }

    // 处理function call, 传入func calls，附带日志
    json AutoTest::handle_function(const json &func_calls){
        // 处理function call
        std::vector<ToolMessage> result;
        // json result=json::array();
        for(auto &func_call:func_calls){
            ToolMessage temp;
            temp.tool_call_id=func_call["id"];
            string funcName=func_call["function"]["name"];
            json funcArgs=json::parse(func_call["function"]["arguments"].get<string>());
            if(funcName=="get_docs"){
                if(check_func_call(funcArgs,funcName).empty()){
                    string docsName=funcArgs["DocsName"];
                    string docsType=funcArgs["DocsType"];
                    temp.content=get_docs(docsName,docsType);
                    _testlog.tlog(_setting[f(Model)].get<string>()+"调用了函数: "+funcName+"("+docsName+","+docsType+")");
                }
            }
            else{
                _testlog.tlog(_setting[f(Model)].get<string>()+": 使用了未知的函数: "+funcName,loglib::ERROR);
                // 获取函数列表
                string funcList;
                for(auto &tool:_tools){
                    funcList+=tool.function.name+", ";
                }
                temp.content="你使用了未知函数: "+funcName+"应该使用的函数包括: "+funcList;
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
    string AutoTest::get_problem_name(){
        // 获取题目名称
        if(_name.empty()){
            _log.tlog("正在自动命名");
            Session prompt;
            // json prompt=json::array();
            UserMessage temp;
            temp.content=_prompt["askname"]+_problem;
            // prompt.push_back({
            //     { "role","user" },
            //     { "content",_prompt["askname"]+_problem }
            //     });
            prompt.push_back(temp);
            json result;
            while(result.empty()){
                result=chat(prompt,Named_Model);
                if(result.empty()){
                    _log.tlog("获取名字为空，正在尝试5秒后重新获取",loglib::WARNING);
                    sleep(5);
                }
            }
            // std::cout<<result.dump(4)<<std::endl;
            // 使用AutoOpen库解析返回值
            ns::Response response=result;
            string resultData="";

            if(!response.choices.empty()){
                resultData=response.choices[0].message.content;
            }

            json resultJson=json::parse(resultData);
            string tempName=resultJson["name"];
            _log.tlog("自动命名成功: "+tempName);
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

        // 使用AutoOpen库解析返回值
        json response=result;

        // 循环处理一次中的function calls
        while(!response["choices"].empty()&&response["choices"][0]["finish_reason"]=="tool_calls"){
            auto &message=response["choices"][0]["message"];
            // json tempData = {
            //     {"content", choice.message.content},
            //     {"function_calls", choice.message.tool_calls}
            // };
            // json tempCalls = tempData["function_calls"];
            // string tempContent = tempData["content"];

            // 处理function call，并输出日志
            json toolMessage=handle_function(message["tool_calls"]);

            // 拼接历史记录
            session+=message;
            session+=toolMessage[0];
            // 除去强制使用工具
            _config["tool_choice"]="auto";
            // 发送请求
            result=chat(session);
            response=result;
        }

        // 读取最后一次数据
        if(!response["choices"][0]["message"]["content"].empty()){
            session.push_back({
                { "role","assistant" },
                { "content",response["choices"][0]["message"]["content"] }
                });
        }

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
                { "temperature",0.0 },
                { "max_tokens",4096 },
                { "top_p",1 }
            };
            _setting.save();
        }
    }
    // 测试配置初始化
    void AutoTest::init_test_config(){
        _config.set_path(_baseConfigPath/"config.json");
        if(!_config.exist()){
            _log.tlog("测试配置文件不存在，正在初始化配置文件",loglib::WARNING);
            ns::TestConfig tempConfig;
            // 默认跟随路径
            tempConfig.name=_name;
            // _config[f(Test_Name)]=_name;
            // 默认跟随全局配置
            tempConfig.attach_global=true;
            // _config[f(Attach_Global)]=true;
            // 生成测试数量，以及提供的初始化随机数值
            tempConfig.now_data=0;
            // _config[f(NowData)]=0;
            // 现在测试到的位置
            tempConfig.now_test=0;
            // _config[f(NowTest)]=0;
            // 初始化特例数量
            tempConfig.special=10;
            // _config[f(Special)]=2;
            // 初始化边界数量
            tempConfig.edge=5;
            // _config[f(Edge)]=2;
            // 错误限制
            tempConfig.error_limit=2;
            // _config[f(ErrorLimit)]=2;
            // 默认内存限制
            tempConfig.mem_limit=256;
            // 默认时间限制
            tempConfig.time_limit=1000;
            // 转换格式
            _config=tempConfig;
            // cph文件名称（源文件名称）
            _config["origin_name"]=_testfile.filename();
            // 找到test文件中对应的cph
            _config["cph_file"]=search_test_cph();
            // 是否启用权重形式控制测试样例的输出 0 1 2的权重
            _config["test_weight"]=false;
            // 对应权重
            _config["weights"]=json::array();
            // 0 1 2的权重
            _config["weights"][0]=10;
            _config["weights"][1]=1;
            _config["weights"][2]=2;
            // 工具调用
            _config["tool_choice"]="auto";
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
        _docs_origin[f(General)]=rfile(path/"Testlib_Total.md");
        _docs_origin[f(Generators)]=rfile(path/"Testlib_Generators.md");
        _docs_origin[f(Validators)]=rfile(path/"Testlib_Validators.md");
        _docs_origin[f(Checkers)]=rfile(path/"Testlib_Checkers.md");
        _docs_origin[f(Interactors)]=rfile(path/"Testlib_Interactors.md");
        auto &_docs_new=_docs["new"];
        _docs_new["index"]=rfile(path/"index.md");
        _docs_new[f(General)]=rfile(path/"general.md");
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
        _prompt["askname"]=rfile(path/"GetName.md");
    }
    // 初始化工具
    bool AutoTest::init_tools(const fs::path &path){
        // 默认工具
        Tool tempTool;
        // 从文件中读取
        tempTool=json::parse(rfile(path/"get_docs.json"));
        _tools.push_back(tempTool);
        return true;
    }
    // 初始化系统提示词
    void AutoTest::init_system(){
        // 初始化系统提示词
        _history.set_path(_baseConfigPath/"history.json");
        if(!_history.exist()){
            auto &history=_history.value();
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
        // 读取默认工具集
        init_tools(_path/"tools");
    }
    // 设置配置文件
    void AutoTest::config(const string key,const string value,ConfigSign target){
        if(target==Global){
            _setting[key]=value;
            _setting.save();
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
        // 保存cph路径
        _config["cph_path"]=_cph.string();
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
        //  设定初始化配置文件夹
        _baseConfigPath=_basePath/"config";
        return true;
    }
    // 初始化共有
    bool AutoTest::init_temp(){
        // 初始化dataDirs
        if(_dataDirs.empty()){
            // 初始化数据文件夹
            _dataDirs={
                _basePath/"inData",
                _basePath/"outData",
                _basePath/"acData"
            };
            if(!fs::exists(_dataDirs[0])){
                // 初始化测试数量
                _config[f(NowData)]=0;
                _config[f(NowTest)]=0;
                _config.save();
            }
            for(const auto &dir:_dataDirs){
                if(!fs::exists(dir)){
                    try{
                        fs::create_directories(dir);
                    }
                    catch(const fs::filesystem_error &e){
                        _testlog.tlog("创建目录失败: "+dir.string()+" - "+e.what(),loglib::ERROR);
                        // Exit res;
                        // res.status=process::ERROR;
                        return false;
                    }
                }
            }
        }
        // 初始化临时存储数字
        _temp_config=_config.value();
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
        // 初始化AI - 构造
        _AI.setToken(_openaiKey.get());
        // WARING 必须使用 https://..../v1/ 因为这个openai库不会自动补上这个/
        string tempURL=_setting[f(OpenAI_URL)];
        if(tempURL.back()!='/'){
            tempURL+='/';
        }
        _AI.setBaseUrl(tempURL);
        // 为测试文件夹命名
        if(_name.empty()){
            _log.tlog("未设定名称,开始为测试文件夹命名",loglib::WARNING);
            _name=get_problem_name();
        }
        // 设定测试文件夹路径
        if(_basePath=="."||_basePath.empty()){
            set_basePath();
        }
        // 初始化日志
        _testlog.set_logPath(_basePath);
        _testlog.set_logName("test.log");
        _testlog.tlog("测试日志开始运行");
        // 初始化测试配置
        init_test_config();
        // 重设需要的文件路径
        _problemfile=_basePath/"problem.md";
        _testfile=_basePath/"test.cpp";
        _ACfile=_basePath/"AC.cpp";
        // 写入文件
        wfile(_problemfile,_problem);
        wfile(_testfile,_testCode);
        wfile(_ACfile,_ACCode);
        _testlog.tlog("文件写入成功");
        // 初始化历史记录
        init_system();
        // 初始化错误样例集合
        _WAdatas.set_path(_baseConfigPath/"WAdatas.json");
        if(!_WAdatas.exist()){
            _log.tlog("正在初始化错误样例集合");
            _WAdatas.value()=json::array();
            _WAdatas.save();
        }
        // 配置可执行文件路径
        _baseProgramPath=_basePath/"exec";
        if(!fs::exists(_baseProgramPath)){
            fs::create_directories(_baseProgramPath);
        }
        // 初始化其他配置
        init_temp();
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
        // 设置配置文件基础文件夹
        _baseConfigPath=_basePath/"config";
        _baseProgramPath=_basePath/"exec";
        // 设置配置文件文件夹
        _config.set_path(_baseConfigPath/"config.json");
        // 读取配置文件
        if(!_config.exist()){
            _log.tlog("配置文件不存在,正在重建",loglib::WARNING);
            _name=path.filename().string();
            init_test_config();
        }
        // 读取配置项
        _name=_config[f(Test_Name)];
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
        _testlog.set_logName("test.log");
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
        // WARING 必须使用 https://..../V1/ 因为这个openai库不会自动补上这个/
        if(tempURL.back()!='/'){
            tempURL+='/';
        }
        _AI.setBaseUrl(tempURL);
        _AI.setToken(_openaiKey.get());
        // 初始化系统提示词
        init_system();
        // 读入错误样例集合
        _WAdatas.set_path(_baseConfigPath/"WAdatas.json");
        if(!_WAdatas.exist()){
            _log.tlog("错误样例集合不存在,正在重建",loglib::WARNING);
            _WAdatas.value()=json::array();
            _WAdatas.save();
        }
        _log.tlog("载入"+_name+"成功");
        _testlog.tlog("重新载入成功");
        // 初始化其他配置
        init_temp();
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
        // 生成文件名
        string targetName=f(name);
        string srcName=targetName+".cpp";
        fs::path targetPath=_baseProgramPath/targetName;
        fs::path srcPath=_basePath/srcName;
        string prompt=_prompt[f(name)];
        // 重试次数
        int tryNums=0;
        while(true){
            // 文件是否存在标识符
            bool srcExist=false;
            // 检查文件是否存在
            if(fs::exists(srcPath)){
                _testlog.tlog(nameStr+"源文件已经存在");
                srcExist=true;
            }
            else{
                _testlog.tlog("正在生成"+nameStr);
                // if(tryNums==0) _config["tool_choice"]="required";
                // 处理请求
                AI(prompt,session);
                // _config["tool_choice"]="auto";
                // 读取数据
                json result;
                try{
                    result=json::parse(string(session.back()["content"]));
                    _testlog.tlog(nameStr+"生成成功");
                }
                catch(const json::exception &e){
                    _testlog.tlog("JSON解析失败: "+string(e.what()),loglib::ERROR);
                    if(tryNums>=3){
                        _testlog.tlog("JSON解析失败次数过多,请修改提示词",loglib::ERROR);
                        return false;
                    }
                    _testlog.tlog("正在重试第"+std::to_string(tryNums)+"生成"+nameStr,loglib::WARNING);
                    prompt="数据生成器JSON解析失败: "+string(e.what());
                    tryNums++;
                    continue;
                }
                string code=result["code"];
                // 写入文件
                wfile(srcPath,code);
            }
            // 编译文件
            // 如果已经编译则不需要再编译，但是前提是前面不需要重新生成源代码
            if(srcExist&&fs::exists(targetPath)){
                _testlog.tlog(nameStr+"编译文件已经存在");
                return true;
            }
            else{
                _testlog.tlog("正在编译"+nameStr);
                process::Args args("g++");
                args.add(srcPath).add("-o").add(targetPath);
                process::Process proc("/bin/g++",args);
                proc.start();
                process::Status status=proc.wait();
                string error=proc.get_error();
                // 如果不是正常退出输出错误信息
                if(status!=process::STOP){
                    _testlog.tlog(nameStr+"编译失败,编译错误信息: \n"+error,loglib::ERROR);
                    if(tryNums>=3){
                        _testlog.tlog("编译失败次数过多,请检查提示词",loglib::ERROR);
                        return false;
                    }
                    _testlog.tlog("正在重试第"+std::to_string(tryNums)+"次编译"+nameStr,loglib::WARNING);
                    prompt="编译失败,编译错误信息: "+error;
                    // 删除源文件
                    fs::remove(srcPath);
                    tryNums++;
                    continue;
                }
            }
            break;
        }
        return true;
    }
    // 生成测试工具
    AutoTest &AutoTest::ai_gen(){
        // 添加 testlib 库文件
        fs::path testlibPath=_path/"testlib.h";
        // 目标地址
        fs::path testlib=_basePath/"testlib.h";
        // 复制
        if(!fs::exists(testlib)){
            fs::copy_file(testlibPath,testlib,fs::copy_options::overwrite_existing);
        }
        // 初始化提示词
        json &session=_history.value();
        // 如果没有数据
        if(session.size()<2){
            session.push_back({
                { "role","user" },
                { "content","完整题面为: "+_problem }
                });
            session.push_back({
                { "role","user" },
                { "content","AC代码为: "+_ACCode }
                });
        }
        else{
            // 如果有就更新
            session[1]["content"]="完整题面为: "+_problem;
            session[2]["content"]="AC代码为: "+_ACCode;
            // 并截断后面的数据
            session.erase(session.begin()+3,session.end());
        }
        // 合并prompt到system
        session[0]["content"]=session[0]["content"].get<string>()+"\n"+session[1]["content"].get<string>()+"\n"+session[2]["content"].get<string>();
        // 截断数据
        session.erase(session.begin()+1,session.end());
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
    AutoTest::Exit AutoTest::run(fs::path program,process::Args args,fs::path infile,fs::path outfile,bool setLimit){
        // 运行测试
        process::Process proc;
        // 返回值
        Exit res;
        proc.load(program,args);
        // _testlog.tlog("正在运行"+program.string());
        // 如果路径不为空，输入文件
        if(!infile.empty()){
            proc.set_stdin(infile);
        }
        // 如果路径不为空，输出文件
        if(!outfile.empty()){
            proc.set_stdout(outfile);
        }
        // auto config=_config.get<ns::TestConfig>();
        if(setLimit){
            proc.set_memout(_config[f(MemLimit)]);
            proc.set_timeout(_config[f(TimeLimit)]);
        }
        proc.start();
        // 等待运行结束
        res.status=proc.wait();
        res.exit_code=proc.get_exit_code();
        res.error=proc.get_error();
        // 去除回车
        if(*res.error.rbegin()=='\n'){
            res.error.pop_back();
        }
        if(outfile.empty()){
            res.content=proc.read();
        }
        return res;
    }
    // 保存到文件
    void AutoTest::append_to(const fs::path &filePath,const string &content){
        // 追加到文件
        std::ofstream file(filePath,std::ios::app);
        if(!file.is_open()){
            _testlog.tlog("无法打开文件: "+filePath.string(),loglib::ERROR);
            throw std::runtime_error("无法打开文件: "+filePath.string());
        }
        file<<content;
        file.close();
    }
    // 生成指定权重数字
    int AutoTest::random_weight(int val0,int val1,int val2){
        // 生成指定权重数字 按照val012的大小按权分配，生成加权概率的0，1，2三个数字
        static std::mt19937 gen(static_cast<unsigned>(std::time(nullptr)));

        // 计算总权重
        int totalWeight=val0+val1+val2;
        // 权重数组
        std::vector<int> weights={ val0,val1,val2 };
        // 生成1到总权重之间的随机数
        std::uniform_int_distribution<> dist(1,totalWeight);
        int randomNum=dist(gen);

        // 根据权重选择数字
        int cumulativeWeight=0;
        for(size_t i=0; i<weights.size(); ++i){
            cumulativeWeight+=weights[i];
            if(randomNum<=cumulativeWeight){
                return i;
            }
        }

        return 0; // 防止意外情况
    }
    // 生成随机字符串
    string AutoTest::random_string(int length){
        // 生成随机字符串
        static const char alphanum[]="0123456789abcdefghijklmnopqrstuvwxyz";
        static std::mt19937 gen(std::chrono::system_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<> dist(0,sizeof(alphanum)-2); // -2 because of null terminator

        string result;
        result.reserve(length);
        for(int i=0; i<length; i++){
            result+=alphanum[dist(gen)];
        }
        return result;
    }
    // 生成数据
    bool AutoTest::generate_data(int testnum){
        // 检测是否已经编译和生成
        if(fs::exists(_baseProgramPath/f(Generators))&&
            fs::exists(_baseProgramPath/f(Validators))&&
            fs::exists(_baseProgramPath/f(Checkers))){
            _log.tlog("测试文件已经编译,开始生成数据");
        }
        else{
            _log.tlog("测试文件不存在,请先编译",loglib::ERROR);
            return false;
        }

        // 循环生成并校验数据直到数据符合题目要求
        while(testnum--){
            // auto config=_config.get<ns::TestConfig>();
            int num=_config[f(NowData)];
            // int num=config.now_data+1;
            num++;
            string info="第"+std::to_string(num)+"个测试点";
            _testlog.tlog("生成"+info);
            // 读取计数
            string dataName="data"+std::to_string(num);
            _config[f(DataNum)]=dataName;
            // 设置路径
            process::Args args;
            // 特判生成数量
            int Special_nums=_config[f(Special)];
            // 边界生成数量
            int Edge_nums=_config[f(Edge)];
            // 生成随机哈希
            string hash=random_string(8);
            _randomSeed=dataName+" : "+hash+"\n";
            // 是否启用权重输出
            if(_config[f(Test_Weight)]){
                // 生成权重
                int weight=random_weight(_config[f(Weights)][0],
                    _config[f(Weights)][1],
                    _config[f(Weights)][2]);
                args.add(f(Generators)).add(weight).add(hash);
            }
            else{
                if(Special_nums>0){
                    args.add(f(Generators)).add(1).add(hash);
                    Special_nums--;
                }
                else if(Edge_nums>0){
                    args.add(f(Generators)).add(2).add(hash);
                    Edge_nums--;
                }
                else{
                    args.add(f(Generators)).add(0).add(hash);
                }
            }
            // 生成数据并检查数据是否符合要求
            // 同步设置
            // _config.sync<ns::TestConfig>();
            Exit res=run(_baseProgramPath/f(Generators),
                args,"",
                _dataDirs[inData]/(dataName+".in"),
                false);
            if(res.status==process::STOP){
                _testlog.tlog(info+": 数据生成器运行成功");
            }
            else{
                _testlog.tlog(info+": 数据生成器运行失败,错误信息："+res.error,loglib::ERROR);
                return false;
            }
            // 运行数据验证器
            args.clear();
            args.add(f(Validators));
            res=run(_baseProgramPath/f(Validators),args,_dataDirs[inData]/(dataName+".in"),"",false);
            if(res.status==process::STOP){
                _testlog.tlog(info+": 数据验证成功");
                _config[f(NowData)]=num;
                _config[f(Special)]=Special_nums;
                _config[f(Edge)]=Edge_nums;
                _config.save();
                append_to(_baseConfigPath/"seed.txt",_randomSeed);
            }
            else if(res.status==process::ERROR){
                _testlog.tlog(info+": 数据生成不符合要求，正在重新生成。"+
                    "不符合信息："+res.error,
                    loglib::WARNING);
                // 重新生成本次数据
                testnum++;
                continue;
            }
            else{
                _testlog.tlog(info+": 数据验证器运行失败。"+
                    "错误信息："+res.error+
                    "失败信息:"+res.content,
                    loglib::ERROR);
                return false;
            }
        }
        return true;
    }
    // 测试数据
    bool AutoTest::test_data(){
        process::Args args;
        // 检测测试代码和AC代码是否编译
        if(!fs::exists(_baseProgramPath/f(Test_Code))){
            args.clear();
            // 编译test代码
            args.add("g++").add(_testfile).add("-o").add(_baseProgramPath/f(Test_Code));
            _testlog.tlog("正在编译测试代码");
            Exit res=run("/bin/g++",args,"","",false);
            // 如果不是正常退出输出错误信息
            if(res.status!=process::STOP){
                _testlog.tlog("测试代码编译失败\n编译错误信息: "+res.error
                    ,loglib::ERROR);
                return false;
            }
        }
        if(!fs::exists(_baseProgramPath/f(AC_Code))){
            args.clear();
            // 编译AC代码
            args.add("g++").add(_ACfile).add("-o").add(_baseProgramPath/f(AC_Code));
            _testlog.tlog("正在编译AC代码");
            Exit res=run("/bin/g++",args,"","",false);
            // 如果不是正常退出输出错误信息
            if(res.status!=process::STOP){
                _testlog.tlog("AC代码编译失败\n编译错误信息: "+res.error
                    ,loglib::ERROR);
                return false;
            }
        }
        // 检测测试数据是否已经生成
        if(_config.value().find(f(DataNum))!=_config.value().end()){
            _log.tlog("测试数据已经生成,开始测试数据");
        }
        else{
            _log.tlog("测试数据不存在,请先生成数据",loglib::ERROR);
            return false;
        }
        // auto config=_config.get<ns::TestConfig>();
        int target_num=_config[f(NowData)];
        int num=_config[f(NowTest)];
        // 检测第num个测试点文件是否存在
        while(true){
            string fileName="data"+std::to_string(num)+".in";
            if(!fs::exists(_dataDirs[inData]/fileName)){
                num++;
            }
            else{
                break;
            }
            if(num>target_num){
                _testlog.tlog("没有可以测试的测试点",loglib::WARNING);
            }
        }
        // int target_num=config.now_data;
        // int &num=config.now_test;
        // 循环验证数据直到找到不一致的数据
        do{
            if(num>target_num) break;
            args.clear();
            string info="第"+std::to_string(num)+"个测试点";
            _testlog.tlog("测试"+info);
            // 读取计数
            string dataName="data"+std::to_string(num);
            _config[f(DataNum)]=dataName;
            // config.data_num=dataName;
            // 运行对应的Test代码
            args.add(f(Test_Code));
            Exit res=run(_baseProgramPath/f(Test_Code),
                args,
                _dataDirs[inData]/(dataName+".in"),
                _dataDirs[outData]/(dataName+".out"));
            JudgeCode temp;
            if(res.status==process::STOP){
                temp=judge(res.status,res.exit_code);
                _testlog.tlog(info+": 测试代码已运行");
                _config[f(JudgeStatus)]=f(temp);
                _config.save();
            }
            else{
                _testlog.tlog(info+": 测试代码运行失败,错误信息: "+res.error,loglib::ERROR);
                return false;
            }
            // 运行对应的AC代码
            args.clear();
            args.add(f(AC_Code));
            res=run(_baseProgramPath/f(AC_Code),
                args,
                _dataDirs[inData]/(dataName+".in"),
                _dataDirs[acData]/(dataName+".out"));
            if(res.status==process::STOP){
                _testlog.tlog(info+": AC代码已运行");
                temp=judge(res.status,res.exit_code);
                if(temp!=Waiting){
                    _testlog.tlog("AC代码出现问题, 状态: "+f(temp)+
                        "错误信息: "+res.error
                        ,loglib::ERROR);
                    return false;
                }
            }
            else{
                _testlog.tlog(info+": AC代码运行失败,错误信息: "+res.error,loglib::ERROR);
                return false;
            }
            // 如果已经判题
            if(_config[f(JudgeStatus)].get<string>()!=f(Waiting)){
                _testlog.tlog("第"+std::to_string(num)+"个测试点,状态: "+string(_config[f(JudgeStatus)]),loglib::WARNING);
                // 把当前样例加入错误集合
                add_WAdatas();
                _config[f(NowTest)]=num+1;
                continue;
            }
            // 开始判题
            // 运行数据检查器
            args.clear();
            args.add(f(Checkers)).add(_dataDirs[inData]/(dataName+".in")).add(_dataDirs[outData]/(dataName+".out")).add(_dataDirs[acData]/(dataName+".out"));
            // 运行数据检查器
            res=run(_baseProgramPath/f(Checkers),
                args,
                "",
                "",
                false);
            if(res.status==process::STOP){
                _config[f(JudgeStatus)]=f(Accept);
                _testlog.tlog(info+": "+f(Accept));
                // 更新配置
                _config[f(NowTest)]=num+1;
                continue;
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
                    else if(actual_code==10){
                        _testlog.tlog(info+"数据检查器运行失败。"+"\n输出信息:"+res.content+"\n错误信息:"+res.error,loglib::ERROR);
                        return false;
                    }
                    else{
                        _testlog.tlog(info+"数据检查器运行失败。\n未知退出状态:"+res.error+"\n输出信息:"+res.content,loglib::ERROR);
                        return false;
                    }
                    _testlog.tlog(info+": 状态 "+string(_config[f(JudgeStatus)]),loglib::WARNING);
                    // 当前样例添加到错误集合
                    add_WAdatas();
                    // 更新配置
                    _config[f(NowTest)]=num+1;
                    continue;
                }
            }
            else{
                string statusString;
                if(res.status==process::RE){
                    statusString="RuntimeError";
                }
                else if(res.status==process::TIMEOUT){
                    statusString="TimeOut";
                }
                else if(res.status==process::MEMOUT){
                    statusString="MemoryOut";
                }
                else{
                    statusString="未知错误";
                }
                _testlog.tlog(info+"数据检查器运行失败。\n退出状态:"+statusString+"\n错误信息:"+res.error+"\n输出信息:"+res.content,loglib::ERROR);
                return false;
            }
        }
        while(num++);
        return true;
    }
    // 开始自动对拍
    bool AutoTest::start(){
        // 开始运行
        // 循环验证数据直到找到不一致的数据
        int error_nums=_config[f(ErrorLimit)];
        while(true){
            // 生成数据
            if(!generate_data()){
                return false;
            }
            // 测试数据
            if(!test_data()){
                return false;
            }
            // 判断是否达到错误限制
            if(_config[f(JudgeStatus)]!=f(Accept)){
                error_nums--;
                if(error_nums<=0){
                    _testlog.tlog("错误限制达到,自动对拍结束",loglib::WARNING);
                    return false;
                }
            }
        }
    }
    // 添加错误集合
    void AutoTest::add_WAdatas(){
        string dataName=_config[f(DataNum)];
        string in=rfile(_dataDirs[inData]/(dataName+".in"));
        string out=rfile(_dataDirs[acData]/(dataName+".out"));
        // 添加到错误样例集合
        json temp={
            { "in",in },
            { "out",out }
        };
        // 去重
        if(std::find(_WAdatas.value().begin(),_WAdatas.value().end(),temp)!=_WAdatas.value().end()){
            _testlog.tlog("错误样例已经存在,跳过添加",loglib::WARNING);
            return;
        }
        _WAdatas.value().push_back(temp);
        _WAdatas.save();
        add_to_cph();
    }
    string AutoTest::search_test_cph(){
        // 如果cph路径被赋值才会执行
        if(_cph=="."||_cph.empty()){
            return "";
        }
        // 在cph文件夹中找到包含name的文件
        std::vector<fs::path> matched_files;

        string waitName=_config["origin_name"];
        waitName="."+waitName+"_";
        try{
            // 遍历CPH文件夹中的所有文件
            for(const auto &entry:fs::directory_iterator(_cph)){
                // 检查是否是文件且文件名包含指定的字符串
                if(fs::is_regular_file(entry)&&entry.path().filename().string().find(waitName)!=string::npos){
                    matched_files.push_back(entry.path());
                    _testlog.tlog("找到匹配的CPH文件: "+entry.path().string());
                }
            }

            if(matched_files.empty()){
                _testlog.tlog("CPH: 未找到包含 "+_name+" 的CPH文件",loglib::WARNING);
                return "";
            }
        }
        catch(const std::exception &e){
            _testlog.tlog("CPH: 查找文件时出错: "+string(e.what()),loglib::ERROR);
        }
        return matched_files[0];
    }
    void AutoTest::add_to_cph(){
        if(_config["cph_file"].empty()){
            return;
        }
        // 以json的方式打开这个文件
        fs::path first_file=_config["cph_file"];
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
                json temp=_WAdatas.value().back();

                // 创建新的测试用例
                json new_test={
                    { "id",cph_json["tests"].size()+1 },
                    { "input",temp["in"] },
                    { "output",temp["out"] }
                };
                // 如果已经存在则不添加
                if(std::find(cph_json.begin(),cph_json.end(),new_test)!=cph_json.end()){
                    _testlog.tlog("CPH文件中已经存在该测试用例",loglib::WARNING);
                    return;
                }
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