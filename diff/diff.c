#include <stdlib.h>
#include <string.h>
#include "diff.h"

#define DELTA 5
#define ALPHA 1

static char *loc[] = {
    "\"!@#$%*()_+",
    "'1234567890-=",
    "qwertyuiop[`{",
    "asdfghjklç~]^{",
    "\\|zxcvbnm,.<>;/:"
};

static int coord(char c, int *x, int *y)
{
    char coff = 'A' - 'a'; // Case offset
    for (int i = 0; i < sizeof(loc)/sizeof(*loc); i++) {
        char *line = loc[i];
        for (int j = 0; j < strlen(line); j++) {
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

static int alpha(char a, char b)
{
    if (a == b) return 0;
    int posa[2], posb[2];
    if (coord(a, &posa[0], &posa[1])) return ALPHA;
    if (coord(b, &posb[0], &posb[1])) return ALPHA;
    return abs(posa[0] - posb[0]) + abs(posa[1] - posb[1]);
}

int diff(char *s1, char *s2)
{
    if (!s1 || !s2) return -1;

    int l1 = strlen(s1), l2 = strlen(s2);
    int *M = malloc((l1+1)*sizeof(int));
    if (!M) return -1;
    int *N = malloc((l1+1)*sizeof(int));
    if (!N) {
        free(M);
        return -1;
    }

    for (int i = 0; i <= l1; i++)
        M[i] = i*DELTA;

    int v[3];
    for (int j = 1; j <= l2; j++) {
        for (int i = 0; i <= l1; i++)
            N[i] = M[i];
        M[0] = j*DELTA;
        for (int i = 1; i <= l1; i++) {
            v[0] = alpha(s1[j-1], s2[i-1]) + N[i-1];
            v[1] = DELTA + M[i-1];
            v[2] = DELTA + N[i];
            int lv = v[0];
            for (int k = 1; k < 3; k++)
                if (v[k] < lv)
                    lv = v[k];
            M[i] = lv;
        }
    }

    int cost = M[l1];

    free(M);
    free(N);

    return cost;
}
