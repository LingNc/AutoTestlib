#include "AutoConfig.h"

namespace acm{
    // 配置转换器
    string f(ConfigSign config){
        switch(config){
        case Allow_Path:
            return "allow_path";
        case OpenAI_URL:
            return "openai_url";
        case Test_Name:
            return "name";
        case Floder_Number:
            return "floder";
        case Attach_Global:
            return "attach_mode";
        case Model:
            return "model";
        case Named_Model:
            return "named_model";
        case Model_Config:
            return "model_config";
        case Prompt:
            return "prompt";
        case Temperature:
            return "temperature";
        case Max_Token:
            return "max_tokens";
        case Top_P:
            return "top_p";
        case Tools:
            return "tools";
        case General:
            return "general";
        case Generators:
            return "generators";
        case Validators:
            return "validators";
        case Checkers:
            return "checkers";
        case Interactors:
            return "interactors";
        case DataNum:
            return "data_num";
        case NowData:
            return "now_data";
        case NowTest:
            return "now_test";
        case AC_Code:
            return "ac_code";
        case Test_Code:
            return "test_code";
        case TimeLimit:
            return "time_limit";
        case MemLimit:
            return "mem_limit";
        case ErrorLimit:
            return "error_limit";
        case JudgeStatus:
            return "judge_status";
        case Special:
            return "special";
        case Edge:
            return "edge";
        default:
            throw std::runtime_error("未知配置项");
        }
    }

    // 配置类构造函数
    AutoConfig::AutoConfig(){}
    AutoConfig::AutoConfig(const fs::path &file){
        set_path(file);
    }
    // 设置路径
    void AutoConfig::set_path(const fs::path &file){
        _filePath=file;
        exist();
        std::ifstream configfile(_filePath);
        if(!configfile.is_open()){
            // 文件不存在则创建新的文件
            std::ofstream newFile(_filePath);
            if(!newFile.is_open()){
                throw std::runtime_error("AutoConfig: 无法创建文件: "+_filePath.string());
            }
            newFile.close();
            _config=json::object();
        }
        else
            _config=json::parse(configfile);
    }
    // 检查配置文件是否存在且不为空
    bool AutoConfig::exist(){
        // 获取文件的父目录
        fs::path parentPath=_filePath.parent_path();
        // 文件目录不存在
        if(!fs::exists(parentPath)){
            fs::create_directory(parentPath);
        }
        // 检查文件是否存在
        if(!fs::exists(_filePath)){
            return false;
        }
        // 检查文件是否为空
        size_t fileSize=fs::file_size(_filePath);
        return fileSize!=(size_t)0;
    }
    // 保存到配置文件
    void AutoConfig::save(size_t dumpNum){
        std::ofstream file(_filePath,std::ios::out);
        if(!file.is_open()){
            throw std::runtime_error("AutoConfig: 无法打开文件: "+_filePath.string());
        }
        file<<_config.dump(dumpNum);
        file.close();
    }
    // 获取原数据
    json &AutoConfig::get(){
        return _config;
    }
}