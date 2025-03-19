# AutoTestlib 测试文档

本文档描述了AutoTestlib的测试系统及其使用方法。

## 测试概述

AutoTestlib测试系统采用了模块化设计，位于`test`目录下。测试涵盖的核心组件包括：

- **Args类**: 命令行参数的解析与管理
- **Process类**: 进程创建、控制和通信（基础、高级和复杂场景）
- **KeyCircle类**: API密钥的存储和管理
- **JudgeSign**: 判题结果代码

## 测试架构

测试系统基于自定义的轻量级测试框架，主要组件包括：

- **TestSuite**: 测试套件类，组织一组相关的测试用例
- **TestManager**: 测试管理器，协调执行多个测试套件
- **断言函数**: 包含`assert_true`、`assert_equal`等用于验证测试结果

## 目录结构

```
test/
├── test.cpp              # 测试主程序入口
├── include/              # 测试框架头文件
│   └── test_framework.h  # 测试框架定义
├── src/                  # 各模块的测试实现
│   ├── test_args.cpp     # Args类测试
│   ├── test_process.cpp  # Process类测试（包含基础/高级/复杂场景）
│   ├── test_keycircle.cpp # KeyCircle类测试
│   └── test_judgesign.cpp # JudgeSign类测试
└── README.md             # 本文档
```

## 测试内容详解

### Args类测试
- 基本参数解析功能
- 命令行选项处理
- 特殊字符与引号处理
- 参数验证与错误处理

### Process类测试
分为三个测试套件：

1. **基础功能测试**
   - 简单命令执行
   - 标准输入/输出处理
   - 环境变量设置

2. **高级功能测试**
   - 超时控制
   - 内存限制实施
   - 工作目录设置
   - 异常处理

3. **复杂场景测试**
   - 多进程管道通信
   - 文件重定向
   - 与其他组件的集成测试

### KeyCircle类测试
- 密钥文件操作
- 密钥生成与验证
- 密钥循环逻辑

### JudgeSign测试
- 评测结果代码验证
- 状态转换逻辑

## 运行测试

### 编译和运行所有测试

```bash
make test
```

### 运行特定模块的测试

```bash
# 运行测试程序并指定要测试的模块
./bin/test args      # 只测试Args类
./bin/test process   # 只测试Process类
./bin/test keycircle # 只测试KeyCircle类
./bin/test judgesign # 只测试JudgeSign类
```

也可以通过make命令指定测试模块：

```bash
make test MODULE=args
```

## 测试结果解读

测试输出格式如下：

```
==================================
   AutoTestlib 测试套件 v1.0
==================================
开始测试...

===== 测试套件: Args测试 =====
✓ [参数解析测试] 通过! (15ms)
✓ [选项处理测试] 通过! (8ms)
...

套件总结: 8 通过, 0 失败

...

========================================
总结: 45 通过, 0 失败
✓ 所有测试通过! (耗时: 2秒)

JUDGESIGN_OK
```

- 每个测试前有标记：✓ 表示通过，✗ 表示失败
- 每个套件和总体结果都会有摘要统计
- 结束时输出`JUDGESIGN_OK`或`JUDGESIGN_FAIL`表示测试整体结果

## 扩展测试

要添加新的测试组件，需要：

1. 在`src`目录下创建新的测试文件（如`test_新组件.cpp`）
2. 实现一个创建测试套件的函数：`TestSuite create_新组件_tests()`
3. 在`test.cpp`中添加对应的函数声明和调用

测试用例模板：

```cpp
#include "test_framework.h"
#include "新组件.h"

TestSuite create_新组件_tests() {
    TestSuite suite("新组件测试");

    suite.add_test("功能1测试", []() {
        // 测试代码
        assert_true(条件, "错误消息");
    });

    // 添加更多测试...

    return suite;
}
```

## 注意事项

- 测试框架要求C++17或更高版本
- 部分测试可能需要特定的系统环境或权限
- 涉及文件操作的测试会在临时目录中执行，不会影响实际系统
