#include <stdio.h>
#include "graph.h"

Graph *g = NULL;

char *test(GraphReturnID *pid, size_t v)
{
    size_t size;
    if (*pid = GraphCreate(&g, v))
        return "Could not create graph";
    if ((*pid = GraphGetNumberOfVertices(g, &size)) || size != v)
        return "Could not determine number of vertices or it is wrong";
    return NULL;
}

int main(int argc, char **argv)
{
    GraphReturnID id;
    size_t v = 10;
    if (argc >= 2) {
        if (sscanf(argv[1], "%zu", &v) != 1 || v == 0) {
            fprintf(stderr, "Invalid graph size '%s'\n", argv[1]);
            return 1;
        }
    }
    char *str = test(&id, v);
    GraphDestroy(g);
    g = NULL;
    if (str) {
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
