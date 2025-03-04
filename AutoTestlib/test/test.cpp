#include <iostream>
#include <filesystem>
#include <string>
#include <sstream>
#include <fstream>
#include <chrono>
#include "../include/KeyCircle.h"
#include "../include/Process.h"
#include "../include/Self.h"
#include "../include/JudgeSign.h"

namespace fs = std::filesystem;
namespace pc = process;

// 测试辅助函数 - 显示测试结果
void report_test(const std::string& test_name, bool success) {
    if (success) {
        std::cout << "✓ " << test_name << " 测试通过!" << std::endl;
    } else {
        std::cerr << "✗ " << test_name << " 测试失败!" << std::endl;
        exit(1);
    }
}

// 测试Args类
void test_args() {
    std::cout << "\n===== 测试 Args 类 =====" << std::endl;

    // 测试方式1：使用构造函数和add方法
    pc::Args args1("hello");
    args1.add("-a").add("-b").add("文件名");
    report_test("Args构造函数和add方法", args1.get_program_name() == "hello" && args1.size() == 4);

    // 测试方式2：使用bash构造
    pc::Args args2("bash");
    report_test("Args bash构造", args2.get_program_name() == "bash" && args2.size() == 1);

    // 测试方式3：使用parse方法
    pc::Args args3;
    args3.parse("find . -name \"*.cpp\" -type f");
    report_test("Args parse方法", args3.get_program_name() == "find" && args3.size() == 5);
    report_test("Args parse内容正确", args3[1] == "." && args3[2] == "-name" && args3[3] == "\"*.cpp\"" && args3[4] == "-type" && args3[5] == "f");

    // 测试set_program_name方法
    pc::Args args4;
    args4.set_program_name("test").add("arg1").add("arg2");
    report_test("Args set_program_name", args4.get_program_name() == "test");

    // 测试clear方法
    args4.clear();
    report_test("Args clear方法", args4.size() == 0);

    // 测试索引操作符
    pc::Args args5("program");
    args5.add("arg1").add("arg2");
    report_test("Args索引操作符", args5[0] == "program" && args5[1] == "arg1" && args5[2] == "arg2");

    // 测试向量构造
    std::vector<string> vec_args = {"prog", "-a", "-b", "--option"};
    pc::Args args6(vec_args);
    report_test("Args向量构造", args6.size() == 4 && args6[0] == "prog");

    // 测试添加多个参数
    pc::Args args7("cmd");
    args7.add(std::vector<string>{"-x", "-y", "-z"});
    report_test("Args添加多个参数", args7.size() == 4 && args7[1] == "-x" && args7[3] == "-z");

    // 测试data()方法生成的C风格参数
    pc::Args args8("test");
    args8.add("-v").add("value");
    char** c_args = args8.data();
    report_test("Args data转换",
        std::string(c_args[0]) == "test" &&
        std::string(c_args[1]) == "-v" &&
        std::string(c_args[2]) == "value" &&
        c_args[3] == nullptr);
}

// 测试Process类
void test_process() {
    std::cout << "\n===== 测试 Process 类 =====" << std::endl;

    // 测试简单命令执行
    pc::Args echoArgs("echo");
    echoArgs.add("测试成功");
    pc::Process echoProc("/bin/echo", echoArgs);
    echoProc.start();
    std::string output = echoProc.getline();
    report_test("Process基本执行", output == "测试成功");

    // 测试非阻塞模式
    pc::Process lsProc("/bin/ls", pc::Args("ls"));
    lsProc.start();
    lsProc.set_block(false);
    bool hasOutput = false;
    while (!lsProc.empty()) {
        if (!lsProc.getline().empty()) {
            hasOutput = true;
            break;
        }
    }
    report_test("Process非阻塞模式", hasOutput);

    // 测试环境变量
    pc::Process envProc("/usr/bin/env", pc::Args("env"));
    envProc.set_env("TEST_VAR", "test_value");
    envProc.start();
    bool foundEnv = false;
    while (!envProc.empty()) {
        std::string line = envProc.getline();
        if (line == "TEST_VAR=test_value") {
            foundEnv = true;
            break;
        }
    }
    report_test("Process环境变量设置", foundEnv);

    // 测试输入输出流
    pc::Process catProc("/bin/cat", pc::Args("cat"));
    catProc.start();
    std::string testString = "测试输入输出流";
    catProc << testString << std::endl;
    std::string echoedOutput = catProc.getline();
    report_test("Process流操作符", echoedOutput == testString);

    // 测试进程状态
    pc::Process sleepProc("/bin/sleep", pc::Args("sleep").add("0.1"));
    sleepProc.start();
    bool isRunning = sleepProc.is_running();
    sleepProc.wait();
    bool hasStopped = !sleepProc.is_running();
    report_test("Process运行状态检测", isRunning && hasStopped);

    // 测试进程终止
    pc::Process longSleepProc("/bin/sleep", pc::Args("sleep").add("10"));
    longSleepProc.start();
    bool killResult = longSleepProc.kill();
    report_test("Process终止", killResult && !longSleepProc.is_running());

    // 测试超时设置
    pc::Process timeoutProc("/bin/sleep", pc::Args("sleep").add("2"));
    timeoutProc.set_timeout(100); // 100毫秒超时
    auto startTime = std::chrono::steady_clock::now();
    timeoutProc.start();
    JudgeCode result = timeoutProc.wait();
    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    report_test("Process超时设置",
        duration.count() < 1500 && // 确保没有等待完整的2秒
        result == JudgeCode::TimeLimitEXceeded);
}

// 测试KeyCircle类
void test_key_circle() {
    std::cout << "\n===== 测试 KeyCircle 类 =====" << std::endl;

    // 创建临时测试文件
    fs::path testKeyPath = "./test_config/test.key";
    fs::create_directories("./test_config");

    // 测试不存在时情况
    if (fs::exists(testKeyPath)) {
        fs::remove(testKeyPath);
    }
    KeyCircle key1(testKeyPath);
    report_test("KeyCircle不存在检查", !key1.exist());

    // 测试保存和获取密钥
    std::string testKey = "test-api-key-12345";
    key1.save(testKey);
    report_test("KeyCircle保存成功", fs::exists(testKeyPath));

    // 测试从文件读取密钥
    KeyCircle key2(testKeyPath);
    report_test("KeyCircle存在检查", key2.exist());
    report_test("KeyCircle密钥获取", key2.get() == testKey);

    // 测试更新密钥
    std::string newKey = "updated-test-key-67890";
    key2.save(newKey);
    report_test("KeyCircle更新密钥", key2.get() == newKey);

    // 确认文件内容正确
    std::ifstream keyFile(testKeyPath);
    std::string fileContent;
    std::getline(keyFile, fileContent);
    report_test("KeyCircle文件内容", fileContent == newKey);

    // 清理测试文件
    fs::remove(testKeyPath);
    fs::remove("./test_config");
}

// 测试JudgeSign
void test_judge_sign() {
    std::cout << "\n===== 测试 JudgeSign =====" << std::endl;

    // 验证枚举值的正确性
    report_test("Waiting枚举值", JudgeCode::Waiting == 0);
    report_test("Accept枚举值", JudgeCode::Accept == 3);
    report_test("CompilationError枚举值", JudgeCode::CompilationError == 4);
    report_test("WrongAnswer枚举值", JudgeCode::WrongAnswer == 5);
    report_test("TimeLimitEXceeded枚举值", JudgeCode::TimeLimitEXceeded == 6);
    report_test("RuntimeError枚举值", JudgeCode::RuntimeError == 10);
}

// 主测试函数
int main() {
    std::cout << "开始自动测试..." << std::endl;

    test_args();
    test_process();
    test_key_circle();
    test_judge_sign();

    std::cout << "\n所有测试通过!" << std::endl;
    return 0;
}
