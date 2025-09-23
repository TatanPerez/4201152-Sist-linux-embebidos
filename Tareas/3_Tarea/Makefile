# CC=gcc
# CFLAGS=-std=c11 -O2 -Wall -Wextra
# BUILD_DIR=build
# BINARY=$(BUILD_DIR)/assignment-sensor

# .PHONY: all build install uninstall clean

# all: build

# build: src/assignment-sensor.c
# 	@echo "Building with $(CC)..."
# 	@mkdir -p $(BUILD_DIR)
# 	$(CC) $(CFLAGS) -o $(BINARY) src/assignment-sensor.c

# install: build
# 	@echo "Installing to /usr/local/bin (may require sudo)"
# 	sudo install -m 0755 $(BINARY) /usr/local/bin/assignment-sensor

# uninstall:
# 	@echo "Removing /usr/local/bin/assignment-sensor"
# 	sudo rm -f /usr/local/bin/assignment-sensor

# clean:
# 	@echo "Cleaning build artifacts"
# 	rm -rf $(BUILD_DIR)
# BINARY=/usr/local/bin/assignment-sensor
# BUILD_DIR=build
# GOFLAGS=

# .PHONY: all build install uninstall clean test

# all: build

# build:
# 	@echo "Building..."
# 	@mkdir -p $(BUILD_DIR)
# 	GOOS=linux GOARCH=amd64 go build -o $(BUILD_DIR)/assignment-sensor ./src

# install: build
# 	@echo "Installing to $(BINARY) (may require sudo)"
# 	sudo install -m 0755 $(BUILD_DIR)/assignment-sensor $(BINARY)

# uninstall:
# 	@echo "Removing $(BINARY)"
# 	sudo rm -f $(BINARY)

# clean:
# 	@echo "Cleaning..."
# 	rm -rf $(BUILD_DIR)

# test:
# 	@echo "Run tests described in tests/README.md"

# Makefile para assignment-sensor (proyecto en C)

CC=gcc
CFLAGS=-std=c11 -O2 -Wall -Wextra
BUILD_DIR=build
BINARY=$(BUILD_DIR)/assignment-sensor
SRC=src/assignment-sensor.c

.PHONY: all build install uninstall clean

all: build

build: $(SRC)
	@echo "Building with $(CC)..."
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $(BINARY) $(SRC)

install: build
	@echo "Installing to /usr/local/bin (may require sudo)"
	sudo install -m 0755 $(BINARY) /usr/local/bin/assignment-sensor

uninstall:
	@echo "Removing /usr/local/bin/assignment-sensor"
	sudo rm -f /usr/local/bin/assignment-sensor

clean:
	@echo "Cleaning build artifacts"
	rm -rf $(BUILD_DIR)
