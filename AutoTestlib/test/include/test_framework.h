#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <iostream>
#include <string>
#include <functional>
#include <vector>
#include <chrono>
#include <iomanip>

// 测试函数类型
typedef std::function<std::string()> TestFunction;

// 测试信息结构
struct TestInfo {
    std::string name;
    TestFunction func;
};

// judgesign 常量定义
const std::string JUDGESIGN_OK = "JUDGESIGN_OK";
const std::string JUDGESIGN_FAIL = "JUDGESIGN_FAIL";

// 测试套件类
class TestSuite {
private:
    std::string name;
    std::vector<TestInfo> tests;
    int passed = 0;
    int failed = 0;

public:
    TestSuite(const std::string& suite_name) : name(suite_name) {}

    // 添加测试
    void add_test(const std::string& test_name, TestFunction test_func) {
        tests.push_back({test_name, test_func});
    }

    // 运行所有测试
    bool run_all() {
        std::cout << "\n===== 测试套件: " << name << " =====" << std::endl;

        for (const auto& test : tests) {
            try {
                auto start = std::chrono::high_resolution_clock::now();
                std::string info = test.func();
                auto end = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

                std::cout << "✓ [" << test.name << "] 通过! (" << duration << "ms)" << std::endl;
                passed++;
                if (!info.empty()) {
                    std::cout << "  提示: " << info << std::endl;
                }
            }
            catch(const std::exception &e){
                std::cerr << "✗ [" << test.name << "] 失败: " << e.what() << std::endl;
                failed++;
            } catch (...) {
                std::cerr << "✗ [" << test.name << "] 失败: 未知错误" << std::endl;
                failed++;
            }
        }

        std::cout << "\n套件总结: " << passed << " 通过, " << failed << " 失败" << std::endl;
        return failed == 0;
    }

    // 获取通过的测试数量
    int get_passed_count() const {
        return passed;
    }

    // 获取失败的测试数量
    int get_failed_count() const {
        return failed;
    }
};

// 全局测试管理器
class TestManager {
private:
    std::vector<TestSuite> suites;

    // 打印评测标记
    void print_judgesign(bool success) {
        std::cout << "\n" << (success ? JUDGESIGN_OK : JUDGESIGN_FAIL) << std::endl;
    }

public:
    // 添加测试套件
    void add_suite(const TestSuite& suite) {
        suites.push_back(suite);
    }

    // 运行所有测试套件
    bool run_all() {
        std::cout << "开始测试..." << std::endl;

        int total_passed = 0;
        int total_failed = 0;
        auto start_time = std::chrono::high_resolution_clock::now();

        bool all_passed = true;
        for (auto& suite : suites) {
            bool suite_passed = suite.run_all();
            // 累加当前套件的通过和失败数量
            total_passed += suite.get_passed_count();
            total_failed += suite.get_failed_count();
            if (!suite_passed) {
                all_passed = false;
            }
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double>(end_time - start_time).count();

        std::cout << "\n========================================" << std::endl;
        std::cout << "总结: " << total_passed << " 通过, " << total_failed << " 失败" << std::endl;
        if (all_passed) {
            std::cout << "✓ 所有测试通过! (耗时: " << std::fixed << std::setprecision(1) << duration << "秒)" << std::endl;
        } else {
            std::cout << "✗ 测试出现错误! (耗时: " << std::fixed << std::setprecision(1) << duration << "秒)" << std::endl;
        }

        // 打印评测标记
        print_judgesign(all_passed);

        return all_passed;
    }
};

// 测试断言函数 - 使用inline避免多重定义
inline void assert_true(bool condition, const std::string& message = "断言失败") {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

inline void assert_equal(const std::string& actual, const std::string& expected, const std::string& message = "值不相等") {
    if (actual != expected) {
        throw std::runtime_error(message + "\n  期望: '" + expected + "'\n  实际: '" + actual + "'");
    }
}

template<typename T>
inline void assert_equal(const T& actual, const T& expected, const std::string& message = "值不相等") {
    if (actual != expected) {
        throw std::runtime_error(message+"\n  期望: " + std::to_string(expected) + "\n  实际: " + std::to_string(actual));
    }
}

// 添加枚举与整数比较的断言函数
template<typename EnumType>
inline void assert_equal_enum(EnumType actual, int expected, const std::string& message = "枚举值不相等") {
    if (static_cast<int>(actual) != expected) {
        throw std::runtime_error(message + "\n  期望: " + std::to_string(expected) +
                                "\n  实际: " + std::to_string(static_cast<int>(actual)));
    }
}

#endif // TEST_FRAMEWORK_H
