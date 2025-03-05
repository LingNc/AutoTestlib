# AutoTestlib 测试文档

本文档描述了AutoTestlib的测试套件及其使用方法。

## 测试概述

AutoTestlib测试套件位于`test`目录下，采用模块化的测试框架设计。测试涵盖了库的核心组件：

- **Args类**: 命令行参数的解析与管理
- **Process类**: 进程创建、控制和通信
- **KeyCircle类**: API密钥的存储和管理
- **JudgeSign**: 判题结果代码

## 测试框架

测试套件使用自定义的简单测试框架，分为以下组件：

- **TestSuite**: 测试套件类，包含一组相关测试
- **TestManager**: 测试管理器，协调多个测试套件的执行
- **assert_xxx**: 断言宏，用于测试验证

## 测试文件结构

```
test/
├── main_test.cpp        # 测试入口文件
├── test_framework.h     # 测试框架头文件
├── test_args.cpp        # Args类测试
├── test_process_basic.cpp    # Process类基础功能测试
├── test_process_advanced.cpp # Process类高级功能测试
├── test_process_complex.cpp  # Process类复杂场景测试
├── test_keycircle.cpp   # KeyCircle类测试
└── test_judgesign.cpp   # JudgeSign类测试
```

## 测试细节

### Args类测试
- 构造函数和方法
- 命令行解析
- 引号和转义字符处理
- 参数管理功能

### Process类测试
分为三个部分：
1. **基础功能**：执行、环境变量、管道等
2. **高级功能**：超时、内存限制、标准错误等
3. **复杂场景**：与Args结合使用、嵌套命令、文件重定向等

### KeyCircle类测试
- 文件存在性检查
- 密钥保存与获取
- 密钥更新

### JudgeSign测试
- 枚举值验证

## 运行测试

编译并运行所有测试：

```bash
# 在项目根目录执行
make test
```

运行特定模块的测试：

```bash
# 只测试Args类
make test-args

# 只测试Process类
make test-process

# 只测试KeyCircle类
make test-keycircle

# 只测试JudgeSign类
make test-judgesign
```

也可以在运行时指定测试模块：

```bash
make test ARGS=args
```

## 测试结果解读

每个测试的结果将显示为：
- ✓ [测试名称] 通过! (耗时)
- ✗ [测试名称] 失败: 错误信息

测试套件执行完毕后会显示套件摘要，所有套件执行完毕后会显示总结果。

## 添加新测试

1. 为新组件创建测试文件 `test_新组件.cpp`
2. 在文件中定义 `create_新组件_tests()` 函数
3. 在 `main_test.cpp` 中添加对应的 extern 声明和调用
4. 可选：在 Makefile 中添加相应的测试目标

## 配置

测试框架不依赖外部库，使用标准C++17功能。确保编译环境支持C++17标准。

## 常见问题

- **测试失败但程序正常**：检查测试环境和预期条件
- **找不到程序或命令**：确保测试中使用的所有命令在系统路径中可用
- **内存测试不稳定**：内存限制测试可能依赖于系统状态，偶尔可能不稳定
