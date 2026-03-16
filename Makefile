CXX := g++
TARGET := paceSolver
BUILD_DIR := build
DIR_TEST ?= input/test
DIR ?= input/dataset

CPPFLAGS := -Isrc -Isrc/core -Isrc/model -Isrc/solver
CXXFLAGS := -std=c++17 -Wall -Wextra -pedantic
LDLIBS := -lilocplex -lconcert -lcplex -lm -lpthread

SRC := $(wildcard src/*.cpp src/core/*.cpp src/model/*.cpp src/service/*.cpp)

CPLEX_STUDIO_DIR ?= /Applications/CPLEX_Studio2211
CPLEX_ARCH ?= arm64_osx
CPLEX_LIBDIR := $(CPLEX_STUDIO_DIR)/cplex/lib/$(CPLEX_ARCH)/static_pic
CONCERT_LIBDIR := $(CPLEX_STUDIO_DIR)/concert/lib/$(CPLEX_ARCH)/static_pic

CPPFLAGS += -DIL_STD -isystem $(CPLEX_STUDIO_DIR)/cplex/include -isystem $(CPLEX_STUDIO_DIR)/concert/include
LDFLAGS += -L$(CPLEX_LIBDIR) -L$(CONCERT_LIBDIR)

OBJ := $(addprefix $(BUILD_DIR)/,$(SRC:.cpp=.o))

.PHONY: all debug run-all test clean

all: $(TARGET)

$(TARGET): $(OBJ)
	@$(CXX) $(CXXFLAGS) $(OBJ) $(LDFLAGS) $(LDLIBS) -o $(TARGET)

$(BUILD_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	@$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

run-all: $(TARGET)
	@for file in $(DIR)/*.nw; do \
		./$(TARGET) "$$file" || exit $$?; \
	done

debug: $(TARGET)
	@./$(TARGET) $(DIR_TEST)/test03.nw

test: $(TARGET)
	@for file in $(DIR_TEST)/*.nw; do \
		./$(TARGET) "$$file" || exit $$?; \
	done

clean:
	@rm -rf $(BUILD_DIR) $(TARGET)
