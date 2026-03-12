CXX = g++
TARGET = Blobfish
SRCS = $(wildcard src/*.cpp)

FLAGS = -Wall -O3 -march=native -mtune=native \
        -mbmi2 -mpopcnt \
        -flto=$(shell nproc) -fno-exceptions -fno-rtti \
        -fno-math-errno -fno-trapping-math -fno-signaling-nans \
        -DNDEBUG

all:
	$(CXX) $(FLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
