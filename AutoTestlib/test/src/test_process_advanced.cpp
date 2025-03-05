#include "test_framework.h"
#include "../include/Process.h"
#include "../include/JudgeSign.h"
#include <iostream>
#include <fstream>

namespace pc = process;

TestSuite create_process_advanced_tests() {
    TestSuite suite("Process类-高级功能");

    // 测试字符读取
    suite.add_test("单字符读取", []() {
        pc::Process echoCharProc("/bin/echo", pc::Args("echo").add("ABC"));
        echoCharProc.start();
        char c = echoCharProc.getc();
        assert_equal(c, 'A');
    });

    // 测试完整的管道输入输出
    suite.add_test("排序输入输出", []() {
        pc::Process sortProc("/bin/sort", pc::Args("sort"));
        sortProc.start();
        sortProc << "c" << std::endl;
        sortProc << "a" << std::endl;
        sortProc << "b" << std::endl;
        sortProc.write(""); // 触发flush
        sortProc.close();

        std::string sortOut1 = sortProc.getline();
        std::string sortOut2 = sortProc.getline();
        std::string sortOut3 = sortProc.getline();

        assert_equal(sortOut1, std::string("a"));
        assert_equal(sortOut2, std::string("b"));
        assert_equal(sortOut3, std::string("c"));
    });

    // 测试空参数构造和延迟加载
    suite.add_test("延迟加载", []() {
        pc::Process delayedProc;
        delayedProc.load("/bin/echo", pc::Args("echo").add("延迟加载测试"));
        delayedProc.start();
        std::string delayedOutput = delayedProc.getline();
        assert_equal(delayedOutput, std::string("延迟加载测试"));
    });

    // 测试内存限制 (无超限)
    suite.add_test("无内存超限", []() {
        pc::Process memProc("/usr/bin/python3", pc::Args("python3").add("-c").add("import numpy as np; a = np.ones((100, 100))"));
        memProc.set_memout(10); // 10MB内存限制
        memProc.start();
        JudgeCode memResult = memProc.wait();
        assert_true(memResult != JudgeCode::MemoryLimitExceeded, "小内存程序不应超出内存限制");
    });

    // 测试严格内存限制
    suite.add_test("内存限制触发", []() {
        pc::Process memLimitProc("/usr/bin/python3", pc::Args("python3").add("-c").add("import numpy as np; a = np.ones((5000, 5000))"));
        memLimitProc.set_memout(1); // 1MB内存限制，应该会超出
        memLimitProc.start();
        JudgeCode memLimitResult = memLimitProc.wait();
        // 可能触发MemoryLimitExceeded或RuntimeError
        assert_true(memLimitResult == JudgeCode::MemoryLimitExceeded ||
                  memLimitResult == JudgeCode::RuntimeError,
                  "大内存程序应触发内存限制");
    });

    // 测试环境变量操作
    suite.add_test("环境变量管理", []() {
        pc::Process envManipProc("/usr/bin/env", pc::Args("env"));
        envManipProc.set_env("VAR1", "value1");
        envManipProc.set_env("VAR2", "value2");
        envManipProc.unset_env("VAR1");
        envManipProc.start();
        std::string envOutput = envManipProc.read(pc::OUT);
        assert_true(envOutput.find("VAR2=value2") != std::string::npos, "环境变量设置失败");
        assert_true(envOutput.find("VAR1=value1") == std::string::npos, "环境变量取消设置失败");
    });

    // 测试标准错误输出
    suite.add_test("标准错误读取", []() {
        pc::Process stderrProc("/bin/bash", pc::Args("bash").add("-c").add("echo 'standard output'; echo 'error output' >&2"));
        stderrProc.start();
        std::string stdOut = stderrProc.getline();
        std::string stdErr = stderrProc.read(pc::ERR);
        assert_equal(stdOut, std::string("standard output"));
        assert_true(stdErr.find("error output") != std::string::npos, "无法读取标准错误输出");
    });

    // 测试取消超时
    suite.add_test("取消超时", []() {
        pc::Process cancelTimeoutProc("/bin/sleep", pc::Args("sleep").add("0.5"));
        cancelTimeoutProc.set_timeout(1000);
        cancelTimeoutProc.cancel_timeout();
        cancelTimeoutProc.start();
        JudgeCode cancelResult = cancelTimeoutProc.wait();
        assert_equal(cancelResult, JudgeCode::Waiting, "取消超时设置失败");
    });

    // 测试取消内存限制
    suite.add_test("取消内存限制", []() {
        pc::Process cancelMemProc("/usr/bin/python3", pc::Args("python3").add("-c").add("import numpy as np; a = np.ones((1000, 1000))"));
        cancelMemProc.set_memout(1);
        cancelMemProc.cancel_memout();
        cancelMemProc.start();
        JudgeCode cancelMemResult = cancelMemProc.wait();
        assert_equal(cancelMemResult, JudgeCode::Waiting, "取消内存限制设置失败");
    });

    return suite;
}
