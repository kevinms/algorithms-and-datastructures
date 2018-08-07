#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include "fifo.h"

void
randomArrayOfInts(int **array, int len)
{
	if (array == NULL) {
		abort();
	}
	*array = malloc(len * sizeof(*array));
	if (*array == NULL) {
		abort();
	}

	int i;
	for (i = 0; i < len; i++) {
		(*array)[i] = rand();
	}
}

int
intCompare(void *x, void *y)
{
	if (*(int *)x < *(int *)y) {
		return -1;
	}
	if (*(int *)x > *(int *)y) {
		return 1;
	}
	return 0;
}

void
validateMinMax(Fifo *fifo)
{
	LifoNode *n;

	int *min = fifoMin(fifo);

	/*
	 * Make sure nothing is smaller.
	 */
	for (n = fifo->blue->head; n; n = n->next) {
		assert(intCompare(min, n->data) <= 0);
	}
	for (n = fifo->yellow->head; n; n = n->next) {
		assert(intCompare(min, n->data) <= 0);
	}

	int *max = fifoMax(fifo);

	/*
	 * Make sure nothing is larger.
	 */
	for (n = fifo->blue->head; n; n = n->next) {
		assert(intCompare(max, n->data) >= 0);
	}
	for (n = fifo->yellow->head; n; n = n->next) {
		assert(intCompare(max, n->data) >= 0);
	}
}

int main()
{
	srand(time(NULL));

	int *array = NULL;
	int len = 1000;
	randomArrayOfInts(&array, len);

	Fifo *fifo = fifoAlloc(intCompare);
	if (fifo == NULL) {
		fprintf(stderr, "Can't allocate fifo?!\n");
		return 1;
	}

	int times = 100000;

	int i;
	for (i = 0; i < times; i++) {
		if (rand() > RAND_MAX / 2) {
			int index = rand() % len;
			//printf("Push %d\n", array[index]);
			assert(fifoPush(fifo, array + index) >= 0);
		} else {
			int *data = (int *)fifoPop(fifo);
			assert(data != (void *)-1);
		}

		validateMinMax(fifo);
	}

	fifoFree(&fifo);

	return 0;
}
