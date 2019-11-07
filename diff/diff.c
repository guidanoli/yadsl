#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "diff.h"

#define DELTA 5
#define ALPHA 1

static char *loc[3] = {
    "qwertyuiop",
    "asdfghjkl",
    "zxcvbnm"
};

int abs(int x) {
    return x < 0 ? -x : x;
}

int coord(char c, int *x, int *y)
{
    char coff = 'A' - 'a'; // Case offset
    for (int i = 0; i < 3; i++) {
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

int alpha(char a, char b)
{
    if (a == b) return 0;
    int posa[2], posb[2];
    if (coord(a, &posa[0], &posa[1])) return 1;
    if (coord(b, &posb[0], &posb[1])) return 1;
    return abs(posa[0] - posb[0]) + abs(posa[1] - posb[1]);
}

int diff(char *s1, char *s2)
{
    if (!s1 || !s2) return 1;

    int l1 = strlen(s1), l2 = strlen(s2);
    int **M = malloc((l1+1)*sizeof(int*));
    if (!M) return 1;

    for (int i = 0; i <= l1; i++) {
        M[i] = malloc((l2+1)*sizeof(int));
        if (!M[i]) return 1;
    }

    for (int i = 0; i <= l1; i++)
        M[i][0] = i*DELTA;

    for (int j = 0; j <= l2; j++)
        M[0][j] = j*DELTA;

    int v[3];
    for (int i = 1; i <= l1; i++) {
        for (int j = 1; j <= l2; j++) {
            v[0] = alpha(s1[i-1], s2[j-1]) + M[i-1][j-1];
            v[1] = DELTA + M[i-1][j];
            v[2] = DELTA + M[i][j-1];
            int lv = v[0];
            for (int k = 1; k < 3; k++)
                if (v[k] < lv)
                    lv = v[k];
            M[i][j] = lv;
        }
    }

    printf("Cost = %d\n", M[l1-1][l2-1]);

    free(M);
    return 0;
}
