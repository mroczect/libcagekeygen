# ============================================================================
# Makefile – Standalone build & install for libcagekeygen (no CMake required)
# ============================================================================
# Usage:
#   make                  Build static library (libcagekeygen.a)
#   make shared           Build shared library (libcagekeygen.so / .dylib)
#   make test             Build and run test suite
#   make install          Install to /usr/local (override with PREFIX)
#   make uninstall        Remove installed files
#   make clean            Clean build artifacts
#
# Variables you can override:
#   CC                    C compiler (default: cc)
#   PREFIX                Installation prefix (default: /usr/local)
#   DESTDIR               Staging directory for package creation
#   CFLAGS                Extra C compiler flags
#   BUILD_TYPE            Debug or Release (default: Release)
# ============================================================================

.PHONY: all static shared test install uninstall clean

# Compiler & tools
CC      ?= cc
AR      ?= ar
RANLIB  ?= ranlib
RM      ?= rm -f
MKDIR   ?= mkdir -p
INSTALL ?= install

# Paths
PREFIX   ?= /usr/local
INCDIR   ?= $(DESTDIR)$(PREFIX)/include
LIBDIR   ?= $(DESTDIR)$(PREFIX)/lib
SRCDIR   := src
INCDIRS  := include
TESTDIR  := test

# Source files
SRCS     := $(SRCDIR)/curve25519-donna.c \
            $(SRCDIR)/gen.c \
            $(SRCDIR)/io.c \
            $(SRCDIR)/error.c \
            $(SRCDIR)/lib.c

# Object files
OBJS     := $(SRCS:.c=.o)

# Test sources & objects
TEST_SRCS := $(TESTDIR)/test.c \
             $(TESTDIR)/test_gen.c \
             $(TESTDIR)/test_io.c \
             $(TESTDIR)/test_random.c
TEST_OBJS := $(TEST_SRCS:.c=.o)
TEST_EXE  := test_libcagekeygen

# Library names
STATIC_LIB  := libcagekeygen.a
ifeq ($(shell uname -s),Darwin)
    SHARED_LIB := libcagekeygen.dylib
    SHARED_FLAG := -dynamiclib
    SHARED_EXT  := .dylib
else
    SHARED_LIB := libcagekeygen.so
    SHARED_FLAG := -shared
    SHARED_EXT  := .so
endif

# Compiler flags
BASE_CFLAGS := -std=c11 -I$(INCDIRS)
WARN_FLAGS  := -Wall -Wextra -Wpedantic
OPT_FLAGS   :=
ifeq ($(BUILD_TYPE),Debug)
    OPT_FLAGS := -g -O0 -DDEBUG
    SAN_FLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer
else
    OPT_FLAGS := -O3 -DNDEBUG
    SAN_FLAGS :=
endif
CFLAGS_ALL := $(BASE_CFLAGS) $(WARN_FLAGS) $(OPT_FLAGS) $(SAN_FLAGS) $(CFLAGS)

# Platform-specific
ifeq ($(OS),Windows_NT)
    # Windows not supported with plain Makefile; use CMake
    $(error Use CMake for Windows builds)
endif

# ============================================================================
# Targets
# ============================================================================

all: static

static: $(STATIC_LIB)

shared: $(SHARED_LIB)

# Build static library
$(STATIC_LIB): $(OBJS)
	$(AR) rcs $@ $(OBJS)
	$(RANLIB) $@
	@echo "  ✓ Built $@"

# Build shared library
$(SHARED_LIB): $(OBJS)
	$(CC) $(SHARED_FLAG) -o $@ $(OBJS) $(LDFLAGS)
	@echo "  ✓ Built $@"

# Compile C sources
$(SRCDIR)/%.o: $(SRCDIR)/%.c $(INCDIRS)/libcagekeygen.h
	$(CC) $(CFLAGS_ALL) -c $< -o $@

# ============================================================================
# Test
# ============================================================================

test: $(TEST_EXE)
	./$(TEST_EXE)

$(TEST_EXE): $(STATIC_LIB) $(TEST_OBJS)
	$(CC) $(CFLAGS_ALL) -o $@ $(TEST_OBJS) -L. -lcagekeygen $(LDFLAGS)

$(TESTDIR)/%.o: $(TESTDIR)/%.c $(INCDIRS)/libcagekeygen.h $(TESTDIR)/test_utils.h
	$(CC) $(CFLAGS_ALL) -I$(INCDIRS) -c $< -o $@

# ============================================================================
# Install / Uninstall
# ============================================================================

install: $(STATIC_LIB)
	$(MKDIR) $(INCDIR) $(LIBDIR)
	$(INSTALL) -m 644 $(INCDIRS)/libcagekeygen.h $(INCDIR)/
	$(INSTALL) -m 644 $(STATIC_LIB) $(LIBDIR)/
	@echo "✓ Installed to $(PREFIX)"
	@echo "  Header : $(INCDIR)/libcagekeygen.h"
	@echo "  Library: $(LIBDIR)/$(STATIC_LIB)"

install-shared: $(SHARED_LIB)
	$(MKDIR) $(INCDIR) $(LIBDIR)
	$(INSTALL) -m 644 $(INCDIRS)/libcagekeygen.h $(INCDIR)/
	$(INSTALL) -m 755 $(SHARED_LIB) $(LIBDIR)/
	@if [ "$(shell uname -s)" = "Linux" ]; then \
		ldconfig || true; \
	fi
	@echo "✓ Installed shared library to $(PREFIX)"

uninstall:
	$(RM) $(INCDIR)/libcagekeygen.h
	$(RM) $(LIBDIR)/libcagekeygen.a
	$(RM) $(LIBDIR)/libcagekeygen$(SHARED_EXT)
	@echo "✓ Uninstalled from $(PREFIX)"

# ============================================================================
# Clean
# ============================================================================

clean:
	$(RM) $(OBJS) $(TEST_OBJS) $(STATIC_LIB) $(SHARED_LIB) $(TEST_EXE)
	@echo "✓ Cleaned"
