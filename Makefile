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
all: gui
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
cli-run: $(TARGET)
	./$(TARGET)
run: gui
	./$(BUILD)/arsenal-desktop
demo: $(TARGET)
	bash scripts/demo.sh ./$(TARGET)
test: core-test gui-test
core-test: $(TEST) $(TARGET)
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

# Qt桌面版：手写Make规则，展示moc、rcc、自动依赖与系统第三方库链接。
PKG_CONFIG ?= pkg-config
QT_CFLAGS := $(subst -I,-isystem ,$(shell $(PKG_CONFIG) --cflags Qt6Widgets 2>/dev/null))
QT_MOC_CFLAGS := $(shell $(PKG_CONFIG) --cflags Qt6Widgets Qt6Test 2>/dev/null)
QT_LIBS := $(shell $(PKG_CONFIG) --libs Qt6Widgets 2>/dev/null)
QT_TEST_LIBS := $(shell $(PKG_CONFIG) --libs Qt6Test 2>/dev/null)
QT_TEST_CFLAGS := $(subst -I,-isystem ,$(shell $(PKG_CONFIG) --cflags Qt6Test 2>/dev/null))
QT_LIBEXEC := $(shell $(PKG_CONFIG) --variable=libexecdir Qt6Core 2>/dev/null)
MOC ?= $(QT_LIBEXEC)/moc
RCC ?= $(QT_LIBEXEC)/rcc
GUI_SOURCES := $(wildcard gui/*.cpp)
GUI_OBJECTS := $(patsubst gui/%.cpp,$(BUILD)/gui/%.o,$(GUI_SOURCES))
MOC_HEADERS := session window reveal
MOC_OBJECTS := $(addprefix $(BUILD)/gui/moc_,$(addsuffix .o,$(MOC_HEADERS)))
RES_OBJECT := $(BUILD)/gui/resources.o
GUI_SHARED := $(filter-out $(BUILD)/gui/main.o,$(GUI_OBJECTS)) $(MOC_OBJECTS) $(RES_OBJECT)
GUI_TARGET := $(BUILD)/arsenal-desktop
.PHONY: gui cli-run core-test gui-test check-qt check-deps
check-qt:
	@$(PKG_CONFIG) --exists Qt6Widgets Qt6Test || { echo 'Install: sudo apt install qt6-base-dev qt6-base-dev-tools pkg-config'; exit 1; }
	@test -x "$(MOC)" -a -x "$(RCC)" || { echo 'Qt moc/rcc missing; set MOC=... RCC=...'; exit 1; }
check-deps: check-qt
	@$(PKG_CONFIG) --modversion Qt6Widgets Qt6Test
	@echo 'C++ compiler:'
	@$(CXX) --version | head -n 1
gui: $(GUI_TARGET)
$(BUILD)/gui/%.o: gui/%.cpp Makefile | check-qt
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) -Igui $(QT_CFLAGS) $(CXXFLAGS) -fPIC -c $< -o $@
$(BUILD)/gui/moc_%.cpp: gui/%.hpp Makefile | check-qt
	@mkdir -p $(@D)
	$(MOC) -Iinclude -Igui $(QT_MOC_CFLAGS) $< -o $@
$(BUILD)/gui/moc_%.o: $(BUILD)/gui/moc_%.cpp Makefile
	$(CXX) $(CPPFLAGS) -Igui $(QT_CFLAGS) $(CXXFLAGS) -fPIC -c $< -o $@
$(BUILD)/gui/resources.cpp: resources/resources.qrc resources/theme.qss Makefile | check-qt
	@mkdir -p $(@D)
	$(RCC) $< -o $@
$(RES_OBJECT): $(BUILD)/gui/resources.cpp
	$(CXX) $(QT_CFLAGS) $(CXXFLAGS) -fPIC -c $< -o $@
$(GUI_TARGET): $(CORE) $(GUI_SHARED) $(BUILD)/gui/main.o
	$(CXX) $^ $(LDFLAGS) $(QT_LIBS) -o $@
$(BUILD)/gui/test.moc: tests/gui_test.cpp Makefile | check-qt
	@mkdir -p $(@D)
	$(MOC) -Igui -Iinclude $(QT_MOC_CFLAGS) $< -o $@
$(BUILD)/gui/gui_test.o: tests/gui_test.cpp $(BUILD)/gui/test.moc Makefile
	$(CXX) $(CPPFLAGS) -Igui -I$(BUILD)/gui $(QT_CFLAGS) $(QT_TEST_CFLAGS) $(CXXFLAGS) -fPIC -c $< -o $@
$(BUILD)/gui_tests: $(CORE) $(GUI_SHARED) $(BUILD)/gui/gui_test.o
	$(CXX) $^ $(LDFLAGS) $(QT_LIBS) $(QT_TEST_LIBS) -o $@
gui-test: $(BUILD)/gui_tests
	QT_QPA_PLATFORM=offscreen ./$(BUILD)/gui_tests
.SECONDARY:
-include $(GUI_OBJECTS:.o=.d) $(MOC_OBJECTS:.o=.d) $(BUILD)/gui/gui_test.d $(RES_OBJECT:.o=.d)
