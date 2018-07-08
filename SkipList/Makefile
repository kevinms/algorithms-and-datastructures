CC = gcc
CFLAGS = -Wall -g -Werror # -DDEBUG
LDFLAGS = -lm

SOURCES=skip-list.c
OBJECTS=$(SOURCES:.c=.o)

all: skip-list

skip-list: $(OBJECTS)
	gcc $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJECTS) skip-list
