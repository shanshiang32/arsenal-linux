.DEFAULT_GOAL := all
CXX ?= g++
MODE ?= release
CPPFLAGS += -Iinclude
CXXFLAGS += -std=c++17 -Wall -Wextra -Wpedantic -Wconversion -MMD -MP
ifeq ($(MODE),debug)
CXXFLAGS += -O0 -g3 -D_GLIBCXX_ASSERTIONS
else ifeq ($(MODE),sanitize)
CXXFLAGS += -O1 -g -fsanitize=address,undefined -fno-omit-frame-pointer
LDFLAGS += -fsanitize=address,undefined
else ifeq ($(MODE),release)
CXXFLAGS += -O2 -DNDEBUG
else
$(error MODE must be release, debug or sanitize)
endif
BUILD := build/$(MODE)
TARGET := $(BUILD)/arsenal
SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst src/%.cpp,$(BUILD)/%.o,$(SOURCES))
CORE := $(filter-out $(BUILD)/main.o $(BUILD)/ui.o,$(OBJECTS))
TEST := $(BUILD)/test_runner
.PHONY: all run demo test debug release sanitize clean dist help
all: $(TARGET)
$(TARGET): $(OBJECTS)
	$(CXX) $^ $(LDFLAGS) -o $@
$(BUILD)/%.o: src/%.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(BUILD)/test.o: tests/test.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@
$(TEST): $(CORE) $(BUILD)/test.o
	$(CXX) $^ $(LDFLAGS) -o $@
run: $(TARGET)
	./$(TARGET)
demo: $(TARGET)
	bash scripts/demo.sh ./$(TARGET)
test: $(TEST) $(TARGET)
	./$(TEST)
	python3 tests/cli_test.py ./$(TARGET)
debug:
	$(MAKE) MODE=debug all
release:
	$(MAKE) MODE=release all
sanitize:
	$(MAKE) MODE=sanitize test
clean:
	rm -rf build
dist:
	@mkdir -p dist
	tar --exclude=./build --exclude=./dist --exclude=./.git --exclude='*.sav*' -czf dist/arsenal-source.tar.gz .
help:
	@echo 'make [-j4] | run | demo | test | debug | release | sanitize | clean | dist'
-include $(OBJECTS:.o=.d) $(BUILD)/test.d
