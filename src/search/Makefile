CXX = g++
CXXFLAGS = -Wall -Wextra -Wshadow -Wdisabled-optimization \
		   -Wlogical-op -Wconversion -Wcast-qual \
		   -Wdeprecated -Wundef -std=gnu++17

OPTFLAGS += -march=native -Ofast -frename-registers -DNDEBUG

MAIN = main.cpp
BIN = main

all: $(MAIN)
	$(CXX) $(CXXFLAGS) main.cpp -o $(BIN)

perf: CXXFLAGS += $(OPTFLAGS)
perf: all

clean:
	rm -f $(BIN)
