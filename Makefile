CXX=g++
RM=rm -f
CPPFLAGS=-I./include -std=c++11

SRCS=client.cpp
OBJS=$(subst .cpp,.o,$(SRCS))

all: client

client: $(OBJS)
	$(CXX) $(CPPFLAGS) -o client $(OBJS)

client.o: client.cpp

clean:
	$(RM) client client.o
