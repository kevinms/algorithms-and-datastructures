#include "HashCircle.h"

#include <stdio.h>
#include <assert.h>

#define PRIME1 3851

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

static uint64_t
myHash(void *key, uint64_t len)
{
	return polyHash(key, len, PRIME1, UINT64_MAX);
}

int main()
{
	Circle *c = circleAlloc(myHash);

	char *keys[] = {
		"key0",
		"key3",
		"key1",
		"key2"
	};
	int numKeys = sizeof(keys) / sizeof(*keys);

	int i;
	for (i = 0; i < numKeys; i++) {
		printf("Inserting %s\n", keys[i]);
		Node * n = circleInsert(c, keys[i], keys[i], 2);
		assert(n != NULL && n->name == keys[i]);
	}

	for (i = 0; i < c->numPoints; i++) {
		printf("Point %i: %" PRIu64 " -> %s-%" PRIu64 "\n",
				i, c->points[i].hash, c->points[i].node->name, c->points[i].x);
	}	

	for (i = 0; i < numKeys; i++) {
		Point *cp = circleClosestPoint(c, keys[i]);
		printf("%s is closest to %s-%" PRIu64"\n", keys[i], cp->node->name, cp->x);

		Point *p = cp;
		while ((p = circleNextPoint(c, p)) != cp) {
			printf("Next closest %s-%" PRIu64 "\n", p->node->name, p->x);
		}
	}

	return 0;
}
