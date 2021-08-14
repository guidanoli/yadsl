#include <diff/diff.h>

#include <assert.h>
#include <string.h>
#include <stddef.h>
#include <math.h>

#include <yadsl/stdlib.h>

#if defined(_MSC_VER)
# pragma warning(disable : 6386)
# pragma warning(disable : 6385)
# pragma warning(disable : 4244)
#endif

#define DELTA (5.0)
#define ALPHA (1.0)

static int charcoord(char c, size_t* x, size_t* y)
{
	static const char* loc[] = {
		"\"!@#$%*()_+",
		"'1234567890-=",
		"qwertyuiop[`{",
		"asdfghjkl~]^{",
		"\\|zxcvbnm,.<>;/:"
	};
	size_t i, j;
	char coff = 'A' - 'a'; // Case offset
	for (i = 0; i < sizeof(loc) / sizeof(*loc); i++) {
		const char* line = loc[i];
		for (j = 0; j < strlen(line); j++) {
			char ch = line[j];
			if (ch == c || (ch >= 'a' && ch <= 'z' && ch == (c + coff))) {
				*x = i;
				*y = j;
				return 0;
			}
		}
	}
	return 1;
}

static size_t modsub(size_t a, size_t b)
{
	return (a > b) ? (a - b) : (b - a);
}

static double alpha(char a, char b)
{
	size_t posa[2], posb[2], di, dj;
	if (a == b)
		return 0.0;
	if (charcoord(a, &posa[0], &posa[1]))
		return ALPHA;
	if (charcoord(b, &posb[0], &posb[1]))
		return ALPHA;
	di = modsub(posa[0], posb[0]);
	dj = modsub(posa[1], posb[1]);
	return sqrt(di * di + dj * dj);
}

/**** External functions definitions ****/

double yadsl_utils_diff(const char* a, size_t alen, const char* b, size_t blen)
{
	size_t i, j, k;
	double v[3], lv, cost = -1.0, * M, * N;
	assert(a != NULL);
	assert(b != NULL);
	M = calloc(alen + 1, sizeof(double));
	N = calloc(alen + 1, sizeof(double));
	if (M == NULL || N == NULL) goto fail;
	for (i = 0; i <= alen; i++)
		M[i] = i * DELTA;
	for (j = 1; j <= blen; j++) {
		for (i = 0; i <= alen; i++)
			N[i] = M[i];
		M[0] = j * DELTA;
		for (i = 1; i <= alen; i++) {
			double ai = a[i-1];
			double bj = b[j-1];
			double Ni = N[i-1];
			double Mi = M[i-1];
			v[0] = alpha(ai, bj) + Ni;
			v[1] = DELTA + Mi;
			v[2] = DELTA + Ni;
			lv = v[0];
			for (k = 1; k < 3; k++)
				if (v[k] < lv)
					lv = v[k];
			M[i] = lv;
		}
	}
	cost = M[alen];
fail:
	free(M);
	free(N);
	return cost;
}
