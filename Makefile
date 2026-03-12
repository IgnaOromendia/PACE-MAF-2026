CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic -Isrc -Isrc/core -Isrc/model -Isrc/solver

TARGET := pace_maf
SRC := $(wildcard src/*.cpp src/core/*.cpp src/model/*.cpp src/solver/*.cpp)
BUILD_DIR := build
OBJ := $(addprefix $(BUILD_DIR)/,$(SRC:.cpp=.o))
DIR_TEST ?= input/test
DIR ?= input/dataset

.PHONY: all debug run-all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(TARGET)

$(BUILD_DIR)/%.o: %.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run-all: $(TARGET)
	for file in $(DIR)/*.nw; do \
		./$(TARGET) "$$file" || exit $$?; \
	done

debug: $(TARGET)
		./$(TARGET) $(DIR_TEST)/test01.nw

test: $(TARGET)
	for file in $(DIR_TEST)/*.nw; do \
		./$(TARGET) "$$file" || exit $$?; \
	done

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
