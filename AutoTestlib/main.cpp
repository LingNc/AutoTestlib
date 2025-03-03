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
namespace pc=Process;

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
    pc::Args args2("ls -l -d -F");

    // 示例3：先创建Args对象，然后解析命令行
    pc::Args args3;
    args3.parse("find . -name \"*.cpp\" -type f");

    // 使用args2运行ls命令
    pc::Process proc;
    proc.load("/bin/ls",args2);
    proc.start();
    string output;
    proc>>output;
    std::cout<<"ls命令输出:\n"<<output<<std::endl;

    return 0;
}