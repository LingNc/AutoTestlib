#include "KeyCircle.h"
#include "Self.h"
#include <fstream>

KeyCircle::KeyCircle(){}
KeyCircle::KeyCircle(const fs::path &keyPath):_keyPath(keyPath){}

void KeyCircle::set_path(const fs::path &keyPath){
    _keyPath=keyPath;
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
    std::fstream file(_keyPath,std::ios::in);
    string key;
    if(file.is_open()){
        file>>key;
        file.close();
    }
    return key;
}

void KeyCircle::save(const string &theKey){
    std::fstream file(_keyPath,std::ios::out);
    if(file.is_open()){
        file<<theKey;
        file.close();
    }
}
