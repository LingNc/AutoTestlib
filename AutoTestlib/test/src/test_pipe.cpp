#include "test_framework.h"
#include "Pipe.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace pc = process;

TestSuite create_pipe_tests() {
    TestSuite suite("Pipe类测试");

    // 测试基本创建和关闭
    suite.add_test("创建和关闭", []() {
        pc::Pipe pipe;
        assert_true(!pipe.is_closed(), "管道创建后不应该处于关闭状态");

        pipe.close();
        assert_true(pipe.is_closed(), "管道关闭后应该处于关闭状态");
    });

    // 测试读写基本功能
    suite.add_test("基本读写", []() {
        pc::Pipe pipe;

        // 设置为写入模式
        pipe.set_type(pc::PIPE_WRITE);

        // 写入数据
        const char* testData = "hello";
        int bytesWritten = pipe.write(testData, strlen(testData));
        assert_true(bytesWritten > 0, "写入数据失败");

        // 设置为读取模式
        pipe.set_type(pc::PIPE_READ);

        // 读取数据
        char buffer[10] = {0};
        int bytesRead = pipe.read(buffer, sizeof(buffer) - 1);
        assert_true(bytesRead > 0, "读取数据失败");
        assert_equal(std::string(buffer), std::string("hello"), "读取内容不匹配");
    });

    // 测试阻塞和非阻塞模式
    suite.add_test("阻塞和非阻塞模式", []() {
        pc::Pipe pipe;

        // 默认应该是阻塞模式
        assert_true(pipe.is_blocked(), "默认应该是阻塞模式");

        // 设置为非阻塞模式
        pipe.set_blocked(false);
        assert_true(!pipe.is_blocked(), "应该是非阻塞模式");

        // 设置回阻塞模式
        pipe.set_blocked(true);
        assert_true(pipe.is_blocked(), "应该是阻塞模式");
    });

    // 测试empty函数
    suite.add_test("测试empty", []() {
        pc::Pipe pipe;

        // 新创建的管道应该是空的
        assert_true(pipe.empty(), "新创建的管道应该是空的");

        // 写入一些数据
        pipe.set_type(pc::PIPE_WRITE);
        pipe.write("test", 4);

        // 切换到读取模式
        pipe.set_type(pc::PIPE_READ);
        assert_true(!pipe.empty(), "写入数据后管道不应该为空");

        // 读取数据
        char buffer[5] = {0};
        pipe.read(buffer, 4);

        // 读取后应该是空的
        assert_true(pipe.empty(), "读取所有数据后管道应该为空");
    });

    // 测试句柄操作
    suite.add_test("句柄操作", []() {
        pc::Pipe pipe;

        // 应该能获取有效的句柄
        process::Handle handle = pipe.get_handle();
        assert_true(handle != -1, "应该能获取有效的句柄");

        // 数组访问方式
        process::Handle handle0 = pipe[0];
        process::Handle handle1 = pipe[1];
        assert_true(handle0 != -1 && handle1 != -1, "应该能通过数组方式获取有效句柄");
    });

    // 测试高级读写功能
    suite.add_test("高级读写功能", []() {
        pc::Pipe pipe;
        pipe.set_type(pc::PIPE_WRITE);

        // 写入字符串
        std::string testStr = "line1\nline2\nline3";
        pipe.write(testStr);

        // 切换到读取模式
        pipe.set_type(pc::PIPE_READ);

        // 读取单个字符
        char c = pipe.read_char();
        assert_equal(c, 'l', "读取单个字符失败");

        // 读取一行
        std::string line = pipe.read_line();
        assert_equal(line, "ine1", "读取一行失败");

        // 读取下一行
        line = pipe.read_line();
        assert_equal(line, "line2", "读取下一行失败");

        // 读取剩余所有内容
        std::string remaining = pipe.read_all();
        assert_true(remaining.find("line3") != std::string::npos, "读取所有内容失败");
    });

    // 测试缓冲区大小设置
    suite.add_test("缓冲区大小设置", []() {
        pc::Pipe pipe;

        // 设置为较大的缓冲区
        int bigSize = 8192;
        pipe.set_buffer_size(bigSize);

        // 生成大量数据
        std::string largeData(5000, 'X');

        // 写入数据
        pipe.set_type(pc::PIPE_WRITE);
        pipe.write(largeData);

        // 读取数据
        pipe.set_type(pc::PIPE_READ);
        std::string readData = pipe.read_all();

        assert_equal(readData.size(), largeData.size(), "大缓冲区数据读取不完整");
    });

    // 测试非阻塞读取超时
    suite.add_test("非阻塞读取超时", []() {
        pc::Pipe pipe;
        pipe.set_blocked(false);

        // 尝试从空管道读取数据，应该立即返回
        std::string result = pipe.read_line(100); // 设置100ms超时
        assert_true(result.empty(), "从空管道的非阻塞读取应返回空字符串");
    });

    return suite;
}
