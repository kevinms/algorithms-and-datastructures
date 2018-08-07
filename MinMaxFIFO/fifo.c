#include "fifo.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

Fifo *
fifoAlloc(FifoComparator cmp)
{
	Fifo *fifo;

	fifo = malloc(sizeof(*fifo));
	if (fifo == NULL) {
		fprintf(stderr, "Can't allocate Fifo structure: %s\n", strerror(errno));
		goto ERROR;
	}
	memset(fifo, 0, sizeof(*fifo));
	fifo->cmp = cmp;

	fifo->blue = lifoAlloc(cmp);
	if (fifo->blue == NULL) {
		fprintf(stderr, "Can't allocate blue lifo?!\n");
		goto ERROR;
	}
	fifo->yellow = lifoAlloc(cmp);
	if (fifo->yellow == NULL) {
		fprintf(stderr, "Can't allocate yellow lifo?!\n");
		goto ERROR;
	}

	return fifo;

ERROR:

	if (fifo) {
		free(fifo->blue);
		free(fifo->yellow);
	}

	return NULL;
}

void
fifoFree(Fifo **fifo)
{
	if (fifo == NULL || *fifo == NULL) {
		return;
	}

	lifoFree(&(*fifo)->blue);
	lifoFree(&(*fifo)->yellow);

	free(*fifo);
	*fifo = NULL;
}

int
fifoPush(Fifo *fifo, void *data)
{
	if (fifo == NULL || data == NULL) {
		fprintf(stderr, "%s(%p,%p): Invalid arguments?!\n",
				__func__, fifo, data);
		return -1;
	}

	return lifoPush(fifo->blue, data);
}

void *
fifoPop(Fifo *fifo)
{
	if (fifo == NULL) {
		fprintf(stderr, "%s(%p): Invalid arguments?!\n", __func__, fifo);
		return (void *)-1;
	}

	/*
	 * If anything is in yellow, pop from it.
	 */
	if (fifo->yellow->head) {
		return lifoPop(fifo->yellow);
	}

	/*
	 * Move everything from blue to yellow.
	 */
	void *data;
	while ((data = lifoPop(fifo->blue)) != NULL) {
		lifoPush(fifo->yellow, data);
	}

	if (fifo->yellow->head) {
		return lifoPop(fifo->yellow);
	}

	return NULL;
}

void *
fifoMin(Fifo *fifo)
{
	if (fifo == NULL) {
		fprintf(stderr, "%s(%p): Invalid arguments?!\n", __func__, fifo);
		return (void *)-1;
	}

	void *min0 = lifoMin(fifo->blue);
	void *min1 = lifoMin(fifo->yellow);

	/*
	 * Handle if one or none are NULL.
	 */
	if (!min0 || !min1) {
		return min0 ? min0 : min1;
	}

	/*
	 * Which is smaller?
	 */
	if (fifo->cmp(min0, min1) < 0) {
		return min0;
	}
	return min1;
}

void *
fifoMax(Fifo *fifo)
{
	if (fifo == NULL) {
		fprintf(stderr, "%s(%p): Invalid arguments?!\n", __func__, fifo);
		return (void *)-1;
	}

	void *max0 = lifoMax(fifo->blue);
	void *max1 = lifoMax(fifo->yellow);

	/*
	 * Handle if one or none are NULL.
	 */
	if (!max0 || !max1) {
		return max0 ? max0 : max1;
	}

	/*
	 * Which is smaller?
	 */
	if (fifo->cmp(max0, max1) > 0) {
		return max0;
	}
	return max1;
}
