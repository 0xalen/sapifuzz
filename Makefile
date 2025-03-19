CC = gcc

CFLAGS = -Wall -Wextra -g

LDFLAGS = -lcurl

TARGET = fuzzapi

SOURCES = main.c fuzzer.c

OBJECTS = $(SOURCES:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)

.PHONY: all clean
