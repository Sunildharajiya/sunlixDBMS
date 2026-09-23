CC ?= gcc

COMPILER_NAME := $(shell $(CC) --version 2>&1 | head -n 1)

ifneq ($(findstring clang,$(COMPILER_NAME)),)

    COMPILER_TYPE = Clang

    CFLAGS = -Wall \
             -Wextra \
             -O3 \
             -std=c11 \
             -Rpass=loop-vectorize

else ifneq ($(findstring gcc,$(COMPILER_NAME)),)

    COMPILER_TYPE = GCC

    CFLAGS = -Wall \
             -Wextra \
             -O3 \
             -std=c11 \
             -fopt-info-vec

else

    COMPILER_TYPE = Unknown

    CFLAGS = -Wall \
             -O2

endif

CFLAGS += -I src \
          -I src/functions \
          -I src/functions/utility \
          -I packages/cJSON

TARGET = sunlixDBMS

SOURCES := $(shell find src -type f -name '*.c')

SOURCES += packages/cJSON/cJSON.c

OBJECTS := $(SOURCES:.c=.o)

.PHONY: all clean info run release

all: info $(TARGET)

info:
	@echo "--- Building DBMS Engine with [$(COMPILER_TYPE)] ---"
	@echo "--- Compiler: $(CC) ---"
	@echo "--- Sources: $(words $(SOURCES)) files ---"

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	find . -name "*.o" -delete
	rm -f $(TARGET)
	@echo "--- Build files removed ---"

run: $(TARGET)
	./$(TARGET)

release: $(TARGET)
	rm -f $(OBJECTS)
	@echo "--- Release build complete ---"
	@echo "--- Object files removed ---"