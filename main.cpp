#include <iostream>
#include "AutoTest.h"

int main(){
    acm::AutoTest test;
    // 是否进行一些基础的配置
    std::string config;
    std::cout<<"是否进行一些基础的配置？(y/n): ";
    std::cin>>config;
    if(config=="y"){
        // 更改密钥
        std::string key;
        std::cout<<"请输入OpenAi密钥: ";
        std::cin>>key;
        if(key!="n")
            test.set_key(key);
        // 设置测试文件夹路径
        fs::path path;
        std::cout<<"请输入测试文件夹路径：";
        std::cin>>path;
        if(path!="n")
            test.set_basePath(path);
        // 设置OpenAi 的 base URL
        std::string url;
        std::cout<<"请输入OpenAi API地址: ";
        std::cin>>url;
        if(url!="n")
            test.config(acm::OpenAI_URL,url,acm::Global);
        // 设置默认模型
        std::string model;
        std::cout<<"请输入默认模型: ";
        std::cin>>model;
        if(model!="n")
            test.config(acm::Model,model,acm::Global);
    }
    // 是否从已有文件夹加载
    std::string load;
    std::cout<<"是否从已有文件夹加载？(y/n): ";
    std::cin>>load;
    if(load=="y"){
        fs::path path;
        std::cout<<"请输入文件夹路径：";
        std::cin>>path;
        test.load(path);
    }
    else{
        fs::path testCode,ACCode,problem;
        // std::cout<<"请输入题目路径：";
        // std::cin>>problem;
        problem="./Code/C.md";
        test.set_problem(problem);
        // std::cout<<"请输入测试代码路径：";
        // std::cin>>testCode;
        testCode="./Code/C.cpp";
        test.set_testCode(testCode);
        // std::cout<<"请输入AC代码路径：";
        // std::cin>>ACCode;
        ACCode="./Code/C-ac.cpp";
        test.set_ACCode(ACCode);
        // 初始化
        test.init();
    }
    // 开始运行
    test.ai_gen().start();
    return 0;
}