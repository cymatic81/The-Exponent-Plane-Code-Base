# =============================================================================
# GENERAL MATHLIB EXTENSION SUBSYSTEM: THE EXPONENT PLANE ENGINE
# Universal, overridable template supporting install, uninstall, and shared targets.
# =============================================================================

CC            ?= clang
RM            ?= rm -f
MKDIR         ?= mkdir -p
CP            := cp -f

PREFIX        ?= /usr/local
INCLUDE_DEST  := $(PREFIX)/include
LIB_DEST      := $(PREFIX)/lib

# Search paths to safely handle includes across workspace configurations
STD_CFLAGS    := -std=c11 -Wall -Wextra -I. -I"Test Headers" -I"Wrappers"
CFLAGS        ?= -O2 $(STD_CFLAGS) $(EXTRA_CFLAGS)

BUILD_DIR     ?= build
TARGET_NAME   ?= mathlib_xplane_verify
TARGET        := $(BUILD_DIR)/$(TARGET_NAME)

HEADER_NAME   := mathlib_xplane.h
LIB_NAME      ?= libmathlib_xplane

# FLAT ROOT MAPPINGS: Sourced directly from your primary project folder
CORE_SRC      := mathlib_xplane.c
FLOAT_SRC     := mathlib_xplane_float64.c
LEAN_SRC      := mathlib_xplane_lean.c

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    SHARED_EXT := dylib
else
    SHARED_EXT := so
endif

SHARED_LIB    := $(LIB_NAME).$(SHARED_EXT)
SHARED_TARGET := $(BUILD_DIR)/$(SHARED_LIB)

# Object files mapping for standalone sandbox execution (main.c)
OBJS          := $(BUILD_DIR)/main.o

# All 3 core tracking components built sequentially
ALL_THREE_OBJS := $(BUILD_DIR)/mathlib_xplane.o \
                  $(BUILD_DIR)/mathlib_xplane_float64.o \
                  $(BUILD_DIR)/mathlib_xplane_lean.o

.PHONY: all shared install uninstall clean remake

all: $(TARGET)

# Compile verification runner linking main.o and all 3 root subsystem objects
$(TARGET): $(OBJS) $(ALL_THREE_OBJS)
	@$(MKDIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OBJS) $(ALL_THREE_OBJS) -o $(TARGET)

# Rule for building the test-harness runner (main.o)
$(BUILD_DIR)/main.o: main.c
	@$(MKDIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Object 1: Original Core Evaluation Subsystem
$(BUILD_DIR)/mathlib_xplane.o: $(CORE_SRC) $(HEADER_NAME)
	@$(MKDIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Object 2: Bitwise Floating Point Quantiser Wrapper
$(BUILD_DIR)/mathlib_xplane_float64.o: $(FLOAT_SRC) $(HEADER_NAME)
	@$(MKDIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Object 3: Lean Safety Compliance Wrapper
$(BUILD_DIR)/mathlib_xplane_lean.o: $(LEAN_SRC) $(HEADER_NAME)
	@$(MKDIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

shared: $(SHARED_TARGET)

# Consolidated shared object library target compiled directly from root source files
$(SHARED_TARGET): $(CORE_SRC) $(FLOAT_SRC) $(LEAN_SRC) $(HEADER_NAME)
	@$(MKDIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) -fPIC -shared $(CORE_SRC) $(FLOAT_SRC) $(LEAN_SRC) -o $(SHARED_TARGET)
	@echo "Shared library target generated successfully: $(SHARED_TARGET)"

install: shared
	@$(MKDIR) $(INCLUDE_DEST)
	@$(MKDIR) $(LIB_DEST)
	$(CP) $(HEADER_NAME) $(INCLUDE_DEST)/$(HEADER_NAME)
	$(CP) $(SHARED_TARGET) $(LIB_DEST)/$(SHARED_LIB)

uninstall:
	$(RM) $(INCLUDE_DEST)/$(HEADER_NAME)
	$(RM) $(LIB_DEST)/$(SHARED_LIB)

clean:
	$(RM) $(OBJS) $(ALL_THREE_OBJS) $(TARGET) $(BUILD_DIR)/libmathlib_xplane.*
	@if [ -d "$(BUILD_DIR)" ]; then rmdir "$(BUILD_DIR)" 2>/dev/null || true; fi

remake: clean all
