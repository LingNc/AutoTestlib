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
namespace pc=process;

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
    fs::path file="./Code/hello";

    // 示例1：使用封装后的Args类的标准方式
    pc::Args args1("hello");
    args1.add("-a").add("-b").add("文件名");

    // 示例2：使用新的命令行解析功能
    pc::Args args2("bash");

    // 示例3：先创建Args对象，然后解析命令行
    pc::Args args3;
    args3.parse("find . -name \"*.cpp\" -type f");

    // 使用args2运行ls命令
    pc::Process proc("/bin/bash",args2);
    proc.start();
    // proc<<"ls -l --color=always"<<std::endl;
    string output,input;
    proc.set_block(false);
    while(1){
        std::getline(std::cin,input);
        if(input=="exit") break;
        proc<<input<<std::endl;
        while(1){
            output=proc.getline();
            // if(output!="") proc.setBlock(false);
            if(proc.empty()) break;
            std::cout<<output<<std::endl;
        }
        // proc.setBlock(true);
    }
    return 0;
}