#ifndef KEY_CIRCLE_H
#define KEY_CIRCLE_H

#include <string>
#include <filesystem>

#include "Slef.h"

// 检查文件或其目录是否存在，如果目录不存在则尝试创建
bool exist_file(const std::filesystem::path &filePath);

class KeyCircle{
private:
    string openaiKey;
    std::filesystem::path keyPath;

public:
    // 构造函数，接收密钥文件路径
    KeyCircle(const std::filesystem::path &keyPath);

    // 检查密钥文件是否存在且不为空
    bool exist();

    // 获取密钥内容
    string get();

    // 保存密钥到文件
    void save(const string &theKey);
};

#endif // KEY_CIRCLE_H
