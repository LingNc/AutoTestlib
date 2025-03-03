#include "KeyCircle.h"
#include "Self.h"
#include <fstream>

bool exist_file(const fs::path &filePath){
    // 获取文件的父目录
    fs::path parentPath=filePath.parent_path();

    // 文件目录不存在
    if(!fs::exists(parentPath)){
        try{
            if(fs::create_directory(parentPath)){
                return true;
            }
            std::cerr<<"目录创建失败，可能是权限不够或者上级目录不存在。"<<std::endl;
            return false;
        }
        catch(const fs::filesystem_error &e){
            std::cerr<<"创建目录时出错: "<<e.what()<<std::endl;
            return false;
        }
    }
    // 检查文件是否存在
    return fs::exists(filePath);
}

KeyCircle::KeyCircle(const fs::path &keyPath): keyPath(keyPath){}

bool KeyCircle::exist(){
    if(!exist_file(keyPath)){
        return false;
    }
    size_t fileSize=fs::file_size(keyPath);
    return fileSize!=(size_t)0;
}

string KeyCircle::get(){
    std::fstream file(keyPath,std::ios::in);
    string key;
    if(file.is_open()){
        file>>key;
        file.close();
    }
    return key;
}

void KeyCircle::save(const string &theKey){
    std::fstream file(keyPath,std::ios::out);
    if(file.is_open()){
        file<<theKey;
        file.close();
    }
}
