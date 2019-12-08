#指令编译器和选项
CC=g++
#库文件与库查找路径
DLIBS=-lmysqlclient
LDFLAGS=-L/usr/lib64/mysql
# #指定运行时的库文件路径
# RPATH=-Wl,-rpath=.
# #头文件查找路径
HEAD=-I/usr/include/mysql


#源文件
SRCS=test.cpp 

#目标文件
TARGET=$(patsubst %.cpp, %, $(SRCS))

#中间文件
OBJS=$(SRCS:.cpp=.o)

all:$(TARGET)

# %.o:%.cpp
# 	$(CC)  -o $@ -c $< 
%:%.cpp
	$(CC) -o $@ $< $(HEAD) $(LDFLAGS) $(DLIBS)

.PHONY:clean all
clean:
	rm -rf $(OBJS) $(TARGET)



