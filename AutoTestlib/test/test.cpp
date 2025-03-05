#include "test_framework.h"
#include <iostream>
#include <cstdlib>

// 导入各测试套件的创建函数
extern TestSuite create_args_tests();
extern TestSuite create_process_basic_tests();
extern TestSuite create_process_advanced_tests();
extern TestSuite create_process_complex_tests();
extern TestSuite create_keycircle_tests();
extern TestSuite create_judgesign_tests();

int main(int argc, char** argv) {
    std::cout << "==================================" << std::endl;
    std::cout << "   AutoTestlib 测试套件 v1.0" << std::endl;
    std::cout << "==================================" << std::endl;

    // 创建测试管理器
    TestManager manager;

    // 根据命令行参数选择要运行的测试
    bool run_all = (argc < 2);
    bool run_args = run_all || (std::string(argv[1]) == "args");
    bool run_process = run_all || (std::string(argv[1]) == "process");
    bool run_keycircle = run_all || (std::string(argv[1]) == "keycircle");
    bool run_judgesign = run_all || (std::string(argv[1]) == "judgesign");

    // 添加要运行的测试套件
    if (run_args) {
        manager.add_suite(create_args_tests());
    }

    if (run_process) {
        manager.add_suite(create_process_basic_tests());
        manager.add_suite(create_process_advanced_tests());
        manager.add_suite(create_process_complex_tests());
    }

    if (run_keycircle) {
        manager.add_suite(create_keycircle_tests());
    }

    if (run_judgesign) {
        manager.add_suite(create_judgesign_tests());
    }

    // 运行所有测试
    bool all_passed = manager.run_all();

    // 返回适当的退出码
    return all_passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
