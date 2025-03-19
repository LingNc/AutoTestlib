# 编译器设置
Cpp = g++
Cpp_flags = -std=c++17 -Wall -Wextra -g -D_GLIBCXX_DEBUG

# OpenAI库依赖
Openai_libs = -lcurl -pthread

# build文件夹
Object_dir = build
Main_base_dir = $(Object_dir)
Test_base_dir = $(Object_dir)/test

# ===头文件===
Include_dirs = include
Include_test_dirs = test/include
Include_obj_dir = $(Object_dir)/include
Include_test_obj_dir = $(Object_dir)/test/include

# 收集文件
Include_files = $(wildcard $(Include_dirs)/*.h)
Include_test_files = $(wildcard $(Include_test_dirs)/*.h)

# 外部库
Include_exts = ext

# ===主程序===
Main = main
Main_src = main.cpp
Main_src_dir = src
Main_obj_dir = $(Main_base_dir)/src

# 收集文件
Main_src_files = $(wildcard $(Main_src_dir)/*.cpp)
Main_obj_files = $(patsubst $(Main_src_dir)/%.cpp,$(Main_obj_dir)/%.o,$(Main_src_files))

# 构建主程序
.PHONY: all
all: $(Main)
	@echo "构建 $(Main) 成功!"

# 链接主文件
$(Main): $(Main_obj_files) $(Main_base_dir)/main.o
	@echo "正在链接 $(Main)..."
	$(Cpp) $(Cpp_flags) $^ -o $@ $(Openai_libs)

# 编译中间产物 - 添加头文件
$(Main_obj_dir)/%.o: $(Main_src_dir)/%.cpp $(Include_files) $(Include_exts)
	@echo "正在编译 $<..."
	@mkdir -p $(dir $@)
	$(Cpp) $(Cpp_flags) -I$(Include_dirs) -I$(Include_exts) -c $< -o $@

# 编译入口 - 添加头文件
$(Main_base_dir)/main.o: $(Main_src) $(Include_files) $(Include_exts)
	@echo "正在编译 $<..."
	@mkdir -p $(dir $@)
	$(Cpp) $(Cpp_flags) -I$(Include_dirs) -I$(Include_exts) -c $< -o $@

# ===测试程序===
Test = test/test
Test_src = test/test.cpp
Test_src_dir = test/src
Test_obj_dir = $(Test_base_dir)/src

# 收集文件
Test_src_files = $(wildcard $(Test_src_dir)/*.cpp)
Test_obj_files = $(patsubst $(Test_src_dir)/%.cpp,$(Test_obj_dir)/%.o,$(Test_src_files))

# 构建测试程序
.PHONY: test
test: $(Test)
	@echo "构建 $(Test) 成功!"

# 链接测试文件
$(Test): $(Main_obj_files) $(Test_obj_files) $(Test_base_dir)/test.o
	@echo "正在链接 $(Test)..."
	$(Cpp) $(Cpp_flags) -I$(Include_dirs) $^ -o $@ $(Openai_libs)

# 编译中间产物 - 添加头文件
$(Test_obj_dir)/%.o: $(Test_src_dir)/%.cpp $(Include_test_files) $(Include_files)
	@echo "正在编译 $<..."
	@mkdir -p $(dir $@)
	$(Cpp) $(Cpp_flags) -I$(Include_dirs) -I$(Include_test_dirs) -c $< -o $@

# 编译入口 - 添加头文件
$(Test_base_dir)/test.o: $(Test_src) $(Include_test_files) $(Include_files)
	@echo "正在编译 $<..."
	@mkdir -p $(dir $@)
	$(Cpp) $(Cpp_flags) -I$(Include_dirs) -I$(Include_test_dirs) -c $< -o $@

# ===汇编目标===
.PHONY: disassembly
disassembly: $(patsubst $(Main_src_dir)/%.cpp,$(Main_obj_dir)/%.S,$(Main_src_files))
	@echo "汇编代码生成完成!"

# 主函数
$(Main_obj_dir)/%.S: $(Main_src_dir)/%.cpp $(Include_files)
	@echo "正在生成汇编代码 $<..."
	@mkdir -p $(dir $@)
	$(Cpp) $(Cpp_flags) -I$(Include_dirs) -S $< -o $@

# 测试函数
$(Test_obj_dir)/%.S: $(Test_src_dir)/%.cpp $(Include_test_files) $(Include_files)
	@echo "正在生成汇编代码 $<..."
	@mkdir -p $(dir $@)
	$(Cpp) $(Cpp_flags) -I$(Include_dirs) -I$(Include_test_dirs) -S $< -o $@

# 为 main.cpp 单独添加规则
$(Main_base_dir)/main.S: $(Main_src) $(Include_files)
	@echo "正在生成汇编代码 $<..."
	@mkdir -p $(dir $@)
	$(Cpp) $(Cpp_flags) -I$(Include_dirs) -S $< -o $@

# 为 test.cpp 单独添加规则
$(Test_base_dir)/test.S: $(Test_src) $(Include_test_files) $(Include_files)
	@echo "正在生成汇编代码 $<..."
	@mkdir -p $(dir $@)
	$(Cpp) $(Cpp_flags) -I$(Include_dirs) -I$(Include_test_dirs) -S $< -o $@

# ===清理===
.PHONY: clean
clean:
	@echo "正在清理所有..."
	clean-main
	clean-test
	@echo "清理完成!"

# 清理主程序
.PHONY: clean-main
clean-main:
	@echo "正在清理 $(Main)..."
	@rm -rf $(Main_base_dir)/src $(Main_base_dir)/main.o $(Main)
	@echo "清理完成 $(Main)!"

# 清理测试程序
.PHONY: clean-test
clean-test:
	@echo "正在清理 $(Test)..."
	@rm -rf $(Test_base_dir) $(Test) $(Main_obj_files)
	@echo "清理完成 $(Test)!"

# ===运行===
# 运行主程序
.PHONY: run
run: all
	@echo "正在运行 $(Main)..."
	./$(Main)
	@echo "运行完成 $(Main)!"

# 运行测试
.PHONY: run-test
run-test: test
	@echo "正在运行 $(Test)..."
	./$(Test)
	@echo "运行完成 $(Test)!"

# 运行测试并生成覆盖率报告
.PHONY: run-test-coverage
run-test-coverage: test
	@echo "正在运行 $(Test)..."
	./$(Test)
	@echo "运行完成 $(Test)!"
	@echo "正在生成覆盖率报告..."
	gcov -o $(Object_dir) $(Test_obj_dir)/*.o
	@echo "生成覆盖率报告完成!"

# 运行测试并生成内存泄漏报告
.PHONY: run-test-valgrind
run-test-valgrind: test
	@echo "正在运行 $(Test)..."
	valgrind --leak-check=full --show-leak-kinds=all ./$(Test)
	@echo "运行完成 $(Test)!"

# ===帮助===
.PHONY: help
help:
	@echo "可用命令:"
	@echo "  make                   构建主程序"
	@echo "  make all               构建主程序"
	@echo "  make test              构建测试程序"
	@echo "  make clean             清理所有"
	@echo "  make clean-main        清理主程序"
	@echo "  make clean-test        清理测试程序"
	@echo "  make run               运行主程序"
	@echo "  make run-test          运行测试程序"
	@echo "  make run-test-coverage 运行测试并生成覆盖率报告"
	@echo "  make run-test-valgrind 运行测试并生成内存泄漏报告"
	@echo "  make help              显示帮助信息"