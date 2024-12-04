CXX = g++

CXXFLAGS = -Wall -Werror -O3 -std=c++20

TARGET = program

MALLOC_COUNT=
LDFLAGS=

ifneq ($(SYSTEM),Darwin)
	MALLOC_COUNT=external/malloc_count.o
endif

CXXFLAGS += $(LDFLAGS)

# Object files
OBJS =  main.o\
		lib/right_mult.o\
	    lib/matrix_compressor.o\
		lib/repair.o\
		lib/utils.o ${MALLOC_COUNT}

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJS) $(TARGET) mult_output.csv decomp_output.csv dataset/*.{re32,V} 

remove:
	rm -rf dataset/*.{V,re32}

rebuild: clean all

