#include "test_framework.h"
#include "Process.h"
#include "JudgeSign.h"
#include <iostream>
#include <chrono>

namespace pc = process;

TestSuite create_process_basic_tests() {
    TestSuite suite("Process类-基础功能");

    // 测试基本执行
    suite.add_test("基本执行", []() {
        pc::Args echoArgs("echo");
        echoArgs.add("测试成功");
        pc::Process echoProc("/bin/echo", echoArgs);
        echoProc.start();
        std::string output = echoProc.getline();
        assert_equal(output, std::string("测试成功"));
    });

    // 测试非阻塞模式
    suite.add_test("非阻塞模式", []() {
        pc::Process lsProc("/bin/ls", pc::Args("ls"));
        lsProc.start();
        lsProc.set_block(false);
        bool hasOutput = false;
        std::string line;
        int attempts = 0;
        while (!lsProc.empty() && attempts < 100) {
            line = lsProc.getline();
            if (!line.empty()) {
                hasOutput = true;
                break;
            }
            attempts++;
        }
        assert_true(hasOutput, "非阻塞模式下无法读取输出");
    });

    // 测试环境变量
    suite.add_test("环境变量设置", []() {
        pc::Process envProc("/usr/bin/env", pc::Args("env"));
        envProc.set_env("TEST_VAR", "test_value");
        envProc.start();
        std::string output = envProc.read(pc::OUT);
        assert_true(output.find("TEST_VAR=test_value") != std::string::npos,
                   "未找到设置的环境变量");
    });

    // 测试输入输出流
    suite.add_test("流操作符", []() {
        pc::Process catProc("/bin/cat", pc::Args("cat"));
        catProc.start();
        std::string testString = "测试输入输出流";
        catProc << testString << std::endl;
        std::string echoedOutput = catProc.getline();
        assert_equal(echoedOutput, testString);
    });

    // 测试进程状态
    suite.add_test("运行状态检测", []() {
        pc::Process sleepProc("/bin/sleep", pc::Args("sleep").add("0.1"));
        sleepProc.start();
        bool isRunning = sleepProc.is_running();
        sleepProc.wait();
        bool hasStopped = !sleepProc.is_running();
        assert_true(isRunning && hasStopped, "进程状态检测失败");
    });

    // 测试进程终止
    suite.add_test("进程终止", []() {
        pc::Process longSleepProc("/bin/sleep", pc::Args("sleep").add("10"));
        longSleepProc.start();
        bool killResult = longSleepProc.kill();
        assert_true(killResult && !longSleepProc.is_running(), "进程终止失败");
    });

    // 测试超时设置
    suite.add_test("超时设置", []() {
        pc::Process timeoutProc("/bin/sleep", pc::Args("sleep").add("2"));
        timeoutProc.set_timeout(100); // 100毫秒超时
        auto startTime = std::chrono::steady_clock::now();
        timeoutProc.start();
        JudgeCode result = timeoutProc.wait();
        auto endTime = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        assert_true(duration.count() < 1500, "超时计时器触发时间过长");
        assert_equal(result, JudgeCode::TimeLimitEXceeded);
    });

    return suite;
}
