# Makefile for Embedded Systems Programming Homework

# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c11
LDFLAGS = -lrt # Required for clock_gettime
DEBUG_FLAGS = -g

# Target Executables
TARGET_64 = ctl64
TARGET_32 = ctl32

# Source Files
SRCS = \
	sensor/sensor.c \
	actuators/led_actuator.c \
	actuators/buzzer_actuator.c \
	controller/ctl.c

# Object Files
OBJS = $(SRCS:.c=.o)

# Include directories
INCLUDE_DIRS = -I./sensor -I./actuators -I./controller

.PHONY: all clean ctl64 ctl32

all: $(TARGET_64)

# Build 64-bit binary
$(TARGET_64): $(OBJS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(INCLUDE_DIRS) -o $@ $(OBJS) $(LDFLAGS)

# Build 32-bit binary (requires gcc-multilib)
$(TARGET_32): $(OBJS)
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(INCLUDE_DIRS) -m32 -o $@ $(OBJS) $(LDFLAGS)

# Generic rule for compiling .c to .o
%.o: %.c
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(INCLUDE_DIRS) -c $< -o $@

# Clean up build artifacts
clean:
	rm -f $(TARGET_64) $(TARGET_32) $(OBJS)
	rm -f $(shell find . -name "*.o") # Ensure all object files are removed