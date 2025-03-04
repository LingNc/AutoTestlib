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

    // 新增测试：获取字符
    pc::Process echoCharProc("/bin/echo", pc::Args("echo").add("ABC"));
    echoCharProc.start();
    char c = echoCharProc.getc();
    report_test("Process单字符读取", c == 'A');

    // 新增测试：完整的管道输入/输出
    pc::Process sortProc("/bin/sort", pc::Args("sort"));
    sortProc.start();
    sortProc << "c" << std::endl;
    sortProc << "a" << std::endl;
    sortProc << "b" << std::endl;

    // 关闭标准输入以通知sort完成输入
    sortProc.write(""); // 触发flush
    sortProc.close();

    std::string sortOut1 = sortProc.getline();
    std::string sortOut2 = sortProc.getline();
    std::string sortOut3 = sortProc.getline();
    report_test("Process排序输入输出",
                sortOut1 == "a" &&
                sortOut2 == "b" &&
                sortOut3 == "c");

    // 新增测试：空参数构造和延迟加载
    pc::Process delayedProc;
    delayedProc.load("/bin/echo", pc::Args("echo").add("延迟加载测试"));
    delayedProc.start();
    std::string delayedOutput = delayedProc.getline();
    report_test("Process延迟加载", delayedOutput == "延迟加载测试");

    // 新增测试：内存限制
    pc::Process memProc("/usr/bin/python3", pc::Args("python3").add("-c").add("import numpy as np; a = np.ones((100, 100))"));
    memProc.set_memout(10); // 10MB内存限制
    memProc.start();
    JudgeCode memResult = memProc.wait();
    report_test("Process无内存超限", memResult != JudgeCode::MemoryLimitExceeded);

    // 新增测试：严格内存限制 (创建一个超大数组应该会超出内存限制)
    pc::Process memLimitProc("/usr/bin/python3", pc::Args("python3").add("-c").add("import numpy as np; a = np.ones((5000, 5000))"));
    memLimitProc.set_memout(1); // 1MB内存限制，应该会超出
    memLimitProc.start();
    JudgeCode memLimitResult = memLimitProc.wait();
    report_test("Process内存限制触发",
        memLimitResult==JudgeCode::MemoryLimitExceeded);// ||
                // memLimitResult == JudgeCode::RuntimeError); // 可能触发RuntimeError或MemoryLimitExceeded

    // 新增测试：环境变量操作
    pc::Process envManipProc("/usr/bin/env", pc::Args("env"));
    envManipProc.set_env("VAR1", "value1");
    envManipProc.set_env("VAR2", "value2");
    envManipProc.unset_env("VAR1");
    envManipProc.start();
    std::string envOutput = envManipProc.read(pc::OUT);
    report_test("Process环境变量管理",
                envOutput.find("VAR2=value2") != std::string::npos &&
                envOutput.find("VAR1=value1") == std::string::npos);

    // 新增测试：流操作符的复杂使用
    pc::Process grepProc("/bin/grep", pc::Args("grep").add("pattern"));
    grepProc.start();
    grepProc << "no pattern here" << std::endl;
    grepProc << "this has pattern inside" << std::endl;
    grepProc << "no match here either" << std::endl;
    grepProc.close();
    std::string grepOut = grepProc.getline();
    report_test("Process复杂流操作", grepOut == "this has pattern inside");

    // 新增测试：读取标准错误输出
    pc::Process stderrProc("/bin/bash", pc::Args("bash").add("-c").add("echo 'standard output'; echo 'error output' >&2"));
    stderrProc.start();
    std::string stdOut = stderrProc.getline();
    std::string stdErr = stderrProc.read(pc::ERR);
    report_test("Process标准错误读取",
                stdOut == "standard output" &&
                stdErr.find("error output") != std::string::npos);

    // 新增测试：取消超时
    pc::Process cancelTimeoutProc("/bin/sleep", pc::Args("sleep").add("0.5"));
    cancelTimeoutProc.set_timeout(1000);
    cancelTimeoutProc.cancel_timeout();
    cancelTimeoutProc.start();
    JudgeCode cancelResult = cancelTimeoutProc.wait();
    report_test("Process取消超时", cancelResult == JudgeCode::Waiting);

    // 新增测试：取消内存限制
    pc::Process cancelMemProc("/usr/bin/python3", pc::Args("python3").add("-c").add("import numpy as np; a = np.ones((1000, 1000))"));
    cancelMemProc.set_memout(1);
    cancelMemProc.cancel_memout();
    cancelMemProc.start();
    JudgeCode cancelMemResult = cancelMemProc.wait();
    report_test("Process取消内存限制", cancelMemResult == JudgeCode::Waiting);

    // 新增测试：清除所有环境变量
    pc::Process clearEnvProc("/usr/bin/env", pc::Args("env"));
    clearEnvProc.set_env("TEST_VAR1", "value1");
    clearEnvProc.set_env("TEST_VAR2", "value2");
    clearEnvProc.clear_env();
    clearEnvProc.start();
    std::string clearEnvOutput = clearEnvProc.read(pc::OUT);
    report_test("Process清除环境变量",
                clearEnvOutput.find("TEST_VAR1") == std::string::npos &&
                clearEnvOutput.find("TEST_VAR2") == std::string::npos);

    // 新增测试：获取环境变量
    pc::Process getEnvProc("/bin/bash", pc::Args("bash"));
    getEnvProc.set_env("CUSTOM_PATH", "/custom/path");
    std::string envVal = getEnvProc.get_env("CUSTOM_PATH");
    std::string sysEnv = getEnvProc.get_env("PATH"); // 获取系统环境变量
    std::string nonExistEnv = getEnvProc.get_env("NON_EXISTENT_VAR");
    report_test("Process获取环境变量",
                envVal == "/custom/path" &&
                !sysEnv.empty() &&
                nonExistEnv.empty());

    // 新增测试：阻塞与非阻塞模式切换
    pc::Process blockProc("/bin/cat", pc::Args("cat"));
    blockProc.start();
    blockProc.set_block(false);
    blockProc << "test blocking" << std::endl;
    std::string blockOut = blockProc.getline();
    blockProc.set_block(true);
    blockProc << "test after switch" << std::endl;
    std::string blockOut2 = blockProc.getline();
    report_test("Process阻塞模式切换",
                blockOut == "test blocking" &&
                blockOut2 == "test after switch");

    // 新增测试：进程退出码检测
    pc::Process exitCodeProc("/bin/bash", pc::Args("bash").add("-c").add("exit 42"));
    exitCodeProc.start();
    exitCodeProc.wait();
    report_test("Process退出码检测", exitCodeProc._exit_code != 0);

    // 新增测试：运行时错误检测
    pc::Process segfaultProc("/bin/bash", pc::Args("bash").add("-c").add("kill -SIGSEGV $$"));
    segfaultProc.start();
    JudgeCode segResult = segfaultProc.wait();
    report_test("Process段错误检测", segResult == JudgeCode::RuntimeError);

    // 新增测试：浮点错误检测
    pc::Process fpeProc("/bin/bash", pc::Args("bash").add("-c").add("kill -SIGFPE $$"));
    fpeProc.start();
    JudgeCode fpeResult = fpeProc.wait();
    report_test("Process浮点错误检测", fpeResult == JudgeCode::FloatingPointError);

    std::cout << "Process类测试完成，共执行了 " << 29 << " 项测试" << std::endl;
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
