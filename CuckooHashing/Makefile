CC = gcc
CFLAGS = -Wall -g -Werror # -DDEBUG
LDFLAGS = -lm

SOURCES=cuckoo.c
OBJECTS=$(SOURCES:.c=.o)

all: cuckoo

cuckoo: $(OBJECTS)
	gcc $(CFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(OBJECTS) cuckoo
