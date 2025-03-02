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
SRCS = main.cpp $(SRC_DIR)/KeyCircle.cpp
OBJS = $(SRCS:.cpp=.o)

# 构建规则
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(OPENAI_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

# 创建config目录
config:
	mkdir -p config

# 清理规则
clean:
	rm -f $(TARGET) $(OBJS)

# 运行程序
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run config
