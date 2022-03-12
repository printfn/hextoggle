.POSIX:

# ?= allows overriding via environment variables
# += adds to an environment variable if one exists

# All assignments (`?=`, `+=`, or `=`) can be overridden
#     by calling make as e.g. `make VAR=custom_value`

CC ?= gcc
CFLAGS += -O3 -g -Wall -std=c99
LDFLAGS +=
BUILD_DIR = build
TARGET = ./$(BUILD_DIR)/hextoggle

# REPRODUCIBLE BUILDS
# set timestamps to 0
export ZERO_AR_DATE=1
# remove compilation dir from binary
CFLAGS += -fdebug-compilation-dir .
# remove build dir from binary (see `man ld` on macOS)
LDFLAGS += -Wl,-oso_prefix,$(realpath $(BUILD_DIR))

# include version number and prefix directory (defaults to `/usr/local`)
include config.mk

ifndef VERSION
$(error Make sure `VERSION` is set to the \
	correct version number, e.g. `1.0.0`)
endif

DIFF_TOOL = diffoscope --exclude-directory-metadata yes

HEADERS = $(wildcard src/*.h)
SOURCES = $(wildcard src/*.c)
OBJECTS = $(patsubst src/%.c, $(BUILD_DIR)/%.o, $(SOURCES))

.PHONY: default build all clean install uninstall test benchmark
.PRECIOUS: $(TARGET) $(OBJECTS)

build: $(TARGET)
default: build
all: build

$(BUILD_DIR)/%.o: src/%.c $(HEADERS) config.mk Makefile
	mkdir -p $(BUILD_DIR)
	$(CC) -c -DHEXTOGGLE_VERSION="$(VERSION)" $< -o $@ \
		$(CPPFLAGS) $(CFLAGS)

$(TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -Wall -o $@ $(LOADLIBES) $(LDLIBS)

# the - means that we are ignoring the return code of this command
clean:
	-rm -rf $(BUILD_DIR)

install: build
	mkdir -p $(PREFIX)/bin
	cp -f $(TARGET) $(PREFIX)/bin
	chmod 755 $(PREFIX)/bin/hextoggle

uninstall:
	-rm -f $(PREFIX)/bin/hextoggle

test: build
	echo test >$(BUILD_DIR)/input.txt
	$(TARGET) $(BUILD_DIR)/input.txt $(BUILD_DIR)/hex.txt
	$(TARGET) $(BUILD_DIR)/hex.txt $(BUILD_DIR)/output.txt
	diff -q $(BUILD_DIR)/input.txt $(BUILD_DIR)/output.txt
	rm $(BUILD_DIR)/input.txt $(BUILD_DIR)/output.txt \
		$(BUILD_DIR)/hex.txt

benchmark: build
	dd if=/dev/random of="$(BUILD_DIR)/bin.txt" bs=1048576 count=64
	time $(TARGET) "$(BUILD_DIR)/bin.txt" "$(BUILD_DIR)/hex.txt"
	time $(TARGET) "$(BUILD_DIR)/hex.txt" "$(BUILD_DIR)/bin.txt"
	rm "$(BUILD_DIR)/bin.txt" "$(BUILD_DIR)/hex.txt"

reproduce:
	$(MAKE) BUILD_DIR=$(BUILD_DIR)/a
	sleep 2
	$(MAKE) BUILD_DIR=$(BUILD_DIR)/b
	$(DIFF_TOOL) $(BUILD_DIR)/a $(BUILD_DIR)/b

ci: build reproduce
	shasum -a 256 $(TARGET) \
		$(BUILD_DIR)/a/hextoggle $(BUILD_DIR)/a/hextoggle
