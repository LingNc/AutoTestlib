#include <iostream>
#include <fstream>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cstdlib>
#include "ext/openai.hpp"

using std::string;

namespace fs=std::filesystem;
namespace ai=openai;

string configPath="./config";
string keyName="openai.key";

bool exist_file(const string &filePath){
    size_t namePos=filePath.find_last_of('/',filePath.size());
    string fileName=filePath.substr(namePos,filePath.size());
    string prePath=filePath.substr(0,namePos);
    // 文件目录不存在
    if(!fs::exists(prePath)){
        if(fs::create_directory(prePath)){
            return true;
        }
        std::cerr<<"目录创建失败，可能是权限不够或者上级目录不存在。 "<<std::endl;
        return false;
    }
    // 文件不存在
    if(!fs::exists(filePath)){
        return false;
    }
    return true;
}

class KeyCircle{
    string openaiKey;
    string keyPath;
public:
    KeyCircle(string keyPath):keyPath(keyPath){}
    bool exist(){
        if(!exist_file(keyPath)){
            return false;
        }
        size_t fileSize=fs::file_size(keyPath);
        if(fileSize!=(size_t)0) return true;
        else return false;
    }

    string get(){
        std::fstream file(keyPath);
        string key;
        if(file.is_open()){
            file>>key;
            file.close();
        }
        return key;
    }

    void save(string theKey){
        std::fstream file(keyPath);
        if(file.is_open()){
            file<<theKey;
            file.close();
        }
    }
};

int main(){
    KeyCircle key(configPath+keyName);
    if(!key.exist()){
        std::cout<<"请输入OpenAi密钥:";
        string tempKey;
        std::cin>>tempKey;
        key.save(tempKey);
    }
    ai::start(key.get(),"SiliconCloud",true,"https://api.siliconflow.cn/v1");

    auto models=ai::model().list();
    std::cout<<models<<std::endl;
    return 0;
}