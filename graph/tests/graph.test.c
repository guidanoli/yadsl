#include <stdio.h>
#include "graph.h"

Graph *g = NULL;

char *test(GraphReturnID *pid, int *nid, size_t size, char **cmds, int count)
{
    size_t pSize, u, v;
    char cmd;
    int i;
    if (*pid = graphCreate(&g, size)) {
        if (*pid == GRAPH_RETURN_MEMORY)
            g = NULL;
        return "Could not create graph";
    }
    if ((*pid = graphGetNumberOfVertices(g, &pSize)) || pSize != size)
        return "Could not determine number of vertices or it is wrong";
    for (i = 0; i < count; ++i) {
        *nid = i+2;
        if (sscanf(cmds[i], "%d,%d%c", &u, &v, &cmd) != 3)
            return "Parsing error";
        switch (cmd) {
            case '+':
                if (*pid = graphAddEdge(g, u, v))
                    return "Could not add edge";
                break;
            case '-':
                if (*pid = graphRemoveEdge(g, u, v))
                    return "Could not remove edge";
                break;
            case '?':
                *pid = graphContainsEdge(g, u, v);
                switch(*pid) {
                    case GRAPH_RETURN_CONTAINS_EDGE:
                        fprintf(stdout, "[%d] Contains (%d,%d)\n", *nid, u, v);
                        break;
                    case GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE:
                        fprintf(stdout, "[%d] Does not contain (%d,%d)\n", *nid, u, v);
                        break;
                    default:
                        return "Could not assert if edge was added or not";
                }
                break;
            default:
                return "Unknown command";
        }
    }
    return NULL;
}

int main(int argc, char **argv)
{
    GraphReturnID id;
    int nid = 1;
    size_t v = 10;
    char **cmds = argc >= 3 ? argv + 2 : NULL;
    char *str;
    if (argc >= 2) {
        if (sscanf(argv[1], "%d", &v) != 1 || v == 0) {
            fprintf(stderr, "Invalid graph size '%s'\n", argv[1]);
            return 1;
        }
    }
    str = test(&id, &nid, v, cmds, argc - 2);
    if (g)
        graphDestroy(g);
    g = NULL;
    if (str) {
        if (id) {
            fprintf(stderr, "Error #%d on %d-th number (%s): %s\n", id, nid, argv[nid], str);
        } else {
            fprintf(stderr, "Error on %d-th number (%s): %s\n", nid, argv[nid], str);
        }
        return 1;
    } else {
        fprintf(stdout, "No errors.\n");
    }
    return 0;
}
