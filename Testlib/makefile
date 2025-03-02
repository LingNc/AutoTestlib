CXX := g++
CXXFLAGS := -std=c++17 -Ilib -ICode
LDFLAGS := -Llib -lboost_process -lpthread

TARGET := testlib_program

# Boost 源码（包含需要编译的 .hpp 文件）
BOOST_SRC_DIR := lib
BOOST_SRCS := $(wildcard $(BOOST_SRC_DIR)/*.c*)
BOOST_OBJS := $(BOOST_SRCS:.cpp=.o) $(BOOST_SRCS:.hpp=.o)

# 主程序源码
MAIN_SRCS := main.cpp
MAIN_OBJS := $(MAIN_SRCS:.cpp=.o)

# 静态库
BOOST_LIB := lib/libboost_process.a

all: $(TARGET)

$(TARGET): $(MAIN_OBJS) $(BOOST_LIB)
	$(CXX) $^ -o $@ $(LDFLAGS)

$(BOOST_LIB): $(BOOST_OBJS)
	ar rcs $@ $^

# 编译规则：.cpp 和 .hpp 均生成 .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(MAIN_OBJS) $(BOOST_OBJS) $(BOOST_LIB)

.PHONY: all clean