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
extern TestSuite create_pipe_tests();  // 添加Pipe测试套件

int main(int argc, char** argv) {
    std::cout << "==================================" << std::endl;
    std::cout << "   AutoTestlib 测试套件 v1.0" << std::endl;
    std::cout << "==================================" << std::endl;

    // 创建测试管理器
    TestManager manager;
    // 设置最大参数数量
    size_t max_args=4;

    // 转换参数表
    std::vector<std::string> args(max_args,"");
    if(argc>4){
        std::cerr << "参数过多，最多支持"+std::to_string(max_args)+"个参数" << std::endl;
        return EXIT_FAILURE;
    }

    for(int i=0; i<argc; ++i){
        args[i]=argv[i];
    }

    // 根据命令行参数选择要运行的测试
    bool run_all = (argc < 2);
    bool run_args=(args[1]=="args")||run_all;
    bool run_process=(args[1]=="process")||run_all;
    bool run_keycircle=(args[1]=="keycircle")||run_all;
    bool run_judgesign=(args[1]=="judgesign")||run_all;
    bool run_pipe=(args[1]=="pipe")||run_all;

    // 添加要运行的测试套件
    if (run_args) {
        manager.add_suite(create_args_tests());
    }

    if(run_process){
        bool run_all_process=(argc<3)||run_all||(args[2]=="all");
        bool run_basic=(args[2]=="basic");
        bool run_advanced=(args[2]=="advanced");
        bool run_complex=(args[2]=="complex");

        if(run_basic||run_all_process){
            manager.add_suite(create_process_basic_tests());
        }
        if(run_advanced||run_all_process){
            manager.add_suite(create_process_advanced_tests());
        }
        if(run_complex||run_all_process){
            manager.add_suite(create_process_complex_tests());
        }
    }

    if (run_keycircle) {
        manager.add_suite(create_keycircle_tests());
    }

    if (run_judgesign) {
        manager.add_suite(create_judgesign_tests());
    }

    if (run_pipe) {
        manager.add_suite(create_pipe_tests());  // 添加Pipe测试套件
    }

    // 运行所有测试
    bool all_passed = manager.run_all();

    // 返回适当的退出码
    return all_passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
