APP_SRCS:=$(wildcard *.cpp)
APP_SRCS += $(wildcard ../include/common/*.cpp)

PROTO_SRCS= ../proto/gameContent.pb.cc 

APP_OBJS=$(APP_SRCS:.cpp=.o)

PROTO_OBJS= ../proto/gameContent.pb.o

INCLUDES=-I./ -I../proto -I../include -I../
LIBRARY=-L../lib -lprotobuf-lite -lprotobuf -lprotoc -lpthread


all: Project echo
.PHONY: all


.cc.o:
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -c -o $@ $<

.cpp.o :
	$(CXX) $(CXXFLAGS) -fpermissive $(INCLUDES) -c -o $@ $<

Project : $(APP_OBJS) $(PROTO_OBJS)
	$(CXX) $(CXXFLAGS) $(PROTO_OBJS) $(APP_OBJS) $(INCLUDES) $(LIBRARY) -o $@

echo : 
	$(CXX) $(CXXFLAGS) $(PROTO_OBJS) ../include/common/CProtoPacket.o -o echo

clean:
	rm -f *.o ../proto/*.o Project ../include/common/*.o core*

