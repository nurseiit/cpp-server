CXX=g++
RM=rm -f
CPPFLAGS=-I./include -std=c++11

CLIENT_SRCS=client.cpp
CLIENT_OBJS=$(subst .cpp,.o,$(CLIENT_SRCS))
SERVER_SRCS=server.cpp
SERVER_OBJS=$(subst .cpp,.o,$(SERVER_SRCS))

all: client server

client: $(CLIENT_OBJS)
	$(CXX) $(CPPFLAGS) -o client $(CLIENT_OBJS)

client.o: client.cpp

server: $(SERVER_OBJS)
	$(CXX) $(CPPFLAGS) -o server $(SERVER_OBJS)

server.o: server.cpp

clean:
	$(RM) client client.o server server.o
