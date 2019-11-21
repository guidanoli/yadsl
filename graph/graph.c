#include <stdlib.h>
#include "graph.h"

struct AdjListNode
{
    struct AdjListNode *next;
    void *info;
};

struct AdjList
{
    struct AdjListNode *current;
    struct AdjListNode *first;
    struct AdjListNode *last;
};

struct Graph
{
    void (*delete_node_info)(void *);
    struct AdjList *adjacency_lists;
    size_t number_of_vertices;
};
