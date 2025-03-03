#include <iostream>
#include <filesystem>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <cstdlib>
#include "ext/openai.hpp"
#include "include/KeyCircle.h"
#include "include/Process.h"
#include "include/Self.h"

namespace ai=openai;

fs::path configPath="./config";
string keyName="openai.key";

int main(){
        // 使用路径拼接
    // fs::path keyFilePath=configPath/keyName;

    // KeyCircle key(keyFilePath);
    // if(!key.exist()){
    //     std::cout<<"请输入OpenAi密钥:";
    //     string tempKey;
    //     std::cin>>tempKey;
    //     key.save(tempKey);
    // }
    // auto &llm=ai::start(key.get(),"SiliconCloud",true,"https://api.siliconflow.cn/v1/");

    // auto models=llm.model.list();
    fs::path file="./hello";
    std::vector<string> args;
    args.push_back("hello");
    Process proc;
    proc.load(file,args);
    proc.start();
    string temp;
    std::cin>>temp;
    proc<<temp;
    proc>>temp;
    std::cout<<temp;
    return 0;
}