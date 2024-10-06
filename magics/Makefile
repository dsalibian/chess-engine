CXX = g++
CXXFLAGS = -Wall -Wextra -Wshadow -Wdisabled-optimization \
		   -Wlogical-op -Wconversion -Wcast-qual \
		   -Wdeprecated -Wundef
OPTFLAGS = -march=native -Ofast -flto -frename-registers -DNDEBUG
CXXFLAGS += $(OPTFLAGS)

BINARY = emain

all: main.cpp ../src/misc.cpp ../src/movegen.cpp
	$(CXX) $(CXXFLAGS) $^ -o $(BINARY)

.PHONY: clean
clean:
	rm $(BINARY)
