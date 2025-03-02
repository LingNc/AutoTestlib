# 编译器设置
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

# 目标可执行文件
TARGET = testlib

# OpenAI库依赖
OPENAI_LIBS = -lcurl -pthread

# 源文件
SRC = main.cpp

# 构建规则
all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $< $(OPENAI_LIBS)

# 创建config目录
config:
	mkdir -p config

# 清理规则
clean:
	rm -f $(TARGET)

# 运行程序
run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run config
