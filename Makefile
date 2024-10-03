# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Werror -g -std=c++20

# Target executable
TARGET = src/repair

# Source file
SRCS = src/repair.cpp

# Default rule to build the executable
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $<

# Clean rule to remove the generated files
clean:
	rm -rf $(TARGET) src/output.txt src/repair.dSYM
