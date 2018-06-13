#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <errno.h>

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

void
skip_validate(SkipList *l)
{
	int i;
	SkipNode *h;
	for (h = l->start, i = 0; h != NULL; h = h->down, i++) {
		SkipNode *p = NULL;
		SkipNode *n;
		for (n = h; n != NULL; p = n, n = n->next) {
			/*
			 * Ensure horizontal sorted order.
			 */
			if (p && n->data == NULL) {
				abort();
			}
			if (p && p->data && l->cmp(p->data, n->data) >= 0) {
				abort();
			}

			/*
			 * Ensure vertical nodes match.
			 */
			if (n->up && n->up->data != n->data) {
				abort();
			}
			if (n->down && n->down->data != n->data) {
				abort();
			}
		}
	}
}

void
skip_print(SkipList *l)
{
	int i;
	SkipNode *h;
	for (h = l->start, i = 0; h != NULL; h = h->down, i++) {
		fprintf(stderr, "lvl %d -> ", i);
		SkipNode *n;
		for (n = h; n != NULL; n = n->next) {
			if (n->data) {
				fprintf(stderr, "%d, ", *(int *)n->data);
			}
		}
		fprintf(stderr, "\n");
	}
}

SkipList *
skip_alloc(SkipListComparator cmp)
{
	SkipList *l;
	
	if ((l = calloc(1, sizeof(*l))) == NULL) {
		fprintf(stderr, "Can't allocate skip list structure: %s\n", strerror(errno));
		return NULL;
	}
	l->cmp = cmp;

	return l;
}

/*
 * NULL is returned if the SkipList is empty.
 * Otherwise, a pointer to the closest SkipNode less than or equal to data is returned.
 * If it is equal to, exactMatch is set to 1.
 */
SkipNode *
skip_find_closest(SkipList *l, void *data, int *exactMatch)
{
	*exactMatch = 0;
	if (l->start == NULL) {
		return NULL;
	}

	SkipNode *n = l->start;

	/*
	 * Go through all levels of the tree.
	 */
	while (n != NULL) {
		/*
		 * Find closest in the current level.
		 */
		while (!*exactMatch && n->next != NULL) {
			int c = l->cmp(n->next->data, data);
			if (c < 0) {
				n = n->next;
			} else if (c == 0) {
				*exactMatch = 1;
				n = n->next;
				break;
			} else {
				break;
			}
		}

		/*
		 * n is the closest SkipNode to our data in the current level.
		 * Now go down a level, if we can.
		 */
		if (n->down == NULL) {
			break;
		}
		n = n->down;
	}

	return n;
}

SkipNode *
skip_insert(SkipList *l, void *data)
{
	if (l == NULL || data == NULL) {
		fprintf(stderr, "%s(%p, %p): Invalid arguments?!\n", __func__, l, data);
		return NULL;
	}

	SkipNode *new;
	if ((new = calloc(1, sizeof(*new))) == NULL) {
		fprintf(stderr, "Can't allocate node structure: %s\n", strerror(errno));
		return NULL;
	}
	new->data = data;

	/*
	 * Insert into the bottom level of tree.
	 */
	int exactMatch = 0;
	SkipNode *n = skip_find_closest(l, data, &exactMatch);
	if (n == NULL) {
		/*
		 * The skip list is empty!
		 */
		SkipNode *newHead;
		if ((newHead = calloc(1, sizeof(*newHead))) == NULL) {
			fprintf(stderr, "Can't allocate head node structure: %s\n", strerror(errno));
			goto error;
		}
		new->prev = newHead;
		newHead->next = new;
		l->start = newHead;
	} else if (exactMatch) {
		/*
		 * Data already exists in the list -- nothing to do.
		 */
		free(new);
		return n;
	} else {
		/*
		 * Insert after this node.
		 */
		new->next = n->next;
		new->prev = n;
		if (n->next) {
			n->next->prev = new;
		}
		n->next = new;
	}
	n = new;

	/*
	 * See how many times we can promote up.
	 */
	while (drand48() < 0.5) {
		/*
		 * We won the coin toss.
		 */

		/*
		 * Search backwards for a way to go up.
		 */
		SkipNode * p;
		for (p = n; p->up == NULL && p->prev != NULL; p = p->prev);

		if (p->up == NULL) {
			/*
			 * Couldn't find a way to go up -- make a new level!
			 */
			SkipNode *newHead;
			if ((newHead = calloc(1, sizeof(*newHead))) == NULL) {
				fprintf(stderr, "Can't allocate head node structure: %s\n", strerror(errno));
				goto error;
			}

			newHead->down = p;
			p->up = newHead;
			l->start = newHead;
		}
		p = p->up;

		SkipNode *intr;
		if ((intr = calloc(1, sizeof(*intr))) == NULL) {
			fprintf(stderr, "Can't allocate intermediate node structure: %s\n", strerror(errno));
			goto error;
		}
		intr->data = data;
		intr->down = n;
		intr->prev = p;
		intr->next = p->next;
		if (p->next) {
			p->next->prev = intr;
		}
		p->next = intr;
		n->up = intr;

		n = intr;
	}

	return new;

error:

	free(new);
	return NULL;
}

int
skip_delete(SkipList *l, SkipNode *n, SkipListDeleteCallback callback, void *user)
{
	if (callback) {
		callback(n->data, user);
	}

	/*
	 * Unlink everything vertically.
	 */
	while (n) {
		if (n->prev) {
			n->prev->next = n->next;
		}
		if (n->next) {
			n->next->prev = n->prev;
		}

		SkipNode *p = n;
		n = n->up;
		free(p);
	}

	/*
	 * Remove any empty top layers.
	 */
	while (l->start) {
		if (l->start->next != NULL) {
			break;
		}

		SkipNode *p = l->start;
		l->start = l->start->down;
		free(p);
	}

	return 0;
}

void
skip_free(SkipList **l, SkipListDeleteCallback callback, void *user)
{
	if (l == NULL || *l == NULL) {
		return;
	}

	/*
	 * Walk to the bottom.
	 */
	SkipNode *head;
	for (head = (*l)->start; head && head->down != NULL; head = head->down);

	while ((*l)->start) {
		skip_delete(*l, head->next, callback, user);
	}

	free(*l);
	*l = NULL;
}

SkipNode *
skip_find(SkipList *l, void *data)
{
	int exactMatch = 0;
	SkipNode *n = skip_find_closest(l, data, &exactMatch);

	if (exactMatch) {
		return n;
	}
	return NULL;
}

/*
 * The input SkipNode 'n' keeps track of the iterator's place in the SkipList.
 * The first call to the iterator (*n) should equal NULL.
 * On success, 1 is returned and 'n' will point to the next SkipNode. Or, 0 is
 *   returned and 'n' will be NULL which means it has reached the end of the SkipList.
 * On error, -1 is returned.
 */
int
skip_iterate(SkipList *l, SkipNode **n)
{
	if (l == NULL || n == NULL) {
		fprintf(stderr, "%s(%p,%p): Invalid arguments?!\n", __func__, l, n);
		return -1;
	}

	if (*n == NULL) {
		/*
		 * Walk to the bottom.
		 */
		for (*n = l->start; *n && (*n)->down != NULL; *n = (*n)->down);
	}

	/*
	 * Advance to the next SkipNode.
	 */
	*n = (*n)->next;

	if (*n == NULL) {
		return 0;
	}
	return 1;
}

int
cmp_int(void *x, void *y)
{
	return *(int *)x - *(int *)y;
}

int main()
{
	srand48(time(NULL));
	srand(time(NULL));

	SkipList *l = skip_alloc(cmp_int);

	int i, N = 1000;
	int *array = malloc(sizeof(*array) * N);
	for (i = 0; i < N; i++) {
		array[i] = rand() % N;
		skip_insert(l, array+i);
	}
	skip_validate(l);

	for (i = 0; i < N; i++) {
		if (skip_find(l, array+i) == NULL) {
			abort();
		}
	}

#ifdef DEBUG
	for (i = 0; i < N; i++) {
		fprintf(stderr, "%d, ", array[i]);
	}
	fprintf(stderr, "\n");

	skip_print(l);
#endif

	for (i = 0; i < N; i++) {
		SkipNode *n;
		if ((n = skip_find(l, array+i)) == NULL) {
			//abort();
			continue;
		}
		//skip_delete(l, n, NULL, NULL);
	}

	SkipNode *n = NULL;
	while (skip_iterate(l, &n) > 0) {
		fprintf(stderr, "%d, ", *(int *)n->data);
	}
	fprintf(stderr, "\n");

	skip_free(&l, NULL, NULL);

	return 0;
}
