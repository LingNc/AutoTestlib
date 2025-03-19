#include "test_framework.h"
#include "JudgeSign.h"
#include <iostream>

TestSuite create_judgesign_tests() {
    TestSuite suite("JudgeSign类");

    // 测试枚举值
    suite.add_test("枚举值测试", []() -> std::string {
        assert_equal_enum(acm::JudgeCode::Waiting, 0);
        assert_equal_enum(acm::JudgeCode::Accept, 3);
        assert_equal_enum(acm::JudgeCode::CompilationError, 4);
        assert_equal_enum(acm::JudgeCode::WrongAnswer, 5);
        assert_equal_enum(acm::JudgeCode::TimeLimitEXceeded, 6);
        assert_equal_enum(acm::JudgeCode::RuntimeError, 10);
        assert_equal_enum(acm::JudgeCode::MemoryLimitExceeded, 7);
        assert_equal_enum(acm::JudgeCode::FloatingPointError, 9);
        return "";
    });

    // 可以添加更多JudgeSign相关测试...

    return suite;
}
