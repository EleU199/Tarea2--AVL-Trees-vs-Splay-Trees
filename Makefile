CXX := g++
CXXFLAGS := -std=c++17 -O3 -march=native -DNDEBUG -Wall -Wextra -Iinclude

SRC := $(wildcard src/*.cpp)
OUT := t2cpp.exe

.PHONY: all run clean debug

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

run: all
	./$(OUT) results

debug:
	$(CXX) -std=c++17 -g -Wall -Wextra -Iinclude $(SRC) -o $(OUT)

clean:
	del /Q src\*.o 2>NUL
	del /Q $(OUT) 2>NUL