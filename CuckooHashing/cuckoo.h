#ifndef __CUCKOO_H
#define __CUCKOO_H

#include <inttypes.h>

typedef struct CuckooElement {
	void *key;
	uint64_t len;
	void *data;
} CuckooElement;

typedef int (*CuckooDeleteCallback)(CuckooElement *e);

typedef struct CuckooTable {
	struct CuckooElement *table;
	uint64_t size;
	CuckooDeleteCallback deleteCallback;
} CuckooTable;

CuckooTable *cuckooAlloc(uint64_t initialSize, CuckooDeleteCallback deleteCallback);

/*
 * 0 success
 * 1 would have resized
 */
int cuckooInsert(CuckooTable *t, void *key, uint64_t len, void *data);

CuckooElement *cuckooLookup(CuckooTable *t, void *key, uint64_t len);
void *cuckooDelete(CuckooTable *t, void *key, uint64_t len, CuckooDeleteCallback deleteCallback);
void cuckooFree(CuckooTable **t);

#endif /* __CUCKOO_H */
