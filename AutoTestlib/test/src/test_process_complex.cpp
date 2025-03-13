#include "test_framework.h"
#include "Process.h"
#include <iostream>
#include <fstream>
#include <cstdlib>

namespace pc = process;

TestSuite create_process_complex_tests() {
    TestSuite suite("Process类-复杂场景");

    // 测试Args与Process的交互：转义引号
    suite.add_test("转义引号", []() -> std::string {
        pc::Args complexArgs;
        complexArgs.parse("bash -c \"echo \\\"Hello, World!\\\"\"");
        pc::Process complexProc("/bin/bash", complexArgs);
        complexProc.start();
        std::string complexOutput=complexProc.getline();
        std::cerr<<complexProc.geterr();
        assert_equal(complexOutput,std::string("Hello, World!"));
        return "";
    });

    // 测试Args与Process的交互：shell特殊字符
    suite.add_test("shell特殊字符", []() -> std::string {
        pc::Args shellArgs;
        shellArgs.parse("bash -c \"echo 'Path: /usr/bin with\\ space'\"");
        pc::Process shellProc("/bin/bash", shellArgs);
        shellProc.start();
        std::string shellOutput=shellProc.getline();
        std::cerr<<shellProc.geterr();
        assert_equal(shellOutput,std::string("Path: /usr/bin with space"));
        return "";
    });

    // 测试Args与Process的交互：嵌套命令
    suite.add_test("嵌套命令", []() -> std::string {
        pc::Args nestedArgs;
        nestedArgs.parse("bash -c \"bash -c \\\"echo nested command\\\"\"");
        pc::Process nestedProc("/bin/bash", nestedArgs);
        nestedProc.start();
        std::string nestedOutput=nestedProc.getline();
        std::cerr<<nestedProc.geterr();
        assert_equal(nestedOutput,std::string("nested command"));
        return "";
    });

    // 测试Args与Process的交互：复杂grep命令
    suite.add_test("复杂grep命令", []() -> std::string {
        pc::Args grepComplexArgs("grep");
        grepComplexArgs.add("-E").add("\"pattern.*test\"");
        pc::Process grepComplexProc("/bin/grep", grepComplexArgs);
        grepComplexProc.start();
        grepComplexProc << "no match here" << std::endl;
        grepComplexProc << "\"pattern test\"" << std::endl;  // 带引号的内容
        grepComplexProc << "pattern advanced test" << std::endl;
        grepComplexProc.close();
        std::string grepComplexOut1 = grepComplexProc.getline();
        std::string grepComplexOut2=grepComplexProc.getline();
        std::cerr<<grepComplexProc.geterr();
        assert_equal(grepComplexOut1,std::string("\"pattern test\""));
        assert_equal(grepComplexOut2, std::string("pattern advanced test"));
        return "";
    });

    // 测试Args与Process的交互：动态命令
    suite.add_test("动态命令", []() -> std::string {
        std::string dynamicCmd = "echo 'Dynamic: \\\"quoted\\\" content'";
        pc::Args dynamicArgs;
        dynamicArgs.parse("bash -c \"" + dynamicCmd + "\"");
        pc::Process dynamicProc("/bin/bash", dynamicArgs);
        dynamicProc.start();
        std::string dynamicOutput=dynamicProc.getline();
        std::cerr<<dynamicProc.geterr();
        assert_equal(dynamicOutput,std::string("Dynamic: \"quoted\" content"));
        return "";
    });

    // 测试Args与Process的交互：文件重定向
    suite.add_test("文件重定向", []() -> std::string {
        // 创建临时文件并写入内容
        system("echo 'redirect test' > /tmp/test_redirect.txt");

        pc::Args redirectArgs;
        redirectArgs.parse("bash -c \"cat < /tmp/test_redirect.txt\"");
        pc::Process redirectProc("/bin/bash", redirectArgs);
        redirectProc.start();
        std::string redirectOutput=redirectProc.getline();
        std::cerr<<redirectProc.geterr();
        assert_equal(redirectOutput,std::string("redirect test"));

        // 清理临时文件
        system("rm -f /tmp/test_redirect.txt");
        return "";
    });

    // 测试错误情况：不存在的命令
    suite.add_test("不存在命令处理", []() -> std::string {
        pc::Args invalidArgs("non_existent_command");
        pc::Process invalidProc("/usr/bin/non_existent_program", invalidArgs);
        bool exceptionCaught = false;
        try {
            invalidProc.start();
        } catch (const std::exception& e) {
            exceptionCaught = true;
            std::string errorMsg = e.what();
            assert_true(errorMsg.find("运行失败") != std::string::npos, "异常信息不包含预期内容");
        }
        assert_true(exceptionCaught, "未正确处理不存在的命令");
        return "";
    });

    // 测试角色流模式与阻塞模式混用
    suite.add_test("混合模式", []() -> std::string {
        pc::Process mixedProc("/bin/cat", pc::Args("cat"));
        mixedProc.start();

        // 非阻塞模式
        mixedProc.set_block(false);
        mixedProc << "line 1" << std::endl;
        std::string output1;

        // 使用empty方法等待数据可用，设置最多100ms超时
        mixedProc.set_flush(100);
        if(!mixedProc.empty()){
            output1 = mixedProc.getline();
        }
        else {
            // 如果100ms内没有数据，则失败
            assert_true(false, "非阻塞模式下超时未收到数据");
        }

        // 切换回阻塞模式
        mixedProc.set_block(true);
        mixedProc << "line 2" << std::endl;
        std::string output2 = mixedProc.getline();
        std::cerr << mixedProc.geterr();
        // 检测两种模式的输出
        assert_equal(output1, std::string("line 1"));
        assert_equal(output2, std::string("line 2"));
        return "";
    });

    return suite;
}
