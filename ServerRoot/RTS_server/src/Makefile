SRCS=$(wildcard *.cpp)

OBJS=$(SRCS:.cpp=.o)

.cpp.o:
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

all: Project


Project : $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@
