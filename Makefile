CC		:= g++
C_FLAGS := -std=c++17 -Wall -Wextra

BIN		:= ./dist
SRC		:= ./src
INCLUDE	:= ./include

LIBRARIES	:= -lstdc++ -lsqlite3

EXECUTABLE	:= adam

all: $(BIN)/$(EXECUTABLE)

clean:
	$(RM) $(BIN)/$(EXECUTABLE)

run: all
	./$(BIN)/$(EXECUTABLE)

$(BIN)/$(EXECUTABLE): $(SRC)/*.cpp
	$(CC) $(C_FLAGS) -I$(INCLUDE) $^ -o $@ $(LIBRARIES)
