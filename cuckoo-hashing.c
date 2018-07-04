#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>

#ifdef DEBUG
#define debug(fmt, ...) printf(fmt, __VA_ARGS__)
#else
#define debug(fmt, ...)
#endif

typedef struct Element {
	void *key;
	uint64_t len;
	void *data;
} Element;

typedef struct Table {
	struct Element *table;
	uint64_t size;
} Table;

uint64_t
polyHash(void *key, uint64_t len, uint64_t p, uint64_t tableSize)
{
	uint64_t hash = 0;
	int i;
	for (i = len-1; i >=0; i--) {
		hash = (hash * p + ((uint8_t *)key)[i]) % tableSize;
	}
	return hash;
}

#define PRIME1 3851
#define PRIME2 7477

uint64_t
h1(void *key, uint64_t len, uint64_t tableSize)
{
	return polyHash(key, len, PRIME1, tableSize);
}

uint64_t
h2(void *key, uint64_t len, uint64_t tableSize)
{
	/*
	 * h1(x) -> [0,M-1]
	 * h2(x) = h1(x) + [1,M-2]
	 */
	return (h1(key, len, tableSize) +
			polyHash(key, len, PRIME2, tableSize-2) + 1) % tableSize;
}

typedef uint64_t (*hashFunction)(void *key, uint64_t len, uint64_t tableSize);
hashFunction hashFuncs[2] = {h1, h2};

Table *
cuckooAlloc(uint64_t initialSize)
{
	Table *t;

	t = calloc(1, sizeof(*t));
	t->table = calloc(initialSize, sizeof(*t->table));
	t->size = initialSize;

	return t;
}

/*
 * 0 exact match
 * 1 they differ
 */
int
cuckooCompare(Element *e, void *key, uint64_t len)
{
	if ((e->key != NULL) &&
		(len == e->len) &&
		(memcmp(key, e->key, len) == 0)) {
		return 0;
	}
	return 1;
}

void
cuckooStore(Element *e, void *key, uint64_t len, void *data)
{
	e->key = key;
	e->len = len;
	e->data = data;
}

void
cuckooEvict(Table *t, Element *e, uint64_t c)
{
	Element *other;

	debug("Evicting(%" PRIu64 "): %s\n", c, (char *)e->key);

	if (c >= (uint64_t)log(t->size)) {
		debug("Need to resize the table!\n");
		abort();
	}

	int i;
	for (i = 0; i < 2; i++) {
		other = t->table + hashFuncs[i](e->key, e->len, t->size);
		if (e != other) {
			debug("Other: %s\n", other->key ? (char *)other->key : NULL);
			/*
			 * Move key/value to this spot.
			 */
			if (other->key != NULL) {
				cuckooEvict(t, other, c+1);
			}
			cuckooStore(other, e->key, e->len, e->data);
			cuckooStore(e, NULL, 0, NULL);
			return;
		}
	}

	abort();
}

int
cuckooInsert(Table *t, void *key, uint64_t len, void *data)
{
	Element *e[2];

	/*
	 * See if it's already in the table.
	 */
	int i;
	for (i = 0; i < 2; i++) {
		e[i] = t->table + hashFuncs[i](key, len, t->size);
		if (cuckooCompare(e[i], key, len) == 0) {
			/*
			 * It's already in the table.
			 */
			return 0;
		}
	}

	/*
	 * Check for a free bucket.
	 */
	for (i = 0; i < 2; i++) {
		if (e[i]->key == NULL) {
			debug("Bucket 1 is free\n");
			cuckooStore(e[i], key, len, data);
			return 0;
		}
	}

	/*
	 * Go cuckoo.
	 */
	cuckooEvict(t, e[0], 0);
	cuckooStore(e[0], key, len, data);

	return 0;
}

Element *
cuckooLookup(Table *t, void *key, uint64_t len)
{
	Element *e = NULL;

	int i;
	for (i = 0; i < 2; i++) {
		e = t->table + hashFuncs[i](key, len, t->size);
		if (cuckooCompare(e, key, len) == 0) {
			/*
			 * Found a match.
			 */
			debug("Found in h%d\n", i);
			return e;
		}
	}

	return NULL;
}

int main()
{
	Table *t = cuckooAlloc(7);

	char *strings[] = {
		"scott",
		"kevin",
		"karl",
		"archie",
		"bob"
	};

	int n = sizeof(strings) / sizeof(*strings);
	int i;
	for (i = 0; i < n; i++) {
		printf("Insert: %s\n", strings[i]);
		cuckooInsert(t, strings[i], strlen(strings[i]), NULL);
	}
	printf("\n");

	for (i = 0; i < n; i++) {
		Element *e = cuckooLookup(t, strings[i], strlen(strings[i]));
		if (e == NULL) {
			abort();
		}
		printf("Found: %s\n", (char *)e->key);
	}

	return 0;
}
