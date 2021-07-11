CC = gcc
CXX = g++
CFLAGS = -Wall -I include -I include/libcnary
CXXFLAGS = -std=c++17 -Wall -Wextra -I include

BIN	= dist
BUILD = build
SOURCEDIR = src
SOURCES := $(shell find $(SOURCEDIR) -name '*.c' -o -name '*.cpp')
OBJECTS := $(patsubst %,$(BUILD)/%.o,$(basename $(SOURCES)))

LIBRARIES = -lstdc++ -lsqlite3
EXECUTABLE = adam

all: adam

adam: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LIBRARIES) $(OBJECTS) -o $(BIN)/$(EXECUTABLE)

$(BUILD)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

.PHONY: clean
clean:
	$(RM) $(BIN)/$(EXECUTABLE)
	$(RM) $(OBJECTS)