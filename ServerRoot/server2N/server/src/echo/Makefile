APP_SRCS:=$(wildcard *.cpp)
APP_SRCS += $(wildcard ../../include/common/*.cpp)

#APP_SRCS:= RTS_server.cpp ../include/common/CQueueManager.cpp ../include/common/CSessionManager.cpp ../include/common/CUser.cpp ../include/common/CUserPool.cpp

PROTO_SRCS= ../../proto/gameContent.pb.cc 

#APP_OBJS =./RTS_server.o ../include/common/CQueueManager.o ../include/common/CSessionManager.o ../include/common/CUser.o ../include/common/CUserPool.o

APP_OBJS=$(APP_SRCS:.cpp=.o)

PROTO_OBJS= ../../proto/gameContent.pb.o

INCLUDES=-I./ -I../../proto -I../../include -I../../
LIBRARY=-L../../lib -lprotobuf-lite -lprotobuf -lprotoc -lpthread


all: echo_exe
.PHONY: all


#$(PROTO_OBJS): $(PROTO_SRCS)

.cc.o:
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -c -o $@ $<

.cpp.o :
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -c -o $@ $<

echo_exe : $(APP_OBJS) $(PROTO_OBJS)
	$(CXX) $(CXXFLAGS) $(PROTO_OBJS) ./echo.o ../../include/common/CProtoManager.o $(INCLUDES) $(LIBRARY) -o $@ 

clean:
	rm -f *.o ../proto/*.o Project ../include/common/*.o echo_exe core*

