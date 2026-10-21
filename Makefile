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

san:
	$(MAKE) clean
	$(MAKE) CXXFLAGS="$(CXXFLAGS) $(SAN)" all

.PHONY: all run san clean
clean:
	rm -rf Build
