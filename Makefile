# Makefile for SPDK Ozone bdev module
#
# Copyright (c) 2024 Wei-Chiu Chuang
# Licensed under the Apache License, Version 2.0

# Configuration
SPDK_ROOT ?= $(HOME)/spdk
JAVA_HOME ?= /usr/lib/jvm/default-java
HADOOP_HOME ?= /usr/local/hadoop

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Werror -g -O2 -std=c11 -fPIC
CFLAGS += -I$(SPDK_ROOT)/include
CFLAGS += -I./include
CFLAGS += -I$(JAVA_HOME)/include
CFLAGS += -I$(JAVA_HOME)/include/linux
CFLAGS += -I$(HADOOP_HOME)/include

LDFLAGS = -L$(SPDK_ROOT)/build/lib
LDFLAGS += -L$(HADOOP_HOME)/lib/native
LDFLAGS += -lhdfs
LDFLAGS += -Wl,--whole-archive
LDFLAGS += -lspdk_bdev -lspdk_bdev_module -lspdk_thread -lspdk_util -lspdk_log
LDFLAGS += -Wl,--no-whole-archive
LDFLAGS += -lpthread -lrt -ldl

ifdef DEBUG
CFLAGS += -DDEBUG -O0
endif

# Source files
MODULE_SRCS = module/bdev_ozone.c
MODULE_OBJS = $(MODULE_SRCS:.c=.o)

LIB_SRCS = lib/ozone_io.c
LIB_OBJS = $(LIB_SRCS:.c=.o)

# Targets
TARGET_LIB = libozone_bdev.a
TARGET_SO = libozone_bdev.so

.PHONY: all clean install test help

all: $(TARGET_LIB) $(TARGET_SO)

# Build static library
$(TARGET_LIB): $(MODULE_OBJS) $(LIB_OBJS)
	@echo "Building static library: $@"
	ar rcs $@ $^

# Build shared library
$(TARGET_SO): $(MODULE_OBJS) $(LIB_OBJS)
	@echo "Building shared library: $@"
	$(CC) -shared -o $@ $^ $(LDFLAGS)

# Compile source files
%.o: %.c
	@echo "Compiling: $<"
	$(CC) $(CFLAGS) -c $< -o $@

# Generate dependency files
%.d: %.c
	@set -e; rm -f $@; \
	$(CC) -M $(CFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

# Include dependency files
-include $(MODULE_SRCS:.c=.d)
-include $(LIB_SRCS:.c=.d)

# Install target
install: $(TARGET_LIB) $(TARGET_SO)
	@echo "Installing to $(SPDK_ROOT)"
	install -d $(SPDK_ROOT)/lib
	install -m 644 $(TARGET_LIB) $(SPDK_ROOT)/lib/
	install -m 755 $(TARGET_SO) $(SPDK_ROOT)/lib/
	install -d $(SPDK_ROOT)/include/ozone
	install -m 644 include/*.h $(SPDK_ROOT)/include/ozone/

# Test target
test:
	@echo "Running tests..."
	@if [ -d test ]; then \
		cd test && $(MAKE) test; \
	else \
		echo "No tests available yet"; \
	fi

# Clean target
clean:
	@echo "Cleaning build artifacts..."
	rm -f $(MODULE_OBJS) $(LIB_OBJS)
	rm -f $(MODULE_SRCS:.c=.d) $(LIB_SRCS:.c=.d)
	rm -f $(TARGET_LIB) $(TARGET_SO)
	rm -f *.o *.d *.a *.so
	@if [ -d test ]; then cd test && $(MAKE) clean; fi

# Help target
help:
	@echo "SPDK Ozone bdev module build system"
	@echo ""
	@echo "Targets:"
	@echo "  all      - Build all libraries (default)"
	@echo "  install  - Install to SPDK directory"
	@echo "  test     - Run tests"
	@echo "  clean    - Remove build artifacts"
	@echo "  help     - Show this help message"
	@echo ""
	@echo "Configuration:"
	@echo "  SPDK_ROOT   - Path to SPDK installation (default: ~/spdk)"
	@echo "  JAVA_HOME   - Path to Java installation"
	@echo "  HADOOP_HOME - Path to Hadoop installation"
	@echo "  DEBUG=1     - Build with debug symbols"
	@echo ""
	@echo "Example:"
	@echo "  make SPDK_ROOT=/opt/spdk DEBUG=1"
