
APP_SRCS=$(wildcard *.cpp)
PROTO_SRCS= ../proto/addressbook.pb.cc


APP_OBJS=$(APP_SRCS:.cpp=.o)
PROTO_OBJS= ../proto/addressbook.pb.o



INCLUDES=-I./ -I../proto -I../include -I../
LIBRARY=-L../lib -lprotobuf-lite -lprotobuf -lprotoc


all: Project


$(PROTO_OBJS): $(PROTO_SRCS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<



Project : $(APP_OBJS) $(PROTO_OBJS)
	$(CXX) $(CXXFLAGS) $(PROTO_OBJS)  $(INCLUDES) $(LIBRARY) $< -o $@

clean:
	rm -f *.o ../proto/*.o Project
