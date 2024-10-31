# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Werror -g -std=c++20

# Target executable
TARGET = program

# Source files
SRCS = matrix_compressor.cpp lib/repair.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default rule to build the target
all: $(TARGET)

# Rule to link object files and create the executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile .cpp files into .o files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -rf $(OBJS) $(TARGET) lib/repair output.txt *.o output.csv dataset/*.V dataset/*.re32

# Rule to rebuild everything from scratch
rebuild: clean all

