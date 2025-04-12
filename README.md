# AutoTestlib - 算法竞赛自动化测试框架

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

**AutoTestlib - 算法竞赛自动化测试框架**
© 2024 版权所有 绫袅LingNc
本程序采用 MIT 许可证。详情请见 [LICENSE](LICENSE) 文件。

AutoTestlib 是一个为算法竞赛编程设计的自动化测试框架，基于 Testlib 扩展开发，集成了 AI 辅助功能。它能够自动生成测试数据、验证输入格式、检查输出结果并进行程序对拍，大大简化了算法竞赛中的测试流程。

## 📋 功能概览

- **AI 辅助生成**: 利用 AI 自动生成测试数据生成器、验证器和检查器
- **自动化对拍**: 对比测试代码与标准解答的输出差异
- **错误样例收集**: 自动记录导致错误的测试样例
- **智能测试权重**: 根据配置的权重生成不同类型的测试样例
- **边界与特例测试**: 支持生成边界情况和特殊测试样例
- **格式验证**: 确保输入数据符合题目要求
- **结果检查**: 智能判断输出结果的正确性
- **CPH 集成**: 与 Competitive Programming Helper VSCode 扩展集成，自动添加错误样例

## ⚠️ 平台支持与限制

- **操作系统**: 目前仅支持 Linux 平台，依赖于 Linux 系统调用
- **编译器**: 需要支持 C++17 或更高版本的编译器（如 GCC 7.0+）
- **内存限制**: 对内存限制的实现依赖于 Linux 的 cgroups，如果您的系统不支持 cgroups，内存限制功能可能无法正常工作

## 🚀 快速开始

### 安装要求

- C++17 或更高版本
- Linux 环境（目前主要支持 Linux 系统调用）
- 使用兼容 OpenAI API 密钥（支持 OpenAI 或 DeepSeek API）

### 编译项目

```bash
# 编译主程序
make main

# 编译并运行测试
make test
```

### 基本用法

1. **运行主程序**

```bash
./main
```

2. **基础配置（首次使用）**

当提示是否进行基础配置时，输入 `y` 并提供以下信息：
- API 密钥（支持 OpenAI 或 DeepSeek API）
- 测试文件夹路径
- API 地址（如 https://api.openai.com/v1 或 https://api.deepseek.com/v1）
- 默认使用的 AI 模型（如 gpt-3.5-turbo、deepseek-chat 等）

3. **创建新的测试或加载现有测试**

- **创建新测试**：输入 `n` 不加载现有文件夹，然后提供：
  - 测试代码路径
  - 题目路径（如果未自动识别）
  - AC代码路径（如果未自动识别）

- **加载现有测试**：输入 `y` 加载现有文件夹，然后提供文件夹路径

4. **等待框架生成测试工具并执行对拍**

## ⚙️ 配置选项

AutoTestlib 支持多种配置选项，通过 JSON 文件进行管理。

### 全局配置文件 (`config/config.json`)

```json
{
    "allow_path": "test_path",        // 默认路径跟随策略
    "openai_url": "https://api.deepseek.com/v1",  // AI API地址
    "floder": 0,                      // 自动创建文件夹的计数
    "model": "deepseek-chat",         // 默认使用的模型
    "named_model": null,              // 命名使用的模型（可为null则使用默认）
    "model_config": {                 // 模型配置
        "temperature": 0.001,         // 温度参数(创意程度)
        "max_tokens": 4096,           // 最大生成标记数
        "top_p": 1                    // 输出概率过滤参数
    }
}
```

### 测试项目配置文件 (`[TestName]/config.json`)

```json
{
    "name": "TestName",               // 测试名称
    "attach_mode": true,              // 是否附加全局配置
    "data_num": 0,                    // 数据计数
    "now_data": 0,                    // 当前数据生成位置
    "now_test": 0,                    // 当前测试到的位置
    "special": 10,                    // 特例数量
    "edge": 5,                        // 边界测试数量
    "error_limit": 2,                 // 错误样例限制
    "time_limit": 1000,               // 时间限制(ms)
    "mem_limit": 256,                 // 内存限制(MB)
    "judge_status": "waiting",        // 判题状态
    "test_weight": false,             // 是否启用权重模式
    "weights": [10, 1, 2],            // 普通/特例/边界的权重
    "tool_choice": "auto"             // AI工具调用选择
}
```

### 可配置参数列表

以下是可以通过 `test.config()` 方法修改的主要配置参数：

| 参数 | 配置项 | 说明 |
|-----|-------|------|
| `Allow_Path` | "allow_path" | 跟随路径策略 |
| `OpenAI_URL` | "openai_url" | API地址 |
| `Model` | "model" | 默认AI模型 |
| `Named_Model` | "named_model" | 命名使用的模型 |
| `Temperature` | "temperature" | AI温度参数(创意程度) |
| `Max_Token` | "max_tokens" | 最大生成标记数 |
| `Top_P` | "top_p" | Top-P参数 |
| `TimeLimit` | "time_limit" | 程序运行时间限制(ms) |
| `MemLimit` | "mem_limit" | 程序内存限制(MB) |
| `Special` | "special" | 特例数量 |
| `Edge` | "edge" | 边界测试数量 |
| `ErrorLimit` | "error_limit" | 错误限制数量 |
| `Test_Weight` | "test_weight" | 是否启用权重测试 |

## 💻 编程接口

### 使用 AutoTest 类

```cpp
#include "AutoTest.h"

int main() {
    // 创建测试实例
    acm::AutoTest test;

    // 设置API密钥(如果需要)
    test.set_key("your-api-key");

    // 设置测试基础路径(可选)
    test.set_basePath("/path/to/test/folder");

    // 设置测试代码、题目和AC代码
    test.set_testCode("path/to/solution.cpp");
    test.set_problem("path/to/problem.md");
    test.set_ACCode("path/to/ac.cpp");

    // 初始化测试环境
    test.init();

    // 生成测试工具并开始对拍
    test.ai_gen().start();

    return 0;
}
```

### 加载现有测试项目

```cpp
#include "AutoTest.h"

int main() {
    // 创建测试实例
    acm::AutoTest test;

    // 加载已有测试项目
    test.load("path/to/TestProject");

    // 开始对拍
    test.start();

    return 0;
}
```

### 自定义配置项

```cpp
// 修改全局配置
test.config(acm::Model, "deepseek-chat", acm::Global);

// 修改测试配置
test.config(acm::TimeLimit, "2000", acm::Test); // 设置时间限制为2000ms
test.config(acm::MemLimit, "512", acm::Test);   // 设置内存限制为512MB
test.config(acm::Special, "15", acm::Test);     // 设置特例数量为15
test.config(acm::Test_Weight, "true", acm::Test); // 启用权重测试
```

## 📁 生成的测试项目结构

当使用 AutoTest 生成一个测试项目时，会创建如下结构：

```
[TestName]/                # 测试项目名称文件夹
├── problem.md             # 题目描述
├── test.cpp               # 待测试代码
├── AC.cpp                 # 标准参考代码
├── testlib.h              # Testlib库
├── config/                # 配置目录
│   ├── config.json        # 测试配置文件
│   ├── history.json       # AI对话历史记录
│   ├── WAdatas.json       # 错误样例集合
│   └── seed.txt           # 随机种子记录
├── [TestName].log         # 测试日志文件
├── generators.cpp         # 数据生成器代码
├── validators.cpp         # 数据验证器代码
├── checkers.cpp           # 数据检查器代码
├── exec/                  # 可执行文件目录
│   ├── generators         # 编译后的生成器
│   ├── validators         # 编译后的验证器
│   ├── checkers           # 编译后的检查器
│   ├── test_code          # 编译后的测试代码
│   └── ac_code            # 编译后的AC代码
├── inData/                # 输入数据文件夹
├── outData/               # 测试代码输出文件夹
└── acData/                # 参考代码输出文件夹
```

## 🔄 自动对拍流程

1. **生成测试数据**：使用 AI 生成的 `generators` 生成测试输入，支持普通/特例/边界三种模式
2. **验证输入格式**：使用 `validators` 验证生成的输入是否符合题目要求
3. **运行测试代码**：提交的代码处理输入并生成输出
4. **运行标准解答**：AC代码处理相同输入，生成标准输出
5. **检查结果**：使用 `checkers` 比较测试代码输出与标准输出
6. **记录错误样例**：如有不一致，记录到 `WAdatas.json` 并添加到 CPH 配置
7. **错误通知**：输出详细的错误信息和判题结果
8. **错误限制**：当达到设定的错误数量限制时，自动停止测试

## 📊 判题状态

支持的判题状态：
- `Accept`: 答案正确
- `WrongAnswer`: 答案错误
- `PresentationError`: 格式错误
- `RuntimeError`: 运行时错误
- `TimeLimitEXceeded`: 超时
- `MemoryLimitExceeded`: 内存超限
- `CompilationError`: 编译错误
- `Waiting`: 等待判题
- `Queuing`: 排队中
- `FloatingPointError`: 浮点错误
- `OutputLimitExceeded`: 输出超限

## 🧪 测试类型

AutoTestlib 支持三种类型的测试数据生成：

1. **普通测试**：标准随机测试数据
2. **特例测试**：针对特殊情况的测试数据
3. **边界测试**：针对边界条件的测试数据

可以通过配置 `special`、`edge` 参数设定不同类型测试的数量，或通过 `test_weight` 和 `weights` 参数设置三种类型测试的权重比例。

## 🏗️ 项目结构与实现细节

### 目录结构

```
.
├── build/                 # 构建输出目录
├── Code/                  # 示例代码目录
├── config/                # 配置文件目录
│   ├── config.json        # 全局配置文件
│   ├── docs/              # Testlib文档
│   ├── openai.key         # API密钥
│   ├── prompt/            # AI提示词模板
│   └── tools/             # 工具配置
├── ext/                   # 第三方库
│   ├── json.hpp           # JSON解析库
│   ├── loglib.hpp         # 日志库
│   ├── openai.hpp         # API客户端
│   └── testlib.h          # Testlib库
├── include/               # 头文件
│   ├── Args.h             # 命令行参数处理
│   ├── AutoConfig.h       # 配置管理
│   ├── AutoJson.h         # JSON处理
│   ├── AutoTest.h         # 自动测试核心类
│   ├── Judge.h            # 判题相关
│   ├── KeyCircle.h        # API密钥管理
│   ├── Pipe.h             # 管道通信
│   ├── Process.h          # 进程管理
│   ├── Self.h             # 通用头文件包含
│   ├── sysapi.h           # 跨平台接口
│   └── Timer.h            # 计时器
├── src/                   # 源代码
├── test/                  # 测试系统
├── main.cpp               # 主程序
├── Makefile               # 构建脚本
├── LICENSE                # GNU GPL v3 许可证
└── README.md              # 项目文档
```

### 核心类介绍

#### AutoTest 类

核心测试类，实现以下功能：
- 初始化测试环境和配置
- 与 AI 交互生成测试工具
- 执行测试数据生成、验证和检查
- 运行对拍流程
- 记录错误样例和种子

主要方法：
- `init()`: 初始化测试环境
- `set_problem()`: 设置题目
- `set_testCode()`: 设置测试代码
- `set_ACCode()`: 设置参考代码
- `ai_gen()`: 使用AI生成测试工具
- `start()`: 开始对拍
- `load()`: 加载已有测试项目
- `set_key()`: 设置API密钥
- `run()`: 运行特定测试工具
- `config()`: 设置配置项
- `generate_data()`: 生成测试数据
- `test_data()`: 测试数据
- `add_WAdatas()`: 添加错误样例
- `add_to_cph()`: 添加样例到CPH配置

#### AutoConfig 类

配置管理类，处理 JSON 配置文件：
- `set_path()`: 设置配置文件路径
- `exist()`: 检查配置是否存在
- `save()`: 保存配置
- `value()`: 获取原始 JSON 数据
- `operator[]`: 访问配置项

#### AutoJson 类

JSON 处理类，封装了项目中常用的 JSON 结构。

#### Process 类

进程管理类，用于控制子进程执行：
- `start()`: 启动进程
- `wait()`: 等待进程结束
- `kill()`: 终止进程
- `set_timeout()`: 设置超时限制
- `set_memout()`: 设置内存限制
- `set_stdin()`: 设置输入文件
- `set_stdout()`: 设置输出文件

### 文档和提示词目录

#### config/docs 目录
包含 Testlib 相关文档，用于 AI 生成器参考：

- `index.md`: Testlib 总体介绍
- `general.md`: Testlib 通用功能
- `validator.md`: 验证器编写指南
- `generator.md`: 生成器编写指南
- `checker.md`: 检查器编写指南
- `interactor.md`: 交互器编写指南

#### config/prompt 目录
包含 AI 提示模板：

- `System.md`: 系统提示词模板
- `GeneratePrompt.md`: 数据生成器的提示模板
- `ValidatePrompt.md`: 数据验证器的提示模板
- `CheckPrompt.md`: 数据检查器的提示模板
- `GetName.md`: 自动命名功能的提示模板

## 🧪 测试系统

项目包含完整的单元测试系统，位于 test 目录下：

- **Args类测试**: 命令行参数解析
- **Process类测试**: 进程管理、输入输出处理、限制控制
- **KeyCircle类测试**: API密钥管理
- **JudgeSign类测试**: 评测结果处理
- **Pipe类测试**: 进程间通信

运行测试：
```bash
# 运行所有测试
make test

# 运行特定模块测试
make test MODULE=process
```

## 🔗 CPH 集成

AutoTestlib 支持与 Competitive Programming Helper (CPH) VSCode 扩展集成，能够自动将发现的错误样例添加到 CPH 配置中，方便后续调试。系统会自动查找与测试代码相关的 CPH 配置文件，无需手动指定。

## 📝 日志系统

AutoTestlib 使用 `loglib.hpp` 库进行日志记录：
- `log()`: 记录日志
- `tlog()`: 记录带时间戳的日志
- `set_logName()`: 设置日志文件名
- `set_logPath()`: 设置日志路径

## 📄 许可证

AutoTestlib 使用 [MIT 许可证](https://opensource.org/licenses/MIT)，这是一个宽松的开源许可证，允许任何人以任何目的使用、修改、分发本软件，只要在副本中包含原始版权声明和许可声明即可。

## 🔄 第三方代码

- **Testlib**: 包含 Mike Mirzayanov 的 Testlib 库（[原仓库](https://github.com/MikeMirzayanov/testlib)），用于生成测试数据、验证输入格式和检查输出结果。Testlib 遵循其自带的宽松许可证，与本项目的 MIT 许可证兼容。
- **nlohmann/json**: 用于解析和生成 JSON 数据的 C++ 库（[原仓库](https://github.com/nlohmann/json)），使用 MIT 许可证。
- **loglib**: 日志库，用于记录程序运行过程中的日志信息，自主开发。
- **openai.hpp**: OpenAI API 客户端，支持与 OpenAI 和 DeepSeek API 进行通信，自主开发。

## 📝 待实现功能

- [ ] **跨平台支持**: 计划实现对 Windows 的支持
- [ ] **图形用户界面**: 开发基于 Web 的图形用户界面，能够更方便地使用该框架
- [ ] **交互题支持**: 完善对竞赛中交互题型的支持
- [ ] **并行测试**: 实现并行测试多个样例以提高效率
- [ ] **Docker 支持**: 提供 Docker 容器支持，简化环境配置

## 📢 贡献与联系

如需报告问题或贡献代码，请通过代码仓库的问题跟踪系统提交您的意见和建议。