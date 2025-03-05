# 编译器设置
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -D_GLIBCXX_DEBUG

# 目标可执行文件
TARGET = main
TEST_TARGET = test/test

# OpenAI库依赖
OPENAI_LIBS = -lcurl -pthread

# 源文件和目录
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
TEST_DIR = test

# 自动收集所有源文件和头文件
SRCS = main.cpp $(wildcard $(SRC_DIR)/*.cpp)
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)
OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# 测试相关
TEST_MAIN = $(TEST_DIR)/main_test.cpp
TEST_SRCS = $(filter-out $(TEST_DIR)/test.cpp, $(wildcard $(TEST_DIR)/*.cpp))
TEST_OBJS = $(patsubst $(TEST_DIR)/%.cpp,$(BUILD_DIR)/$(TEST_DIR)/%.o,$(TEST_MAIN) $(TEST_SRCS))
LIB_OBJS = $(filter-out $(BUILD_DIR)/main.o, $(OBJS))

# 构建规则
all: $(BUILD_DIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(OPENAI_LIBS)

$(BUILD_DIR)/%.o: %.cpp $(HEADERS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

$(BUILD_DIR)/$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp $(HEADERS) $(TEST_DIR)/test_framework.h
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# 创建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/$(SRC_DIR)
	mkdir -p $(BUILD_DIR)/$(TEST_DIR)

# 创建config目录
config:
	mkdir -p config

# 清理规则
clean:
	rm -f $(TARGET) $(TEST_TARGET)
	rm -rf $(BUILD_DIR)

# 运行程序
run: $(TARGET)
	./$(TARGET)

# 确保test目录存在
$(TEST_DIR):
	mkdir -p $(TEST_DIR)

# 编译测试程序
$(TEST_TARGET): $(TEST_DIR) $(LIB_OBJS) $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $(LIB_OBJS) $(TEST_OBJS) $(OPENAI_LIBS)

# 测试目标 - 可以使用参数执行部分测试
test: $(TEST_TARGET)
	./$(TEST_TARGET) $(ARGS)

# 运行特定测试
test-args: $(TEST_TARGET)
	./$(TEST_TARGET) args

test-process: $(TEST_TARGET)
	./$(TEST_TARGET) process

test-keycircle: $(TEST_TARGET)
	./$(TEST_TARGET) keycircle

test-judgesign: $(TEST_TARGET)
	./$(TEST_TARGET) judgesign

.PHONY: all clean run config test test-args test-process test-keycircle test-judgesign
