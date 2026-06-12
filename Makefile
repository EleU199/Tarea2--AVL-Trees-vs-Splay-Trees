CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude

SRC = src/main.cpp src/experiments.cpp
OUT = t2cpp

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

run: all
	./$(OUT)

debug:
	$(CXX) -std=c++17 -g -Wall -Wextra -Iinclude $(SRC) -o $(OUT)

clean:
	rm -f $(OUT) $(OUT).exe

.PHONY: all run debug clean