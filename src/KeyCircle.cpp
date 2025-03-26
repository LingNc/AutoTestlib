#include "KeyCircle.h"
#include "Self.h"
#include <fstream>

KeyCircle::KeyCircle(){}
KeyCircle::KeyCircle(const fs::path &keyPath){
    set_path(keyPath);
}

void KeyCircle::set_path(const fs::path &keyPath){
    _keyPath=keyPath;
    // 读取密钥
    std::ifstream file(_keyPath);
    if(file.is_open()){
        std::getline(file,_openaiKey);
        file.close();
    }
    else{
        _openaiKey="";
    }
}

bool KeyCircle::exist(){
    // 获取文件的父目录
    fs::path parentPath=_keyPath.parent_path();
    // 文件目录不存在
    if(!fs::exists(parentPath)){
        fs::create_directory(parentPath);
    }
    // 检查文件是否存在
    if(!fs::exists(_keyPath)){
        return false;
    }
    // 检查文件是否为空
    size_t fileSize=fs::file_size(_keyPath);
    return fileSize!=(size_t)0;
}

string KeyCircle::get(){
    return _openaiKey;
}

void KeyCircle::save(const string &theKey){
    // 保存文件
    std::ofstream file(_keyPath);
    if(file.is_open()){
        file<<theKey;
        file.close();
    }
    else{
        throw std::runtime_error("无法打开文件: "+_keyPath.string());
    }
}
