#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "skiplist.h"
	
void
skipValidate(SkipList *l)
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
skipPrint(SkipList *l)
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

int
cmpInt(void *x, void *y)
{
	return *(int *)x - *(int *)y;
}

int main()
{
	srand48(time(NULL));
	srand(time(NULL));

	SkipList *l = skipAlloc(cmpInt);

	int i, N = 1000;
	int *array = malloc(sizeof(*array) * N);
	for (i = 0; i < N; i++) {
		array[i] = rand() % N;
		skipInsert(l, array+i);
	}
	skipValidate(l);

	for (i = 0; i < N; i++) {
		if (skipFind(l, array+i) == NULL) {
			abort();
		}
	}

#ifdef DEBUG
	for (i = 0; i < N; i++) {
		fprintf(stderr, "%d, ", array[i]);
	}
	fprintf(stderr, "\n");

	skipPrint(l);
#endif

	for (i = 0; i < N; i++) {
		SkipNode *n;
		if ((n = skipFind(l, array+i)) == NULL) {
			//abort();
			continue;
		}
		//skipDelete(l, n, NULL, NULL);
	}

	SkipNode *n = NULL;
	while (skipIterate(l, &n) > 0) {
		fprintf(stderr, "%d, ", *(int *)n->data);
	}
	fprintf(stderr, "\n");

	skipFree(&l, NULL, NULL);

	return 0;
}
