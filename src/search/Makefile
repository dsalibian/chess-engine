CXX = g++
CXXFLAGS = -Wall -Wextra -Wshadow -Wdisabled-optimization \
		   -Wlogical-op -Wconversion -Wcast-qual \
		   -Wdeprecated -Wundef -std=gnu++17

CXXFLAGS += -march=native -Ofast -frename-registers -DNDEBUG

SEEDS_BIN = seed
MAGIC_BIN = magic

all:
	@echo "use 'make seeds' or 'make magics'"


seeds: obj/seeds.o obj/tbls.o obj/ran.o
	$(CXX) $(CXXFLAGS) $^ -o $(SEEDS_BIN)

magics: obj/magics.o obj/tbls.o obj/ran.o
	$(CXX) $(CXXFLAGS) $^ -o $(MAGIC_BIN)


obj/%.o: %.cpp %.h | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/seeds.o: seeds.cpp tbls.h ran.h | obj
	$(CXX) $(CXXFLAGS) -c $< -o $@

obj/magics.o: magics.cpp tbls.h ran.h| obj
	$(CXX) $(CXXFLAGS) -c $< -o $@


obj: 
	mkdir obj

clean:
	rm -f $(SEEDS_BIN) $(MAGIC_BIN) obj/*.o
