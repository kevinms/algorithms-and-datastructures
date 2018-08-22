#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <assert.h>
#include <math.h>

#include "sort.h"

void fillRandom(int *a, int l)
{
	int i;
	for (i = 0; i < l; i++) {
		a[i] = rand();
	}
}

void checkOrder(int *a, int l)
{
	int i;
	for (i = 0; i < l-1; i++) {
		if (a[i+1] < a[i]) {
			printf("..., %d, %d, ...\n", a[i], a[i+1]);
			abort();
		}
	}
}

typedef void (*SortFunc)(int *a, int l);

int timedSort(SortFunc sort, int *a, int l)
{
	struct timeval start, end;
	gettimeofday(&start, NULL);
	sort(a, l);
	gettimeofday(&end, NULL);
	return (end.tv_sec * 1000000 + end.tv_usec) -
		(start.tv_sec * 1000000 + start.tv_usec);
}

void test(SortFunc sort, char *desc, int powersOfTwo, int runs)
{
	printf("%s:\n", desc);

	int y;
	for (y = 0; y <= powersOfTwo; y++) {
		int l = (int)pow(2, y);

		int *a = malloc(sizeof(*a) * l);
		if (a == NULL) {
			abort();
		}

		int total_usec = 0;
		int i;
		for (i = 0; i < runs; i++) {
			fillRandom(a, l);
			total_usec += timedSort(sort, a, l);
			checkOrder(a, l);
		}

		printf("  2^%-3d items, %4d runs, %.3lf sec/run, %.2lf item/sec\n",
				y, runs, (double)total_usec / runs / 1000000.0,
				(double)l * runs / ((double)total_usec / 1000000.0));

		free(a);
	}
}

int main()
{
	srand(time(NULL));

	test(bubble, "bubble", 12, 10);
	test(selection, "selection", 14, 10);
	test(insertion, "insertion", 14, 10);
	test(quicksort, "quicksort", 20, 10);
	test(bogosort, "bogosort", 13, 10);

	return 0;
}
