CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude

SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:src/%.cpp=build/%.o)
BIN = bin/practice5

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(CXXFLAGS) $^ -o $@

build/%.o: src/%.cpp include/car.h
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(BIN)
	mkdir -p data
	./$(BIN)

clean:
	rm -f build/*.o $(BIN) data/*

.PHONY: all run clean
