CXX = g++
CXXFLAGS = -Wall -Wextra -Wshadow -Wdisabled-optimization \
		   -Wlogical-op -Wconversion -Wcast-qual \
		   -Wdeprecated -Wundef -pthread -std=gnu++17
OPTFLAGS = -march=native -Ofast -flto -frename-registers -DNDEBUG
CXXFLAGS += $(OPTFLAGS)

BINARY = emain

all: main.cpp ../src/misc.cpp ../src/movegen.cpp
	$(CXX) $(CXXFLAGS) $^ -o $(BINARY)

.PHONY: clean
clean:
	rm $(BINARY)
