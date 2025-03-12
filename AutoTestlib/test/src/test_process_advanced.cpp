#include "test_framework.h"
#include "Process.h"
#include "JudgeSign.h"
#include <iostream>
#include <fstream>

namespace pc=process;

TestSuite create_process_advanced_tests(){
    TestSuite suite("Process类-高级功能");

    // 测试字符读取
    suite.add_test("单字符读取",[]()-> std::string{
        pc::Process echoCharProc("/bin/echo",pc::Args("echo").add("ABC"));
        echoCharProc.start();
        char c=echoCharProc.getchar();
        assert_equal(c,'A');
        return "";
        });

    // 测试大量数据写入和读取
    suite.add_test("大量数据传输",[]()->std::string{
        pc::Args cat_args("cat");
        pc::Process cat("/bin/cat",cat_args);
        cat.start();
        // 生成一个大的数据块
        std::string large_data;
        for(int i=0; i<10000; ++i){
            large_data+="Line "+std::to_string(i)+" of test data\n";
        }
        // 调整缓冲区大小
        cat.set_buffer_size(pc::MB(16));
        // 计时
        auto start_time=std::chrono::high_resolution_clock::now();
        cat.write(large_data);
        auto end_time=std::chrono::high_resolution_clock::now();
        auto writeTimes=std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count();
        start_time=std::chrono::high_resolution_clock::now();
        std::string output=cat.read(pc::OUT);
        end_time=std::chrono::high_resolution_clock::now();
        auto readTimes=std::chrono::duration_cast<std::chrono::milliseconds>(end_time-start_time).count();
        cat.kill(SIGTERM);
        cat.wait();
        assert_equal(output,large_data);
        string info="写入时间: "+std::to_string(writeTimes)+"ms\n读取时间: "+std::to_string(readTimes)+"ms";
        return info;
        });

    // 测试完整的管道输入输出
    suite.add_test("排序输入输出",[]()->std::string{
        pc::Process sortProc("/bin/sort",pc::Args("sort"));
        sortProc.start();
        sortProc<<"c"<<std::endl;
        sortProc<<"a"<<std::endl;
        sortProc<<"b"<<std::endl;
        sortProc.write(""); // 触发flush
        sortProc.close();

        std::string sortOut1=sortProc.getline();
        std::string sortOut2=sortProc.getline();
        std::string sortOut3=sortProc.getline();

        assert_equal(sortOut1,std::string("a"));
        assert_equal(sortOut2,std::string("b"));
        assert_equal(sortOut3,std::string("c"));
        return "";
        });

    // 测试空参数构造和延迟加载
    suite.add_test("延迟加载",[]()->std::string{
        pc::Process delayedProc;
        delayedProc.load("/bin/echo",pc::Args("echo").add("延迟加载测试"));
        delayedProc.start();
        std::string delayedOutput=delayedProc.getline();
        assert_equal(delayedOutput,std::string("延迟加载测试"));
        return "";
        });

    // 测试内存限制 (无超限)
    suite.add_test("无内存超限",[]()->std::string{
        pc::Process memProc("/usr/bin/python3",pc::Args("python3").add("-c").add("import numpy as np; a = np.ones((100, 100))"));
        memProc.set_memout(10); // 10MB内存限制
        memProc.start();
        JudgeCode memResult=memProc.wait();
        assert_true(memResult!=JudgeCode::MemoryLimitExceeded,"小内存程序不应超出内存限制");
        return "";
        });

    // 测试严格内存限制
    suite.add_test("内存限制触发",[]()->std::string{
        pc::Process memLimitProc("/usr/bin/python3",pc::Args("python3").add("-c").add("import numpy as np; a = np.ones((5000, 5000))"));
        memLimitProc.set_memout(1); // 1MB内存限制，应该会超出
        memLimitProc.start();
        JudgeCode memLimitResult=memLimitProc.wait();
        // 可能触发MemoryLimitExceeded或RuntimeError
        assert_true(memLimitResult==JudgeCode::MemoryLimitExceeded||
            memLimitResult==JudgeCode::RuntimeError,
            "大内存程序应触发内存限制");
        return "";
        });

    // 测试自定义程序内存限制
    suite.add_test("自定义内存限制测试",[]()->std::string{
        // 创建一个简单的C程序用于测试内存限制
        std::string program=R"(
            #include <stdio.h>
            #include <stdlib.h>
            #include <string.h>

            int main() {
                // 尝试分配大量内存 (约100MB)
                char* buffer = (char*)malloc(100 * 1024 * 1024);

                if (buffer) {
                    // 使用内存以防编译器优化
                    memset(buffer, 'X', 100 * 1024 * 1024);
                    printf("Memory allocation successful\n");
                    free(buffer);
                    return 0;
                } else {
                    printf("Memory allocation failed\n");
                    return 1;
                }
            }
            )";

        // 写入文件
        std::ofstream file("memtest.c");
        file<<program;
        file.close();

        // 编译程序
        pc::Process gcc("/usr/bin/gcc",pc::Args("gcc").add("-o").add("memtest").add("memtest.c"));
        gcc.start();
        gcc.wait();

        // 运行测试程序，限制内存为50MB
        pc::Args args("memtest");
        pc::Process proc("./memtest",args);
        proc.set_memout(50); // 50MB限制
        proc.start();
        JudgeCode result=proc.wait();

        // 清理
        pc::Process rm("/bin/rm",pc::Args("rm").add("-f").add("memtest").add("memtest.c"));
        rm.start();
        rm.wait();
        assert_true(result==JudgeCode::MemoryLimitExceeded||proc.get_exit_code()!=0,
            "内存限制应该阻止程序正常执行");
        return "";
        });

    // 测试设置环境变量
    suite.add_test("环境变量设置",[]()->std::string{
        pc::Process envProc("/usr/bin/env",pc::Args("env"));
        envProc.set_env("TEST_VAR1","value1");
        envProc.set_env("TEST_VAR2","value2");
        envProc.start();
        std::string output=envProc.read(pc::OUT);
        envProc.wait();
        assert_true(output.find("TEST_VAR1=value1")!=std::string::npos,"环境变量TEST_VAR1设置失败");
        assert_true(output.find("TEST_VAR2=value2")!=std::string::npos,"环境变量TEST_VAR2设置失败");
        return "";
        });

    // 测试环境变量操作
    suite.add_test("环境变量管理",[]()->std::string{
        pc::Process envManipProc("/usr/bin/env",pc::Args("env"));
        envManipProc.set_env("VAR1","value1");
        envManipProc.set_env("VAR2","value2");
        envManipProc.unset_env("VAR1");
        envManipProc.start();
        std::string envOutput=envManipProc.read(pc::OUT);
        assert_true(envOutput.find("VAR2=value2")!=std::string::npos,"环境变量设置失败");
        assert_true(envOutput.find("VAR1=value1")==std::string::npos,"环境变量取消设置失败");
        return "";
        });

    // 测试标准错误输出
    suite.add_test("标准错误读取",[]()->std::string{
        pc::Process stderrProc("/bin/bash",pc::Args("bash").add("-c").add("echo 'standard output'; echo 'error output' >&2"));
        stderrProc.start();
        std::string stdOut=stderrProc.getline();
        std::string stdErr=stderrProc.read(pc::ERR);
        assert_equal(stdOut,std::string("standard output"));
        assert_true(stdErr.find("error output")!=std::string::npos,"无法读取标准错误输出");
        return "";
        });

    // 测试取消超时
    suite.add_test("取消超时",[]()->std::string{
        pc::Process cancelTimeoutProc("/bin/sleep",pc::Args("sleep").add("0.5"));
        cancelTimeoutProc.set_timeout(1000);
        cancelTimeoutProc.cancel_timeout();
        cancelTimeoutProc.start();
        JudgeCode cancelResult=cancelTimeoutProc.wait();
        assert_equal(cancelResult,JudgeCode::Waiting,"取消超时设置失败");
        return "";
        });

    // 测试取消内存限制
    suite.add_test("取消内存限制",[]()->std::string{
        pc::Process cancelMemProc("/usr/bin.python3",pc::Args("python3").add("-c").add("import numpy as np; a = np.ones((1000, 1000))"));
        cancelMemProc.set_memout(1);
        cancelMemProc.cancel_memout();
        cancelMemProc.start();
        JudgeCode cancelMemResult=cancelMemProc.wait();
        assert_equal(cancelMemResult,JudgeCode::Waiting,"取消内存限制设置失败");
        return "";
        });

    return suite;
}