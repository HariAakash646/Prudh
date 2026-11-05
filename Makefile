# ---- Config ----
TARGET   := app
CXX      := g++-15
CXXFLAGS := -std=c++23 -Wall -Wextra -Iinclude -MMD -MP
LDFLAGS  :=
LDLIBS   :=

SRC_DIR   := src
BUILD_DIR := build

# ---- Files ----
SRCS := $(shell find $(SRC_DIR) -name '*.cpp')
OBJS := $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)

TEST_DIR    := tests
TEST_TARGET := $(BUILD_DIR)/test_runner
TEST_SRCS   := $(shell find $(TEST_DIR) -name '*.cpp')
TEST_OBJS   := $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(BUILD_DIR)/tests/%.o)
# All app objects except the one containing main()
LIB_OBJS    := $(filter-out $(BUILD_DIR)/main.o,$(OBJS))
DEPS        += $(TEST_OBJS:.o=.d)

# ---- Build modes: make / make debug / make release ----
.PHONY: all debug release run test clean

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

# ---- Tests: make test ----
test: CXXFLAGS += -g -O0
test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) $(LIB_OBJS)
	$(CXX) $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(BUILD_DIR)/tests/%.o: $(TEST_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -I$(TEST_DIR)/vendor -c $< -o $@

run: all
	./$(BUILD_DIR)/$(TARGET)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
