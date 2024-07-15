CXXFLAGS =  -Wall -Werror
LIB = lib

all: repair

repair: repair.cpp
	g++ -std=c++20 $(CXXFLAGS) $^ -o $@

clean:
	rm -rf repair decompressedString.txt dataset/*.re8 dataset/*.re16 dataset/*.re32
