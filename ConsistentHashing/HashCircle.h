#ifndef __HASH_CIRCLE_H
#define __HASH_CIRCLE_H

#include <inttypes.h>

typedef struct Node {
	char *name;
	void *data;

	uint64_t weight;

	struct Node *prev;
	struct Node *next;
} Node;

typedef struct Point {
	uint64_t hash;
	uint64_t x;
	Node *node;
} Point;

typedef uint64_t (*hashFunction)(void *key, uint64_t len);

typedef struct Circle {
	/*
	 * Linked list of nodes.
	 */
	Node *nodes;

	/*
	 * Array of points sorted by hash.
	 */
	Point *points;
	uint64_t numPoints;

	hashFunction hashFunc;
} Circle;

/*
 * Allocate a new Circle handle.
 *
 * The 'hashFunc' is the hashing function the Circle will use when
 * hashing points onto the circle.
 *
 * On success, a pointer to the opened Circle is returned.
 * On error, NULL is returned.
 */
Circle *circleAlloc(hashFunction hashFunc);

Node *circleInsert(Circle *c, char *name, void *data, int weight);
Point *circleClosestPoint(Circle *c, char *name);
Point *circleNextPoint(Circle *c, Point *p);

#endif /* __HASH_CIRCLE_H */
