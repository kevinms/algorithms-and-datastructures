#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "sort.h"

#define SWAP(x,y) (x)^=(y);(y)^=(x);(x)^=(y)

void bubble(int *a, int l)
{
	int i, swap;
	do {
		swap = 0;
		for (i = 0; i < l-1; i++) {
			if (a[i+1] <= a[i]) {
				SWAP(a[i], a[i+1]);
				swap = 1;
			}
		}
	} while (swap);
}

void selection(int *a, int l)
{
	int i;
	for (i = 0; i < l; i++) {
		int j, small = i;
		for (j = i+1; j < l; j++) {
			if (a[j] < a[small]) {
				a[small] = a[j];
			}
		}
		SWAP(a[i], a[small]);
	}
}

void insertion(int *a, int l)
{
	int i, j;
	for (i = 1; i < l; i++) {
		for (j = i; j > 0; j--) {
			if (a[j] < a[j-1]) {
				SWAP(a[j], a[j-1]);
			} else {
				break;
			}
		}
	}
}

/*
 * Modified hoare scheme where a random pivot is chosen.
 */
void quicksort(int *a, int l)
{
	if (l <= 1) return;

	int pivot = a[rand() % l];

	int i = -1;
	int j = l;
	for (;;) {
		do { i++; } while (a[i] < pivot);
		do { j--; } while (a[j] > pivot);
		if (i >= j) break;
		SWAP(a[i], a[j]);
	}
	
	quicksort(a, j+1);
	quicksort(a + j+1, l - (j+1));
}

static void fisher_yates_shuffle(int *a, int l)
{
	int i;
	for (i = l-1; i > 0; i--) {
		int j = rand() % (i+1);
		SWAP(a[i], a[j]);
	}
}

void bogosort(int *a, int l)
{
	int i;
	while (i < l-1) {
		for (i = 0; i < l-1; i++) {
			if (a[i+1] < a[i]) {
				fisher_yates_shuffle(a, l);
				break;
			}
		}
	}
}
