#include "test_framework.h"
#include "Process.h"
#include <iostream>

namespace pc=process;

TestSuite create_args_tests(){
    TestSuite suite("Args类");

    // 测试构造函数和add方法
    suite.add_test("构造函数和add方法",[]() -> std::string{
        pc::Args args1("hello");
        args1.add("-a").add("-b").add("文件名");
        assert_equal(args1.get_program_name(),std::string("hello"));
        assert_equal(args1.size(),(size_t)4);
        pc::Args args2;
        args2.add("echo").add("Hello").add("World");

        assert_equal(args2.size(),(size_t)3);
        assert_equal(args2.get_program_name(),"echo");
        assert_equal(args2[1],"Hello");
        assert_equal(args2[2],"World");
        return "";
        });

    // 测试bash构造
    suite.add_test("bash构造",[]() -> std::string{
        pc::Args args2("bash");
        assert_equal(args2.get_program_name(),std::string("bash"));
        assert_equal(args2.size(),(size_t)1);
        return "";
        });

    // 测试命令行解析
    suite.add_test("命令行解析",[]() -> std::string{
        pc::Args cmd_args;
        cmd_args.parse("ls -la \"/home/user with spaces/\"");
        assert_equal(cmd_args.size(), (size_t)3);
        assert_equal(cmd_args[0], std::string("ls"));
        assert_equal(cmd_args[1], std::string("-la"));
        assert_equal(cmd_args[2], std::string("\"/home/user with spaces/\""));
        return "";
        });
        // 测试复杂命令行解析
        suite.add_test("复杂命令行解析",[]() -> std::string{
            pc::Args complex_args;
            complex_args.parse("grep \"hello world\" file.txt | sort");
            assert_equal(complex_args.size(), (size_t)5);
            assert_equal(complex_args[0], std::string("grep"));
            assert_equal(complex_args[1], std::string("\"hello world\""));
            assert_equal(complex_args[2], std::string("file.txt"));
            assert_equal(complex_args[3], std::string("|"));
            assert_equal(complex_args[4], std::string("sort"));
            return "";
            });

    // 测试parse方法
    suite.add_test("parse方法",[]() -> std::string{
        pc::Args args3;
        args3.parse("find . -name \"*.cpp\" -type f");
        assert_equal(args3.get_program_name(),std::string("find"));
        assert_equal(args3.size(),(size_t)6);
        assert_equal(args3[1],std::string("."));
        assert_equal(args3[2],std::string("-name"));
        assert_equal(args3[3],std::string("\"*.cpp\""));
        assert_equal(args3[4],std::string("-type"));
        assert_equal(args3[5],std::string("f"));
        return "";
        });
    // 测试转义字符
    suite.add_test("parse转义字符",[]() -> std::string{
        pc::Args args;
        args.parse("echo \"quoted \\\"string\\\"\" '单引号\\'内容' file\\ with\\ space");
        assert_equal(args[0],std::string("echo"));
        assert_equal(args[1],std::string("\"quoted \\\"string\\\"\""));
        assert_equal(args[2],std::string("'单引号\\'内容'"));
        assert_equal(args[3],std::string("file\\ with\\ space"));
        return "";
        });

    // 测试常见命令行格式
    suite.add_test("常见命令行格式",[]() -> std::string{
        pc::Args args;
        args.parse("bash -c \"echo hello\"");
        assert_equal(args[0],std::string("bash"));
        assert_equal(args[1],std::string("-c"));
        assert_equal(args[2],std::string("\"echo hello\""));
        return "";
        });

    // 测试set_program_name
    suite.add_test("set_program_name",[]() -> std::string{
        pc::Args args;
        args.set_program_name("test").add("arg1").add("arg2");
        assert_equal(args.get_program_name(),std::string("test"));
        return "";
        });

    // 测试clear方法
    suite.add_test("clear方法",[]() -> std::string{
        pc::Args args;
        args.set_program_name("test").add("arg1");
        args.clear();
        assert_equal(args.size(),(size_t)0);
        return "";
        });

    // 测试索引操作符
    suite.add_test("索引操作符",[]() -> std::string{
        pc::Args args("program");
        args.add("arg1").add("arg2");
        assert_equal(args[0],std::string("program"));
        assert_equal(args[1],std::string("arg1"));
        assert_equal(args[2],std::string("arg2"));
        return "";
        });

    // 测试向量构造
    suite.add_test("向量构造",[]() -> std::string{
        std::vector<std::string> vec_args={ "prog","-a","-b","--option" };
        pc::Args args(vec_args);
        assert_equal(args.size(),(size_t)4);
        assert_equal(args[0],std::string("prog"));
        return "";
        });

    // 测试添加多个参数
    suite.add_test("添加多个参数",[]() -> std::string{
        pc::Args args("cmd");
        args.add(std::vector<std::string>{"-x","-y","-z"});
        assert_equal(args.size(),(size_t)4);
        assert_equal(args[1],std::string("-x"));
        assert_equal(args[3],std::string("-z"));
        return "";
        });

    // 测试data()方法
    suite.add_test("data方法",[]() -> std::string{
        pc::Args args("test");
        args.add("-v").add("value");
        char **c_args=args.data();
        assert_equal(std::string(c_args[0]),std::string("test"));
        assert_equal(std::string(c_args[1]),std::string("-v"));
        assert_equal(std::string(c_args[2]),std::string("value"));
        assert_true(c_args[3]==nullptr);
        return "";
        });

    return suite;
}
