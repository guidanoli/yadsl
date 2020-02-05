#include "diff.h"

#include <stdlib.h>
#include <string.h>

#define DELTA 5
#define ALPHA 1

static char *loc[] = {
	"\"!@#$%*()_+",
	"'1234567890-=",
	"qwertyuiop[`{",
	"asdfghjklç~]^{",
	"\\|zxcvbnm,.<>;/:"
};

static int coord(char c, size_t *x, size_t *y)
{
	unsigned long i, j;
	char coff = 'A' - 'a'; // Case offset
	for (i = 0; i < sizeof(loc)/sizeof(*loc); i++) {
		char *line = loc[i];
		for (j = 0; j < strlen(line); j++) {
			char ch = line[j];
			if (ch == c || ch == (c + coff)) {
				*x = i;
				*y = j;
				return 0;
			}
		}
	}
	return 1;
}

static unsigned long modsub(size_t a, size_t b)
{
	return (a > b) ? (a - b) : (b - a);
}

static unsigned long alpha(char a, char b)
{
	unsigned long posa[2], posb[2];
	if (a == b) return 0;
	if (coord(a, &posa[0], &posa[1])) return ALPHA;
	if (coord(b, &posb[0], &posb[1])) return ALPHA;
	return modsub(posa[0], posb[0]) + modsub(posa[1], posb[1]);
}

unsigned long diff(const char *s1, const char *s2)
{
	unsigned long v[3], lv, cost, i, j, k, l1, l2, *M, *N;
	if (!s1 || !s2) return -1;
	l1 = strlen(s1);
	l2 = strlen(s2);
	M = malloc((l1 + 1)*sizeof(size_t));
	if (!M) return DIFFERR;
	N = malloc((l1 + 1)*sizeof(size_t));
	if (!N) {
		free(M);
		return DIFFERR;
	}
	for (i = 0; i <= l1; i++)
		M[i] = i*DELTA;
	for (j = 1; j <= l2; j++) {
		for (i = 0; i <= l1; i++)
			N[i] = M[i];
		M[0] = j*DELTA;
		for (i = 1; i <= l1; i++) {
			v[0] = alpha(s1[j-1], s2[i-1]) + N[i-1];
			v[1] = DELTA + M[i-1];
			v[2] = DELTA + N[i];
			lv = v[0];
			for (k = 1; k < 3; k++)
				if (v[k] < lv)
					lv = v[k];
			M[i] = lv;
		}
	}
	cost = M[l1];
	free(M);
	free(N);
	return cost;
}
