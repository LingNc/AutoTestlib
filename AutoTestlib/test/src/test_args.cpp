#include "test_framework.h"
#include "../include/Process.h"
#include <iostream>

namespace pc = process;

TestSuite create_args_tests() {
    TestSuite suite("Args类");

    // 测试构造函数和add方法
    suite.add_test("构造函数和add方法", []() {
        pc::Args args1("hello");
        args1.add("-a").add("-b").add("文件名");
        assert_equal(args1.get_program_name(), std::string("hello"));
        assert_equal(args1.size(), (size_t)4);
    });

    // 测试bash构造
    suite.add_test("bash构造", []() {
        pc::Args args2("bash");
        assert_equal(args2.get_program_name(), std::string("bash"));
        assert_equal(args2.size(), (size_t)1);
    });

    // 测试parse方法
    suite.add_test("parse方法", []() {
        pc::Args args3;
        args3.parse("find . -name \"*.cpp\" -type f");
        assert_equal(args3.get_program_name(), std::string("find"));
        assert_equal(args3.size(), (size_t)6);
        assert_equal(args3[1], std::string("."));
        assert_equal(args3[2], std::string("-name"));
        assert_equal(args3[3], std::string("\"*.cpp\""));
        assert_equal(args3[4], std::string("-type"));
        assert_equal(args3[5], std::string("f"));
    });

    // 测试转义字符
    suite.add_test("parse转义字符", []() {
        pc::Args args;
        args.parse("echo \"quoted \\\"string\\\"\" '单引号\\'内容' file\\ with\\ space");
        assert_equal(args[0], std::string("echo"));
        assert_equal(args[1], std::string("\"quoted \\\"string\\\"\""));
        assert_equal(args[2], std::string("'单引号\\'内容'"));
        assert_equal(args[3], std::string("file\\ with\\ space"));
    });

    // 测试常见命令行格式
    suite.add_test("常见命令行格式", []() {
        pc::Args args;
        args.parse("bash -c \"echo hello\"");
        assert_equal(args[0], std::string("bash"));
        assert_equal(args[1], std::string("-c"));
        assert_equal(args[2], std::string("\"echo hello\""));
    });

    // 测试set_program_name
    suite.add_test("set_program_name", []() {
        pc::Args args;
        args.set_program_name("test").add("arg1").add("arg2");
        assert_equal(args.get_program_name(), std::string("test"));
    });

    // 测试clear方法
    suite.add_test("clear方法", []() {
        pc::Args args;
        args.set_program_name("test").add("arg1");
        args.clear();
        assert_equal(args.size(), (size_t)0);
    });

    // 测试索引操作符
    suite.add_test("索引操作符", []() {
        pc::Args args("program");
        args.add("arg1").add("arg2");
        assert_equal(args[0], std::string("program"));
        assert_equal(args[1], std::string("arg1"));
        assert_equal(args[2], std::string("arg2"));
    });

    // 测试向量构造
    suite.add_test("向量构造", []() {
        std::vector<std::string> vec_args = {"prog", "-a", "-b", "--option"};
        pc::Args args(vec_args);
        assert_equal(args.size(), (size_t)4);
        assert_equal(args[0], std::string("prog"));
    });

    // 测试添加多个参数
    suite.add_test("添加多个参数", []() {
        pc::Args args("cmd");
        args.add(std::vector<std::string>{"-x", "-y", "-z"});
        assert_equal(args.size(), (size_t)4);
        assert_equal(args[1], std::string("-x"));
        assert_equal(args[3], std::string("-z"));
    });

    // 测试data()方法
    suite.add_test("data方法", []() {
        pc::Args args("test");
        args.add("-v").add("value");
        char** c_args = args.data();
        assert_equal(std::string(c_args[0]), std::string("test"));
        assert_equal(std::string(c_args[1]), std::string("-v"));
        assert_equal(std::string(c_args[2]), std::string("value"));
        assert_true(c_args[3] == nullptr);
    });

    return suite;
}
