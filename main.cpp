/*
 * AutoTestlib - 算法竞赛自动化测试框架
 * Copyright (C) 2024 绫袅LingNc <1020449099@qq.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <iostream>
#include "AutoTest.h"

// 从文件夹中查找包含特定字符串的文件名返回第一个
string find_file_in(fs::path path,string filename){
    if(!fs::exists(path)){
        std::cerr<<"文件夹不存在: "<<path.string()<<std::endl;
    }
    std::vector<fs::path> matchFiles;
    try{
        for(const auto &entry:fs::directory_iterator(path)){
            if(fs::is_regular_file(entry)&&entry.path().filename().string().find(filename)!=string::npos){
                matchFiles.push_back(entry.path());
            }
        }
        if(matchFiles.empty()){
            return "";
        }
    }
    catch(const std::exception &e){
        std::cerr<<"查找文件时出错: "<<e.what()<<std::endl;
        return "";
    }
    return matchFiles[0];
}

// 是否有测试文件对应的ac和题目文件
bool have_test_files(fs::path testPath){
    fs::path folderPath=testPath.parent_path();
    string testName=testPath.filename().string();
    string acName=testName.substr(0,testName.size()-4)+"-ac.cpp";
    string mdName=testName.substr(0,testName.size()-3)+"md";
    if(find_file_in(folderPath,acName)==""){
        return false;
    }
    if(find_file_in(folderPath,mdName)==""){
        return false;
    }
    return true;
}

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
        std::cout<<"请输入测试代码路径：";
        std::cin>>testCode;
        // testCode="./Code/C.cpp";
        test.set_testCode(testCode);
        if(have_test_files(testCode)){
            problem=testCode.parent_path()/(testCode.filename().string().substr(0,testCode.filename().string().size()-4)+".md");
            ACCode=testCode.parent_path()/(testCode.filename().string().substr(0,testCode.filename().string().size()-4)+"-ac.cpp");

        }
        else{
            std::cout<<"请输入题目路径：";
            std::cin>>problem;
            std::cout<<"请输入AC代码路径：";
            std::cin>>ACCode;
        }
        // problem="./Code/C.md";
        test.set_problem(problem);
        // ACCode="./Code/C-ac.cpp";
        test.set_ACCode(ACCode);
        // 初始化
        test.init();
    }
    // 开始运行
    test.ai_gen().start();
    return 0;
}