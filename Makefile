CC = gcc
CXX = g++
CFLAGS = -Wall -c -I include
CXXFLAGS = -std=c++17 -Wall -Wextra -I include

BIN	= dist
CSOURCES = src/*.c
OBJECTC = *.o
CXXSOURCES = src/*.cpp

LIBRARIES = -lstdc++ -lsqlite3
EXECUTABLE = adam

all: adam

adam:
    # $(CC) $(CFLAGS) $(CSOURCES)
	$(CXX) $(CXXFLAGS) $(CXXSOURCES) -o $(BIN)/$(EXECUTABLE) $(LIBRARIES)

clean:
	$(RM) $(BIN)/$(EXECUTABLE)
	$(RM) $(OBJECTC)