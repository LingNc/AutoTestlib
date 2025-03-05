# AutoTestlib 测试文档

本文档描述了AutoTestlib的测试套件及其使用方法。

## 测试概述

AutoTestlib测试套件位于`test`目录下，主要测试文件是`test.cpp`。测试涵盖了库的核心组件：

- **Args类**: 命令行参数的解析与管理
- **Process类**: 进程创建、控制和通信
- **KeyCircle类**: API密钥的存储和管理
- **JudgeSign**: 判题结果代码

## 测试细节

### Args类测试

测试了Args类的以下功能：
- 构造函数和add方法
- bash构造方式
- parse方法解析命令行
- set_program_name方法
- clear方法
- 索引操作符
- 向量构造
- 添加多个参数
- data()方法生成C风格参数

### Process类测试

测试了Process类的以下功能：
- 基本执行功能
- 非阻塞模式
- 环境变量设置和管理
- 输入输出流操作
- 进程状态检测
- 进程终止
- 超时设置和取消
- 字符读取
- 管道输入/输出
- 延迟加载
- 内存限制和取消
- 标准输出和错误读取
- 阻塞模式切换
- 退出码检测
- 运行时错误检测
- 浮点错误检测

### KeyCircle类测试

测试了KeyCircle类的以下功能：
- 不存在情况的处理
- 保存和获取密钥
- 文件读取密钥
- 更新密钥
- 文件内容验证

### JudgeSign测试

测试了JudgeCode枚举的正确性，验证了各种判题结果代码：
- Waiting
- Accept
- CompilationError
- WrongAnswer
- TimeLimitEXceeded
- RuntimeError

## 运行测试

编译并运行测试：

```bash
# 在项目根目录执行
make test

# 或者手动编译运行
g++ -std=c++17 test/test.cpp -o test_autolib
./test_autolib
```

## 测试结果解读

测试使用`report_test`函数报告结果：
- ✓ 表示测试通过
- ✗ 表示测试失败

如果所有测试通过，程序将显示"所有测试通过!"并以退出码0结束。
如果任何测试失败，程序将以非零退出码结束，并指示失败的测试。

## 添加新测试

添加新测试时：
1. 为新组件创建测试函数（如`test_new_component()`）
2. 使用`report_test()`函数验证结果
3. 在`main()`函数中调用新的测试函数
4. 确保测试结束后清理所有临时资源
