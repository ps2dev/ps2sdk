CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Werror

TARGET = test_bd_cache
SOURCE = test_bd_cache.c
BD_CACHE_SOURCE = ../src/bd_cache.c

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SOURCE) $(BD_CACHE_SOURCE)
	$(CC) $(CFLAGS) -Iinclude $(SOURCE) $(BD_CACHE_SOURCE) -o $@

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
