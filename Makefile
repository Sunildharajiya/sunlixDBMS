# 1. Look for the system compiler (defaults to 'cc' if not specified)
CC ?= gcc

# 2. CONDITIONAL CHECKS: Detect if the compiler is GCC or Clang
ifeq ($(origin CC), default)
    # If the system default is 'cc', let's check what it actually points to
    COMPILER_NAME := $(shell $(CC) --version 2>&1 | head -n 1)
else
    COMPILER_NAME := $(shell $(CC) --version 2>&1 | head -n 1)
endif

# 3. Apply custom flags based on the detected compiler string
ifneq ($(findstring clang,$(COMPILER_NAME)),)
    # --- CLANG SPECIFIC SETTINGS ---
    # Termux / MacOS / Modern Linux
    COMPILER_TYPE = Clang
    CFLAGS = -Wall -Wextra -O3 -std=c11 -Rpass=loop-vectorize
else ifneq ($(findstring gcc,$(COMPILER_NAME)),)
    # --- GCC SPECIFIC SETTINGS ---
    # Standard Desktop Linux / Ubuntu
    COMPILER_TYPE = GCC
    CFLAGS = -Wall -Wextra -O3 -std=c11 -fopt-info-vec
else
    # --- FALLBACK SETTINGS ---
    COMPILER_TYPE = Unknown
    CFLAGS = -Wall -O2
endif

# Header/include directories
CFLAGS += -I src \
          -I src/functions \
	-I src/functions/utility \
          -I packages/cJSON \


# ==========================================
# Rest of your standard Makefile continues here
# ==========================================
TARGET =  sunlixDBMS
SOURCES = src/main.c	\
	src/functions/command-line.c	\
	src/functions/crud/writer.c	  \
	src/functions/crud/updater.c	\
	src/functions/utility/key_genrator.c \
  	packages/cJSON/cJSON.c

OBJECTS = $(SOURCES:.c=.o)

.PHONY: all clean info

all: info $(TARGET)

# Prints out a nice status message showing which compiler condition triggeredinfo:
	@echo "--- Building DBMS Engine with [$(COMPILER_TYPE)] ---"

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS)

run:
	./sunlixDBMS

