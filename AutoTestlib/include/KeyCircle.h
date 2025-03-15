#ifndef KEY_CIRCLE_H
#define KEY_CIRCLE_H

#include <string>
#include <filesystem>

#include "Self.h"

class KeyCircle{
private:
    string _openaiKey;
    fs::path _keyPath;

public:
    // 构造函数，接收密钥文件路径
    KeyCircle();
    KeyCircle(const fs::path &keyPath);
    // 设置密钥文件路径
    void set_path(const fs::path &keyPath);
    // 检查密钥文件是否存在且不为空
    bool exist();
    // 获取密钥内容
    string get();
    // 保存密钥到文件
    void save(const string &theKey);
};

#endif // KEY_CIRCLE_H
