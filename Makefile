.POSIX:

CC ?= gcc
CFLAGS += -O3 -g -Wall -std=c11
BUILD_DIR = build
TARGET = $(BUILD_DIR)/hextoggle

include config.mk

ifndef VERSION
$(error Make sure `VERSION` is set to the correct version number, e.g. `1.0.0`)
endif

HEADERS = $(wildcard src/*.h)
SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

.PHONY: default build all clean test install uninstall
.PRECIOUS: $(TARGET) $(OBJECTS)

build: $(TARGET)
default: build
all: build

$(BUILD_DIR)/%.o: src/%.c $(HEADERS) config.mk Makefile
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -DHEXTOGGLE_VERSION="$(VERSION)" $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -Wall -o $@ $(LDLIBS)

clean:
	-rm -rf $(BUILD_DIR)

test: build
	echo test >$(BUILD_DIR)/input.txt
	./$(TARGET) $(BUILD_DIR)/input.txt $(BUILD_DIR)/hex.txt
	./$(TARGET) $(BUILD_DIR)/hex.txt $(BUILD_DIR)/output.txt
	diff -q $(BUILD_DIR)/input.txt $(BUILD_DIR)/output.txt

install: build
	mkdir -p $(PREFIX)/bin
	cp -f $(TARGET) $(PREFIX)/bin
	chmod 755 $(PREFIX)/bin/hextoggle

uninstall:
	rm -f $(PREFIX)/bin/hextoggle
