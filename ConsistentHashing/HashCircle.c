#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "HashCircle.h"

Circle *
circleAlloc(hashFunction hashFunc)
{
	Circle *c;

	c = calloc(1, sizeof(*c));
	if (c == NULL) {
		fprintf(stderr, "Can't alloc circle: %s\n", strerror(errno));
		return NULL;
	}

	c->hashFunc = hashFunc;

	return c;
}

static uint64_t
indexOfClosestPoint(Circle *c, uint64_t hash, int *exactMatch)
{
	if (exactMatch) {
		*exactMatch = 0;
	}

	if (c->numPoints <= 0) {
		return 0;
	}

	uint64_t min = 0;
	uint64_t max = c->numPoints;

	while (min < max) {
		uint64_t middle = min + (max - min) / 2;

		if (c->points[middle].hash < hash) {
			min = middle + 1;
		} else if (c->points[middle].hash > hash) {
			max = middle;
		} else {
			if (exactMatch) {
				*exactMatch = 1;
			}
			return middle;
		}

		if (middle == min) {
			return middle;
		}
	}

	return min;
}

Node *
circleInsert(Circle *c, char *name, void *data, int weight)
{
	/*
	 * Create a new node.
	 */
	Node *n;

	n = calloc(1, sizeof(*n));
	if (n == NULL) {
		fprintf(stderr, "Can't alloc node: %s\n", strerror(errno));
		return NULL;
	}
	n->name = name;
	n->data = data;
	n->weight = weight;

	/*
	 * Add points to the circle for the new node based on it's weight.
	 */
	uint64_t x;
	for (x = 0; x < n->weight; x++) {
		/*
		 * Create a new point.
		 */
		char *pname = NULL;
		asprintf(&pname, "%s-%" PRIu64, name, x);
		uint64_t hash = c->hashFunc(pname, strlen(pname));
		free(pname);

		int exactMatch;
		uint64_t i = indexOfClosestPoint(c, hash, &exactMatch);
		if (exactMatch) {
			fprintf(stderr, "Point already exists on the circle?!\n");
			free(n);
			return NULL;
		}

		c->points = realloc(c->points, (c->numPoints + 1) * sizeof(*c->points));

		if (i < c->numPoints) {
			uint64_t bytesToMove = (c->numPoints - i) * sizeof(*c->points);
			memmove(c->points + i + 1, c->points + i, bytesToMove);
		}

		c->numPoints++;
		c->points[i].hash = hash;
		c->points[i].x = x;
		c->points[i].node = n;
	}

	/*
	 * Add it to the nodes list.
	 */
	n->next = c->nodes;
	if (c->nodes) {
		c->nodes->prev = n;
	}
	c->nodes = n;

	return n;
}

Point *
circleClosestPoint(Circle *c, char *name)
{
	uint64_t hash = c->hashFunc(name, strlen(name));
	uint64_t i = indexOfClosestPoint(c, hash, NULL);
	if (i == c->numPoints) {
		i = 0;
	}
	return c->points + i;
}

static uint64_t
getPointIndex(Circle *c, Point *p)
{
	if (p < c->points || p >= c->points + c->numPoints) {
		abort();
	}
	uint64_t i = ((uintptr_t)p - (uintptr_t)c->points) / sizeof(*p);
	return i;
}

Point *
circleNextPoint(Circle *c, Point *p)
{
	return c->points + (getPointIndex(c, p) + 1) % c->numPoints;
}
