#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "cuckoo.h"

int main()
{
	CuckooTable *t = cuckooAlloc(4, NULL);

	char *strings[] = {
		"scott",
		"kevin",
		"karl",
		"archie",
		"bob"
	};

	int n = sizeof(strings) / sizeof(*strings);
	int i;
	for (i = 0; i < n; i++) {
		printf("Insert: %s\n", strings[i]);
		cuckooInsert(t, strings[i], strlen(strings[i]), NULL);
	}
	printf("\n");

	for (i = 0; i < n; i++) {
		CuckooElement *e = cuckooLookup(t, strings[i], strlen(strings[i]));
		if (e == NULL) {
			printf("Can't find: %s\n", strings[i]);
			abort();
		}
		printf("Found: %s\n", (char *)e->key);
	}

	cuckooFree(&t);

	return 0;
}
