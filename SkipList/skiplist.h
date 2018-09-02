#ifndef __SKIPLIST_H
#define __SKIPLIST_H

typedef int (*SkipListComparator)(void *a, void *b);
typedef int (*SkipListDeleteCallback)(void *data, void *user);

typedef struct SkipNode {
	struct SkipNode *up, *down;
	struct SkipNode *prev, *next;
	void *data;
} SkipNode;

typedef struct SkipList {
	SkipNode *start;
	SkipListComparator cmp;
} SkipList;

SkipList *skipAlloc(SkipListComparator cmp);

/*
 * NULL is returned if the SkipList is empty.
 * Otherwise, a pointer to the closest SkipNode less than or equal to data is returned.
 * If it is equal to, exactMatch is set to 1.
 */
SkipNode *skipFindClosest(SkipList *l, void *data, int *exactMatch);

SkipNode *skipInsert(SkipList *l, void *data);
int skipDelete(SkipList *l, SkipNode *n, SkipListDeleteCallback callback, void *user);
void skipFree(SkipList **l, SkipListDeleteCallback callback, void *user);
SkipNode *skipFind(SkipList *l, void *data);

/*
 * The input SkipNode 'n' keeps track of the iterator's place in the SkipList.
 * The first call to the iterator (*n) should equal NULL.
 * On success, 1 is returned and 'n' will point to the next SkipNode. Or, 0 is
 *   returned and 'n' will be NULL which means it has reached the end of the SkipList.
 * On error, -1 is returned.
 */
int skipIterate(SkipList *l, SkipNode **n);

#endif /* __SKIPLIST_H */
