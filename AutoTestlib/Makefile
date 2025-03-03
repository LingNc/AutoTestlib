# 编译器设置
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -g -D_GLIBCXX_DEBUG

# 目标可执行文件
TARGET = main

# OpenAI库依赖
OPENAI_LIBS = -lcurl -pthread

# 源文件和目录
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# 自动收集所有源文件和头文件
SRCS = main.cpp $(wildcard $(SRC_DIR)/*.cpp)
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)
OBJS = $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(SRCS))

# 构建规则
all: $(BUILD_DIR) $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(OPENAI_LIBS)

$(BUILD_DIR)/%.o: %.cpp $(HEADERS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# 创建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)
	mkdir -p $(BUILD_DIR)/$(SRC_DIR)

# 创建config目录
config:
	mkdir -p config

# 清理规则
clean:
	rm -f $(TARGET)
	rm -rf $(BUILD_DIR)

# 运行程序
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run config
