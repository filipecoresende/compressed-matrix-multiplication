# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -Wall -Werror -O3 -std=c++20

# Target executable
TARGET = program

MALLOC_COUNT=
LDFLAGS=

ifneq ($(SYSTEM),Darwin)
	MALLOC_COUNT=external/malloc_count.o
endif

CXXFLAGS += $(LDFLAGS)

# Object files
OBJS = matrix_compressor.o\
			 lib/repair.o\
			 lib/utils.o ${MALLOC_COUNT}
			

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
	rm -rf $(OBJS) $(TARGET) dataset/*.{V,re32}

remove:
	rm -rf dataset/*.{V,re32}

# Rule to rebuild everything from scratch
rebuild: clean all


INPUT = dataset/covtype.csv 
BLOCKS = 1

run:
	./program  -c $(INPUT) $(BLOCKS)


