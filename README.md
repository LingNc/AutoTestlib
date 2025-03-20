# AutoTestlib

AutoTestlib 是一个为算法竞赛编程设计的自动化测试框架，基于 Testlib 扩展开发，集成了 AI 辅助功能。它能够自动生成测试数据、验证输入格式、检查输出结果并实现程序对拍，大大简化了算法竞赛中的测试流程。

## 目录结构

```
.
├── .cph/                  # 竞赛助手(Competitive Programming Helper)配置
├── .vscode/               # VSCode配置文件
├── build/                 # 构建输出目录
├── bin/                   # 编译后的二进制文件
├── Code/                  # 示例代码
├── config/                # 配置文件目录
│   ├── config.json        # 全局配置文件
│   ├── docs/              # Testlib文档
│   │   ├── checker.md     # 检查器文档
│   │   ├── general.md     # 通用功能文档
│   │   ├── generator.md   # 生成器文档
│   │   ├── index.md       # 总体介绍
│   │   ├── interactor.md  # 交互器文档
│   │   └── validator.md   # 验证器文档
│   ├── openai.key         # OpenAI API密钥
│   └── prompt/            # AI提示词模板
│       ├── CheckPrompt.md     # 检查器提示词
│       ├── GeneratePrompt.md  # 生成器提示词
│       ├── System.md          # 系统提示词
│       └── ValidatePrompt.md  # 验证器提示词
├── ext/                   # 第三方库
│   ├── json.hpp           # JSON解析库
│   ├── loglib.hpp         # 日志库
│   ├── openai.hpp         # OpenAI API客户端
│   └── testlib.h          # Testlib库
├── include/               # 头文件
│   ├── Args.h             # 命令行参数处理
│   ├── AutoConfig.h       # 配置管理
│   ├── AutoTest.h         # 自动测试核心类
│   ├── Judge.h            # 判题相关
│   ├── KeyCircle.h        # API密钥管理
│   ├── Pipe.h             # 管道通信
│   ├── Process.h          # 进程管理
│   ├── Self.h             # 通用头文件包含
│   ├── sysapi.h           # 跨平台接口(暂未完成)
│   └── Timer.h            # 计时器
├── src/                   # 源代码
│   ├── Args.cpp           # 命令行参数处理实现
│   ├── AutoConfig.cpp     # 配置管理实现
│   ├── AutoTest.cpp       # 自动测试实现
│   ├── Judge.cpp          # 判题实现
│   ├── KeyCircle.cpp      # API密钥管理实现
│   ├── Pipe.cpp           # 管道通信实现
│   ├── sysapi.cpp         # 跨平台api实现(暂未完成)
│   ├── Process.cpp        # 进程管理实现
│   └── Timer.cpp          # 计时器实现
├── test/                  # 测试系统
│   ├── include/           # 测试框架头文件
│   │   └── test_framework.h  # 测试框架定义
│   ├── src/               # 测试实现
│   │   ├── test_args.cpp     # Args类测试
│   │   ├── test_keycircle.cpp # KeyCircle类测试
│   │   ├── test_judgesign.cpp # JudgeSign类测试
│   │   ├── test_pipe.cpp      # Pipe类测试
│   │   └── test_process.cpp   # Process类测试
│   ├── README.md          # 测试说明文档
│   └── test.cpp           # 测试主程序
├── main.cpp               # 主程序
├── Makefile               # 构建脚本
└── README.md              # 项目文档
```

## AutoTest 目标文件夹结构

当使用 AutoTest 生成一个测试项目时，会创建如下结构的文件夹：

```
[TestName]/                # 测试项目名称文件夹
├── problem.md             # 题目描述
├── test.cpp               # 待测试代码
├── AC.cpp                 # 标准参考代码
├── config.json            # 测试配置文件
├── history.json           # AI对话历史记录
├── openai.key             # OpenAI API密钥(可选)
├── [TestName].log         # 测试日志文件
├── WAdatas.json           # 错误样例集合
├── generators.cpp         # 数据生成器代码
├── validators.cpp         # 数据验证器代码
├── checkers.cpp           # 数据检查器代码
├── generators             # 编译后的生成器可执行文件
├── validators             # 编译后的验证器可执行文件
├── checkers               # 编译后的检查器可执行文件
├── inData/                # 输入数据文件夹
│   ├── data1.in
│   └── ...
├── outData/               # 测试代码输出文件夹
│   ├── data1.out
│   └── ...
└── acData/                # 参考代码输出文件夹
    ├── data1.out
    └── ...
```

## 配置文件结构

### 全局配置文件 (`config/config.json`)

```json
{
    "allow_path": "test_path",        // 默认路径跟随策略
    "openai_url": "https://api.openai.com/v1",  // OpenAI API地址
    "floder": 0,                      // 自动创建文件夹的计数
    "model": "gpt-3.5-turbo",         // 默认使用的模型
    "named_model": "gpt-4",           // 命名使用的模型
    "model_config": {                 // 模型配置
        "temperature": 0.7,           // 温度参数(创意程度)
        "max_tokens": 4096,           // 最大生成标记数
        "top_p": 1                    // 输出概率过滤参数
    },
    "tools": [                        // AI可用工具列表
        {
            "type": "function",
            "function": {
                "name": "get_docs",   // 获取文档的函数
                "description": "获得Testlib函数库的参考文档...",
                "parameters": {
                    "type": "object",
                    "properties": {
                        "DocsName": {
                            "type": "string",
                            "description": "文档的名称，有五个参数候选项..."
                        }
                    },
                    "required": ["DocsName"]
                }
            }
        }
    ]
}
```

### 测试项目配置文件 (`[TestName]/config.json`)

```json
{
    "name": "TestName",               // 测试名称
    "attach_mode": true,              // 是否附加全局配置
    "data_num": 0,                    // 数据计数
    "now_data": "",                   // 当前数据文件名
    "time_limit": 1000,               // 时间限制(ms)
    "mem_limit": 256,                 // 内存限制(MB)
    "judge_status": "waiting"         // 判题状态
}
```

### 错误样例集合 (`[TestName]/WAdatas.json`)

```json
[
    {
        "in": "输入数据1",
        "out": "期望输出1"
    },
    {
        "in": "输入数据2",
        "out": "期望输出2"
    }
]
```

### AI对话历史记录 (`[TestName]/history.json`)

```json
[
    {
        "role": "system",
        "content": "系统提示词内容"
    },
    {
        "role": "user",
        "content": "完整题面为: ..."
    },
    {
        "role": "user",
        "content": "AC代码为: ..."
    },
    {
        "role": "assistant",
        "content": "AI回复内容"
    }
]
```

## ConfigSign 枚举及映射

`ConfigSign` 枚举类型在 AutoConfig.h 中定义，通过 `f()` 函数映射为字符串：

| 枚举值 | 字符串 | 描述 |
|-------|------|------|
| `Global` | - | 全局配置文件 |
| `Test` | - | 测试配置文件 |
| `Allow_Path` | "allow_path" | 跟随路径策略 |
| `AC_Path` | "ac_path" | AC代码路径 |
| `Test_Path` | "test_path" | 测试代码路径 |
| `Problem_Path` | "problem_path" | 题目路径 |
| `OpenAI_URL` | "openai_url" | OpenAI API地址 |
| `Test_Name` | "name" | 测试文件名称 |
| `Floder_Number` | "floder" | 自动创建文件夹计数 |
| `Attach_Global` | "attach_mode" | 附加模式 |
| `Model` | "model" | 默认模型 |
| `Named_Model` | "named_model" | 命名模型 |
| `Model_Config` | "model_config" | 模型配置 |
| `Prompt` | "prompt" | 提示词 |
| `Temperature` | "temperature" | 温度参数 |
| `Max_Token` | "max_tokens" | 最大令牌数 |
| `Top_P` | "top_p" | Top-P参数 |
| `Tools` | "tools" | 工具列表 |
| `Generators` | "generators" | 生成器 |
| `Validators` | "validators" | 验证器 |
| `Checkers` | "checkers" | 检查器 |
| `Interactors` | "interactors" | 交互器 |
| `System` | "system" | 系统 |
| `DataNum` | "data_num" | 数据计数 |
| `NowData` | "now_data" | 当前数据文件 |
| `AC_Code` | "ac_code" | AC代码 |
| `Test_Code` | "test_code" | 测试代码 |
| `TimeLimit` | "time_limit" | 时间限制 |
| `MemLimit` | "mem_limit" | 内存限制 |
| `JudgeStatus` | "judge_status" | 判题状态 |

## config/docs 目录

文档文件夹包含 Testlib 相关文档：

- index.md: Testlib 总体介绍
- general.md: Testlib 通用功能、结果代码和流对象
- validator.md: 验证器编写指南和示例
- generator.md: 生成器编写指南和示例
- checker.md: 检查器编写指南和示例
- interactor.md: 交互器编写指南和示例
- Testlib_Total.md: Testlib 总体介绍原文
- Testlib_Generators.md: 生成器指南原文
- Testlib_Validators.md: 验证器指南原文
- Testlib_Checkers.md: 检查器指南原文
- Testlib_Interactors.md: 交互器指南原文

## config/prompt 目录

包含 AI 提示模板：

- `System.md`: 系统提示词模板
- `GeneratePrompt.md`: 数据生成器的提示模板
- `ValidatePrompt.md`: 数据验证器的提示模板
- `CheckPrompt.md`: 数据检查器的提示模板
- 命名提示: 用于根据题目自动生成合适的名称，其内容为 `"你是一个自动命名器,请根据题面生成一个合适的题目名称。请你的回复JSON格式为:{\"name\":\"题目名称\"}。题面："`

## 核心类介绍

### AutoTest

核心测试类，实现以下功能：
- 初始化测试环境和配置
- 与 AI 交互生成测试工具
- 执行测试数据生成、验证和检查
- 运行对拍流程
- 记录错误样例

主要方法：
- `init()`: 初始化测试环境
- `set_problem()`: 设置题目
- `set_testCode()`: 设置测试代码
- `set_ACCode()`: 设置参考代码
- `gen()`: 生成测试工具
- `start()`: 开始对拍
- `load()`: 加载已有测试项目
- `set_key()`: 设置API密钥
- `run()`: 运行特定测试工具
- `add_tool()`: 添加AI工具
- `config()`: 设置配置项
- `get_history()`: 获取对话历史
- `set_name()`: 设置测试名称
- `set_cph()`: 设置CPH路径
- `set_basePath()`: 设置基础路径

### AutoConfig

配置管理类，处理 JSON 配置文件：
- `set_path()`: 设置配置文件路径
- `exist()`: 检查配置是否存在
- `save()`: 保存配置
- `get()`: 获取原始 JSON 数据
- `operator[]`: 访问配置项

### Process

进程管理类，用于控制子进程执行：
- `start()`: 启动进程
- `wait()`: 等待进程结束
- `kill()`: 终止进程
- `set_timeout()`: 设置超时限制
- `set_memout()`: 设置内存限制
- `write()`: 向进程写入数据
- `read()`, `getline()`, `read_line()`: 从进程读取数据
- `set_block()`: 设置阻塞/非阻塞模式
- `set_flush()`: 设置刷新超时
- `set_env()`: 设置环境变量
- `empty()`: 检查流是否为空
- `is_running()`: 检查进程是否运行
- `get_status()`: 获取进程状态
- `get_exit_code()`: 获取退出码

### KeyCircle

API 密钥管理类：
- `get()`: 获取密钥
- `save()`: 保存密钥
- `exist()`: 检查密钥是否存在
- `set_path()`: 设置密钥文件路径

### Judge

判题结果管理：
- `f()`: 将判题状态转换为字符串
- `judge()`: 判断进程状态和返回码

### Log 类 (位于 ext/loglib.hpp)

日志记录类：
- `log()`: 记录日志
- `tlog()`: 记录带时间戳的日志
- `set_logName()`: 设置日志文件名
- `set_logPath()`: 设置日志路径

## 判题状态

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

## 测试系统

测试系统位于 test 目录下，已有详细的 README.md，用于测试库的各个组件：

- **Args类测试**
  - 基本参数解析功能
  - 命令行选项处理
  - 特殊字符与引号处理
  - 参数验证与错误处理

- **Process类测试**（分为基础/高级/复杂三个测试套件）
  - 简单命令执行
  - 标准输入/输出处理
  - 环境变量设置
  - 超时控制和内存限制
  - 多进程管道通信

- **KeyCircle类测试**
  - 密钥文件操作
  - 密钥生成与验证
  - 密钥循环逻辑

- **JudgeSign类测试**
  - 评测结果代码验证
  - 状态转换逻辑

- **Pipe类测试**
  - 管道通信测试
  - 进程间数据传输

## 使用示例

### 创建新的测试项目

```cpp
#include "AutoTest.h"

int main() {
    // 创建测试实例
    acm::AutoTest test("两数之和");

    // 设置题目、测试代码和AC代码
    test.set_problem("path/to/problem.md");
    test.set_testCode("path/to/solution.cpp");
    test.set_ACCode("path/to/ac.cpp");

    // 初始化测试环境
    test.init();

    // 生成测试工具
    test.gen().run();

    // 开始对拍
    test.start();

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

## 自动对拍流程

1. **生成测试数据**：使用 AI 生成的 `generators` 生成测试输入
2. **验证输入格式**：使用 `validators` 验证生成的输入是否符合题目要求
3. **运行测试代码**：提交的代码处理输入并生成输出
4. **运行标准解答**：AC代码处理相同输入，生成标准输出
5. **检查结果**：使用 `checkers` 比较测试代码输出与标准输出
6. **记录错误样例**：如有不一致，记录到 `WAdatas.json`
7. **错误通知**：输出详细的错误信息和判题结果

## CPH集成

自动测试框架支持与 Competitive Programming Helper (CPH) VSCode插件集成，能够自动将发现的错误样例添加到 CPH 配置中，方便后续调试。

## 构建与运行

使用 Makefile 构建项目：

```bash
# 构建主程序
make

# 构建并运行测试
make test

# 运行特定模块测试
make test MODULE=args
make test MODULE=process
make test MODULE=keycircle
make test MODULE=judgesign
make test MODULE=pipe
```

## 环境要求

- C++17或更高版本
- Linux环境（目前主要支持Linux系统调用）
- 用于AI功能的有效OpenAI API密钥

## 工具函数

- `rfile()`: 读取文件内容
- `wfile()`: 写入文件内容
- `chat()`: 调用OpenAI API进行对话
- `handle_function()`: 处理AI工具调用
- `f()`: 配置项和判题结果的字符串映射
- `get_docs()`: 获取文档内容
- `get_problem_name()`: 获取题目名称
- `check_func_call()`: 验证函数调用参数

通过这个自动化测试框架，用户可以更高效地进行算法题目的测试与对拍，特别适合算法竞赛选手和教练使用。主要优势在于集成了AI辅助生成测试工具，自动化了测试数据生成和验证流程，使竞赛准备更加高效。