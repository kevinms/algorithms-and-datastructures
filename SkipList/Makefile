CC = gcc
CFLAGS = -Wall -g -Werror # -DDEBUG
LDFLAGS = -lm

BINARY=test
SOURCES=skiplist.c test.c
OBJECTS=$(SOURCES:.c=.o)

all: $(BINARY)

$(BINARY): $(OBJECTS)
	gcc $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJECTS) $(BINARY)
