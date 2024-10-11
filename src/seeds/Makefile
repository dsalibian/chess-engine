CXX = g++
CXXFLAGS = -Wall -Wextra -Wshadow -Wdisabled-optimization \
		   -Wlogical-op -Wconversion -Wcast-qual \
		   -Wdeprecated -Wundef -pthread -std=gnu++17

CXXFLAGS += -march=native -Ofast -frename-registers -DNDEBUG

BINARY = run


all: $(BINARY)

$(BINARY): seeds.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@


.PHONY: clean
clean:
	rm $(BINARY)
