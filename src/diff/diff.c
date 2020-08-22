#include <diff/diff.h>

#include <aa/posixstring.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>

#include <memdb/memdb.h>

#if defined(_MSC_VER)
# pragma warning(disable : 6386)
# pragma warning(disable : 6385)
# pragma warning(disable : 4244)
#endif

#define DELTA (5.0)
#define ALPHA (1.0)

static int aa_utils_diff_coord_internal(char c, size_t *x, size_t *y)
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
		const char *line = loc[i];
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

static size_t aa_utils_diff_modsub_internal(size_t a, size_t b)
{
	return (a > b) ? (a - b) : (b - a);
}

static double aa_utils_diff_alpha_internal(char a, char b)
{
	size_t posa[2], posb[2], di, dj;
	if (a == b)
		return 0.0;
	if (aa_utils_diff_coord_internal(a, &posa[0], &posa[1]))
		return ALPHA;
	if (aa_utils_diff_coord_internal(b, &posb[0], &posb[1]))
		return ALPHA;
	di = aa_utils_diff_modsub_internal(posa[0], posb[0]);
	dj = aa_utils_diff_modsub_internal(posa[1], posb[1]);
	return sqrt(di*di + dj*dj);
}

/**** External functions definitions ****/

double aa_utils_diff(const char *s1, const char *s2)
{
	size_t l1, l2, i, j, k;
	double v[3], lv, cost = -1.0, *M, *N;
	if (!s1 || !s2)
		goto fail0;
	l1 = strlen(s1) + 1;
	l2 = strlen(s2) + 1;
	if (l1 == 1 && l2 == 1)
		return 0.0;
	M = malloc(l1 * sizeof(double));
	if (!M)
		goto fail0;
	N = malloc(l1 * sizeof(double));
	if (!N)
		goto fail1;
	for (i = 0; i < l1; i++)
		M[i] = i * DELTA;
	for (j = 1; j < l2; j++) {
		for (i = 0; i < l1; i++)
			N[i] = M[i];
		M[0] = j * DELTA;
		for (i = 1; i < l1; i++) {
			v[0] = aa_utils_diff_alpha_internal(s1[j - 1], s2[i - 1]) + N[i - 1];
			v[1] = DELTA + M[i - 1];
			v[2] = DELTA + N[i];
			lv = v[0];
			for (k = 1; k < 3; k++)
				if (v[k] < lv)
					lv = v[k];
			M[i] = lv;
		}
	}
	cost = M[l1 - 1];
	free(M);
fail1:
	free(N);
fail0:
	return cost;
}
