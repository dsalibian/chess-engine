CXX = g++
CXXFLAGS = -Wall -Wextra -Wshadow -Wdisabled-optimization \
		   -Wlogical-op -Wconversion -Wcast-qual \
		   -Wdeprecated -Wundef -pthread -std=gnu++17

CXXFLAGS += -march=native -Ofast -flto -frename-registers -DNDEBUG

BINARY = seeds


all: $(BINARY)

$(BINARY): seed.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@


.PHONY: clean
clean:
	rm $(BINARY)
