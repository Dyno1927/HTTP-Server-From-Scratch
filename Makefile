CXX      = clang++
CXXFLAGS = -std=c++20 -Wall -Wextra -g -O0 -IInclude
SAN      = -fsanitize=address,undefined

SRCS := $(wildcard Src/*.cpp)
OBJS := $(patsubst Src/%.cpp,Build/%.o,$(SRCS))

all: $(if $(strip $(SRCS)),Build/Server)

Build/Server: $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

Build/%.o: Src/%.cpp | Build
	$(CXX) $(CXXFLAGS) -c $< -o $@

Build:
	mkdir -p Build

run: Build/Server
	./Build/Server

# parser unit tests. compiles Tests/*.cpp against the parser implementation.
TESTS := $(patsubst Tests/%.cpp,Build/%,$(wildcard Tests/*.cpp))

test: $(TESTS)
	@for t in $(TESTS); do echo "== $$t =="; ./$$t || exit 1; done

Build/ParserTests: Tests/ParserTests.cpp Src/HTTPParser.cpp | Build
	$(CXX) $(CXXFLAGS) -o $@ $^

san:
	$(MAKE) clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(SAN)" all

.PHONY: all run san clean test
clean:
	rm -rf Build
