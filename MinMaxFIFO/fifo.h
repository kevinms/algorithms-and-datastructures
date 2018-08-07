#ifndef __FIFO_H
#define __FIFO_H

#include "lifo.h"

typedef LifoComparator FifoComparator;

/*
 * Amoritzed constant runtime to determine min and max values in a FIFO.
 */
typedef struct Fifo {
	Lifo *blue;
	Lifo *yellow;

	FifoComparator cmp;
} Fifo;

/*
 * Allocates a Fifo (last in, first out) queue handle.
 *
 * On success, returns a pointer to the newly allocated queue handle.
 * On error, returns NULL.
 */
Fifo *fifoAlloc(FifoComparator cmp);

/*
 * Frees the Fifo handle.
 *
 * Attention:
 * This function does not free the data pointers. If the data pointer
 * needs to be freed, make sure to manually call fifoPop() to empty
 * the Fifo before freeing the handle.
 */
void fifoFree(Fifo **fifo);

/*
 * Pushes 'data' onto the queue.
 *
 * On success, returns 0.
 * On error, returns -1.
 */
int fifoPush(Fifo *fifo, void *data);

/*
 * Removes the most recently pushed 'data' from the queue.
 *
 * On success, returns the removed data or NULL if the queue is empty.
 * On error, returns (void *)-1.
 */
void *fifoPop(Fifo *fifo);

/*
 * Get a pointer to the smallest 'data' item in the Fifo queue.
 * Runtime: O(1) amortized
 *
 * On success, returns a pointer to the smallest 'data' item or NULL if
 * the queue is empty.
 * On error, returns (void *)-1.
 */
void *fifoMin(Fifo *fifo);

/*
 * Get a pointer to the largest 'data' item in the Fifo queue.
 * Runtime: O(1) amortized
 *
 * On success, returns a pointer to the largest 'data' item or NULL if
 * the queue is empty.
 * On error, returns (void *)-1.
 */
void *fifoMax(Fifo *fifo);

#endif /* __FIFO_H */
