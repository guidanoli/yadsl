#include <stdio.h>
#include "graph.h"

int free_count = 0;

void free_cb(void *p)
{
    ++free_count;
}

char *test(GraphReturnID *pid, int v)
{
    Graph *g;
    if (*pid = GraphCreate(&g, v, free_cb))
        return "Could not create graph";
    if (*pid = GraphDestroy(g))
        return "Could not destroy graph";
    if (free_count != 0)
        return "Called free_cb even though no vertices were added";
    return NULL;
}

int main(int argc, char **argv)
{
    GraphReturnID id;
    char *str;
    size_t v = 10;
    if (argc >= 2) {
        if (sscanf(argv[1], "%zu", &v) != 1 || v == 0) {
            fprintf(stderr, "Invalid graph size '%s'\n", argv[1]);
            return 1;
        }
    }
    if (str = test(&id, v)) {
        if (id) {
            fprintf(stderr, "Error #%d: %s\n", id, str);
        } else {
            fprintf(stderr, "Error: %s\n", str);
        }
        return 1;
    } else {
        fprintf(stdout, "No errors.\n");
    }
    return 0;
}
