#include "test_framework.h"
#include "../include/KeyCircle.h"
#include <iostream>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

TestSuite create_keycircle_tests() {
    TestSuite suite("KeyCircle类");

    // 测试不存在情况
    suite.add_test("不存在检查", []() {
        // 准备测试环境
        fs::path testKeyPath = "./test_config/test.key";
        if (fs::exists(testKeyPath)) {
            fs::remove(testKeyPath);
        }

        if (!fs::exists("./test_config")) {
            fs::create_directories("./test_config");
        }

        KeyCircle key(testKeyPath);
        assert_true(!key.exist(), "文件不存在时应返回false");

        // 清理
        if (fs::exists("./test_config")) {
            fs::remove_all("./test_config");
        }
    });

    // 测试保存和获取密钥
    suite.add_test("保存和获取", []() {
        // 准备测试环境
        fs::path testKeyPath = "./test_config/test.key";
        if (!fs::exists("./test_config")) {
            fs::create_directories("./test_config");
        }

        // 测试保存
        KeyCircle key1(testKeyPath);
        std::string testKey = "test-api-key-12345";
        key1.save(testKey);
        assert_true(fs::exists(testKeyPath), "保存密钥后文件应存在");

        // 测试获取
        KeyCircle key2(testKeyPath);
        assert_true(key2.exist(), "文件存在时应返回true");
        assert_equal(key2.get(), testKey, "获取的密钥与保存的密钥不匹配");

        // 清理
        fs::remove_all("./test_config");
    });

    // 测试更新密钥
    suite.add_test("更新密钥", []() {
        // 准备测试环境
        fs::path testKeyPath = "./test_config/test.key";
        if (!fs::exists("./test_config")) {
            fs::create_directories("./test_config");
        }

        // 初始化密钥
        KeyCircle key(testKeyPath);
        std::string initialKey = "initial-key";
        key.save(initialKey);

        // 更新密钥
        std::string updatedKey = "updated-key";
        key.save(updatedKey);

        // 验证更新
        assert_equal(key.get(), updatedKey, "更新后的密钥不匹配");

        // 清理
        fs::remove_all("./test_config");
    });

    return suite;
}

