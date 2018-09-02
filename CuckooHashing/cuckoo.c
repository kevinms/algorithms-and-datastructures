#include "cuckoo.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>

#ifdef DEBUG
#define debug(...) printf(__VA_ARGS__)
#else
#define debug(...)
#endif

static uint64_t
polyHash(void *key, uint64_t len, uint64_t p, uint64_t M)
{
	uint64_t hash = 0;
	int i;
	for (i = len-1; i >=0; i--) {
		hash = (hash * p + ((uint8_t *)key)[i]) % M;
	}
	return hash;
}

#define PRIME1 3851
#define PRIME2 7477

static uint64_t
h1(void *key, uint64_t len, uint64_t M)
{
	return polyHash(key, len, PRIME1, M);
}

static uint64_t
h2(void *key, uint64_t len, uint64_t M)
{
	/*
	 * h1(x) -> [0,M-1]
	 * h2(x) = h1(x) + [1,M-2]
	 */
	return (h1(key, len, M) + polyHash(key, len, PRIME2, M-2) + 1) % M;
}

typedef uint64_t (*hashFunction)(void *key, uint64_t len, uint64_t tableSize);
hashFunction hashFuncs[2] = {h1, h2};

/*
 * 0 exact match
 * 1 they differ
 */
static inline int
cuckooCompare(CuckooElement *e, void *key, uint64_t len)
{
	if ((e->key != NULL) &&
		(len == e->len) &&
		(memcmp(key, e->key, len) == 0)) {
		return 0;
	}
	return 1;
}

static inline void
cuckooStore(CuckooElement *e, void *key, uint64_t len, void *data)
{
	e->key = key;
	e->len = len;
	e->data = data;
}

/*
 * 0 Success
 * 1 Need to resize table
 */
static int
cuckooEvict(CuckooTable *t, CuckooElement *e, uint64_t c)
{
	CuckooElement *other;

	if (c >= (uint64_t)log2(t->size)) {
		debug("Need to resize the table!\n");
		return 1;
	}

	int i;
	for (i = 0; i < 2; i++) {
		other = t->table + hashFuncs[i](e->key, e->len, t->size);
		if (e != other) {
			/*
			 * Move key/value to this spot.
			 */
			if (other->key != NULL) {
				if (cuckooEvict(t, other, c+1) > 0) {
					return 1;
				}
			}
			cuckooStore(other, e->key, e->len, e->data);
			cuckooStore(e, NULL, 0, NULL);
			return 0;
		}
	}

	/*
	 * If we get here, both hash functions mapped to the same value.
	 * That should never happen!
	 */
	abort();
}

static int
cuckooResize(CuckooTable *t, uint64_t newSize)
{
	if (newSize < 4) {
		debug("The table size must be 2 or greater.\n");
		abort();
	}
	debug("Resize to %" PRIu64 "\n", t->size * 2);

	CuckooElement *oldTable = t->table;
	uint64_t oldSize = t->size;
	t->table = calloc(newSize, sizeof(*t->table));
	t->size = newSize;

	int i;
	for (i = 0; i < oldSize; i++) {
		CuckooElement *e = oldTable + i;
		if (e->key == NULL) {
			continue;
		}

		cuckooInsert(t, e->key, e->len, e->data);
	}

	free(oldTable);

	return 0;
}

CuckooTable *
cuckooAlloc(uint64_t initialSize, CuckooDeleteCallback deleteCallback)
{
	CuckooTable *t;

	if (initialSize < 4) {
		debug("The table size must be 2 or greater.\n");
		abort();
	}

	t = calloc(1, sizeof(*t));
	t->table = calloc(initialSize, sizeof(*t->table));
	t->size = initialSize;
	t->deleteCallback = deleteCallback;

	return t;
}

/*
 * 0 success
 * 1 would have resized
 */
int
cuckooInsert(CuckooTable *t, void *key, uint64_t len, void *data)
{
	CuckooElement *e[2];

RETRY: ;

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
			debug("Bucket %d is free\n", i);
			cuckooStore(e[i], key, len, data);
			return 0;
		}
	}

	/*
	 * Go cuckoo.
	 */
	if (cuckooEvict(t, e[0], 0) > 0) {
		cuckooResize(t, t->size * 2);
		goto RETRY;
	}
	cuckooStore(e[0], key, len, data);

	return 0;
}

CuckooElement *
cuckooLookup(CuckooTable *t, void *key, uint64_t len)
{
	CuckooElement *e = NULL;

	int i;
	for (i = 0; i < 2; i++) {
		e = t->table + hashFuncs[i](key, len, t->size);
		if (cuckooCompare(e, key, len) == 0) {
			/*
			 * Found a match.
			 */
			return e;
		}
	}

	return NULL;
}

void *
cuckooDelete(CuckooTable *t, void *key, uint64_t len, CuckooDeleteCallback deleteCallback)
{
	CuckooElement *e = cuckooLookup(t, key, len);
	void *data = e->data;
	if (deleteCallback) {
		deleteCallback(e);
	}
	cuckooStore(e, NULL, 0, NULL);
	return data;
}

void
cuckooFree(CuckooTable **t)
{
	if (t == NULL) {
		return;
	}

	int i;
	for (i = 0; i < (*t)->size; i++) {
		CuckooElement *e = (*t)->table + i;
		if ((*t)->deleteCallback) {
			(*t)->deleteCallback(e);
		}
		cuckooStore(e, NULL, 0, NULL);
	}

	free((*t)->table);
	free(*t);
	*t = NULL;
}
