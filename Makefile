# ============================================================
# SunlixDBMS Makefile
# ============================================================


# ------------------------------------------------------------
# Compiler
# ------------------------------------------------------------

# Use the compiler provided by the environment.
# If CC is not already defined, use gcc.
CC ?= gcc


# ------------------------------------------------------------
# Compiler Detection
# ------------------------------------------------------------

# Get the first line of the compiler version output.
# This is used to detect GCC or Clang.
COMPILER_NAME := $(shell $(CC) --version 2>&1 | head -n 1)


# ------------------------------------------------------------
# Compiler Flags
# ------------------------------------------------------------

# Clang-specific compiler settings.
ifneq ($(findstring clang,$(COMPILER_NAME)),)

    COMPILER_TYPE = Clang

    CFLAGS = -Wall \
             -Wextra \
             -O3 \
             -std=c11 \
             -Rpass=loop-vectorize


# GCC-specific compiler settings.
else ifneq ($(findstring gcc,$(COMPILER_NAME)),)

    COMPILER_TYPE = GCC

    CFLAGS = -Wall \
             -Wextra \
             -O3 \
             -std=c11 \
             -fopt-info-vec


# Fallback settings for unknown compilers.
else

    COMPILER_TYPE = Unknown

    CFLAGS = -Wall \
             -O2

endif


# ------------------------------------------------------------
# Header Include Directories
# ------------------------------------------------------------

# Tell the compiler where to search for header files.
CFLAGS += -I src \
          -I src/functions \
          -I src/functions/utility \
          -I packages/cJSON


# ------------------------------------------------------------
# Project Target
# ------------------------------------------------------------

# Name of the final executable.
TARGET = sunlixDBMS


# ------------------------------------------------------------
# Source Files
# ------------------------------------------------------------

# Automatically find every .c file inside src/.
#
# This means new source files do not need to be
# manually added to the Makefile.
#
# Example:
#
#   src/functions/crud/reader.c
#   src/functions/crud/delete.c
#   src/functions/cli/help.c
#
# will automatically be included.
SOURCES := $(shell find src -type f -name '*.c')


# Add the cJSON library source file.
#
# cJSON is kept outside the src/ directory, so it needs
# to be added separately.
SOURCES += packages/cJSON/cJSON.c


# Convert every .c source file into its corresponding .o file.
#
# Example:
#
#   src/main.c
#       ↓
#   src/main.o
#
OBJECTS := $(SOURCES:.c=.o)


# ------------------------------------------------------------
# Phony Targets
# ------------------------------------------------------------

# These targets are commands rather than actual files.
.PHONY: all clean info run release


# ------------------------------------------------------------
# Default Target
# ------------------------------------------------------------

# Running just "make" will execute:
#
#   info
#   sunlixDBMS
#
all: info $(TARGET)


# ------------------------------------------------------------
# Build Information
# ------------------------------------------------------------

# Display information about the current build.
info:
	@echo "--- Building DBMS Engine with [$(COMPILER_TYPE)] ---"
	@echo "--- Compiler: $(CC) ---"
	@echo "--- Sources: $(words $(SOURCES)) files ---"


# ------------------------------------------------------------
# Linking
# ------------------------------------------------------------

# Link all object files into the final executable.
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)


# ------------------------------------------------------------
# Compilation
# ------------------------------------------------------------

# Compile every .c file into a .o object file.
#
# $< = source file
# $@ = output object file
#
# Example:
#
#   reader.c → reader.o
#
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@


# ------------------------------------------------------------
# Clean
# ------------------------------------------------------------

# Remove the executable and all generated object files.
clean:
	rm -f $(TARGET) $(OBJECTS)


# ------------------------------------------------------------
# Run
# ------------------------------------------------------------

# Build the project first, then run SunlixDBMS.
run: $(TARGET)
	./$(TARGET)

# ------------------------------------------------------------
# Release Build
# ------------------------------------------------------------

# Build the executable and remove all object files afterward.
#
# Useful when you only want the final SunlixDBMS executable
# without keeping intermediate .o files.
release: $(TARGET)
	rm -f $(OBJECTS)

	@echo "--- Release build complete ---"
	@echo "--- Object files removed ---"