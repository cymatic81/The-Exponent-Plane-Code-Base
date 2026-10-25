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

STD_CFLAGS    := -std=c11 -Wall -Wextra
CFLAGS        ?= -O2 $(STD_CFLAGS) $(EXTRA_CFLAGS)

SRC_DIR       ?= .
BUILD_DIR     ?= build
TARGET_NAME   ?= mathlib_xplane_verify
TARGET        := $(BUILD_DIR)/$(TARGET_NAME)

HEADER_NAME   := mathlib_xplane.h
LIB_SRC       := mathlib_xplane.c
LIB_NAME      ?= libmathlib_xplane

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    SHARED_EXT := dylib
else
    SHARED_EXT := so
endif

SHARED_LIB    := $(LIB_NAME).$(SHARED_EXT)
SHARED_TARGET := $(BUILD_DIR)/$(SHARED_LIB)

# Object files mapping for standalone sandbox execution
SRCS          := $(SRC_DIR)/main.c
OBJS          := $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

.PHONY: all shared install uninstall clean remake

all: $(TARGET)

# Corrected macro syntax typo from $(SRC_DIR)/(LIB_SRC) to $(SRC_DIR)/$(LIB_SRC)
$(TARGET): $(OBJS) $(SRC_DIR)/$(LIB_SRC)
	@$(MKDIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) $(OBJS) $(SRC_DIR)/$(LIB_SRC) -o $(TARGET)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@$(MKDIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

shared: $(SHARED_TARGET)

$(SHARED_TARGET): $(SRC_DIR)/$(LIB_SRC) $(SRC_DIR)/$(HEADER_NAME)
	@$(MKDIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) -fPIC -shared $(SRC_DIR)/$(LIB_SRC) -o $(SHARED_TARGET)
	@echo "Shared library target generated successfully: $(SHARED_TARGET)"

install: shared
	@$(MKDIR) $(INCLUDE_DEST)
	@$(MKDIR) $(LIB_DEST)
	$(CP) $(SRC_DIR)/$(HEADER_NAME) $(INCLUDE_DEST)/$(HEADER_NAME)
	$(CP) $(SHARED_TARGET) $(LIB_DEST)/$(SHARED_LIB)

uninstall:
	$(RM) $(INCLUDE_DEST)/$(HEADER_NAME)
	$(RM) $(LIB_DEST)/$(SHARED_LIB)

clean:
	$(RM) $(OBJS) $(TARGET) $(BUILD_DIR)/libmathlib_xplane.*
	@if [ -d "$(BUILD_DIR)" ]; then rmdir "$(BUILD_DIR)" 2>/dev/null || true; fi

remake: clean all
