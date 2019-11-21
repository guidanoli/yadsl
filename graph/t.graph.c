#include <stdio.h>
#include "graph.h"

int free_count = 0;

void free_cb(void *p)
{
    ++free_count;
}

char *test(GraphReturnID *pid)
{
    Graph *g;
    const int v = 10;
    if (*pid = GraphCreate(&g, v, free_cb))
        return "Could not create graph";
    if (*pid = GraphDestroy(g))
        return "Could not destroy graph";
    if (free_count != 0)
        return "Called free_cb even though no vertices were added";
    return NULL;
}

int main(void)
{
    GraphReturnID id;
    char *str;
    if (str = test(&id)) {
        if (id) {
            fprintf(stderr, "Error #%d: %s\n", id, str);
        } else {
            fprintf(stderr, "Error: %s\n", str);
        }
    } else {
        fprintf(stdout, "No errors.\n");
    }
    return 0;
}
