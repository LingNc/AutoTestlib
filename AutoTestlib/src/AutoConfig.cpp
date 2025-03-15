#include "AutoConfig.h"

namespace acm{
    // 配置转换器
    string f(ConfigSign config){
        switch(config){
        case Allow_Path:
            return str(Allow_Path);
        case AC_Path:
            return str(AC_Path);
        case Test_Path:
            return str(Test_Path);
        case Problem_Path:
            return str(Problem_Path);
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
        exist();
        _filePath=file;
        std::ifstream configfile(_filePath);
        if(!configfile.is_open()){
            throw std::runtime_error("无法打开文件: "+_filePath.string());
        }
        _config=json::parse(configfile);
    }
    // 检查配置文件是否存在且不为空
    bool AutoConfig::exist(){
        // 获取文件的父目录
        fs::path parentPath=_filePath.parent_path();

        // 文件目录不存在
        if(!fs::exists(parentPath)){
            try{
                fs::create_directory(parentPath);
                std::cerr<<"目录创建失败，可能是权限不够或者上级目录不存在。"<<std::endl;
            }
            catch(const fs::filesystem_error &e){
                throw std::runtime_error("创建目录时出错: "+std::string(e.what()));
            }
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
    void AutoConfig::save(){
        std::ofstream file(_filePath);
        if(!file.is_open()){
            throw std::runtime_error("无法打开文件: "+_filePath.string());
        }
        file<<_config.dump(4);
        file.close();
    }
}