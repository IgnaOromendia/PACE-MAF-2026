CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -Isrc -Isrc/core -Isrc/model -Isrc/solver

TARGET := pace_maf
SRC := $(wildcard src/*.cpp src/core/*.cpp src/model/*.cpp src/solver/*.cpp)
BUILD_DIR := build
OBJ := $(addprefix $(BUILD_DIR)/,$(SRC:.cpp=.o))

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET) input/dataset/tiny01.nw

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
