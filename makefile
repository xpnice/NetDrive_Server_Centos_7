#ָ���������ѡ��
CC=g++
#���ļ�������·��
DLIBS=-lmysqlclient
LDFLAGS=-L/usr/lib64/mysql
# #ָ������ʱ�Ŀ��ļ�·��
# RPATH=-Wl,-rpath=.
# #ͷ�ļ�����·��
HEAD=-I/usr/include/mysql


#Դ�ļ�
SRCS=test.cpp 

#Ŀ���ļ�
TARGET=$(patsubst %.cpp, %, $(SRCS))

#�м��ļ�
OBJS=$(SRCS:.cpp=.o)

all:$(TARGET)

# %.o:%.cpp
# 	$(CC)  -o $@ -c $< 
%:%.cpp
	$(CC) -o $@ $< $(HEAD) $(LDFLAGS) $(DLIBS)

.PHONY:clean all
clean:
	rm -rf $(OBJS) $(TARGET)



