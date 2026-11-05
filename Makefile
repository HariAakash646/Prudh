# ---- Config ----
TARGET   := app
CXX      := g++
CXXFLAGS := -std=c++23 -Wall -Wextra -Iinclude -MMD -MP
LDFLAGS  :=
LDLIBS   :=

SRC_DIR   := src
BUILD_DIR := build

# ---- Files ----
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

# ---- Build modes: make / make debug / make release ----
.PHONY: all debug release run clean

all: CXXFLAGS += -O2
all: $(BUILD_DIR)/$(TARGET)

debug: CXXFLAGS += -g -O0 -DDEBUG
debug: $(BUILD_DIR)/$(TARGET)

release: CXXFLAGS += -O3 -DNDEBUG
release: $(BUILD_DIR)/$(TARGET)

$(BUILD_DIR)/$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: all
	./$(BUILD_DIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
