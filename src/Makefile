CXX = g++
CXXFLAGS = -Wall -Wextra -Wshadow -Wdisabled-optimization \
		   -Wlogical-op -Wconversion -Wcast-qual \
		   -Wdeprecated -Wundef -pthread -std=gnu++17
OPTFLAGS = -march=native -Ofast -flto -frename-registers -DNDEBUG
DBGFLAGS = -ggdb

BINARY = engine

OBJ_FILES = $(patsubst %.cpp, obj/%.o, $(wildcard *.cpp))
GENERIC = types.h Makefile


all: $(BINARY)

debug: $(BINARY)
debug: CXXFLAGS += $(DBGFLAGS)

perf: $(BINARY) 
perf: CXXFLAGS += $(OPTFLAGS)

pgo: 
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -fprofile-generate $(wildcard *.cpp) -o $(BINARY)
	./$(BINARY)
	$(CXX) $(CXXFLAGS) $(OPTFLAGS) -fprofile-use $(wildcard *.cpp) -o $(BINARY)



$(BINARY): $(OBJ_FILES)
	$(CXX) $(CXXFLAGS) $^ -o $@

obj/main.o: main.cpp misc.h movegen.h position.h | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/movegen.o: misc.h

obj/%.o: %.cpp %.h $(GENERIC) | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj: 
	mkdir obj


.PHONY: clean
clean:
	rm $(BINARY) obj/*.o

.PHONY: cleanpgo
cleanpgo:
	rm $(BINARY) *.gcda
