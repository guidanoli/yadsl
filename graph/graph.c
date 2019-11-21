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

GraphReturnID GraphCreate(struct Graph **ppGraph, size_t number_of_vertices,
                                            void (*delete_node_info)(void *))
{
    if (ppGraph == NULL || number_of_vertices == 0 || delete_node_info == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    struct Graph *pGraph = malloc(sizeof(struct Graph));
    if (pGraph == NULL)
        return GRAPH_RETURN_MEMORY;
    pGraph->adjacency_lists = malloc(sizeof(struct AdjList)*number_of_vertices);
    if (pGraph->adjacency_lists == NULL) {
        free(pGraph);
        return GRAPH_RETURN_MEMORY;
    }
    for (size_t i = 0; i < number_of_vertices; ++i) {
        pGraph->adjacency_lists[i].current = NULL;
        pGraph->adjacency_lists[i].first = NULL;
        pGraph->adjacency_lists[i].last = NULL;
    }
    pGraph->number_of_vertices = number_of_vertices;
    pGraph->delete_node_info = delete_node_info;
    *ppGraph = pGraph;
    return GRAPH_RETURN_OK;
}

GraphReturnID GraphDestroy(struct Graph *pGraph)
{
    if (pGraph == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    for (size_t i = 0; i < pGraph->number_of_vertices; ++i) {
        struct AdjList list = pGraph->adjacency_lists[i];
        struct AdjListNode *current = list.first;
        struct AdjListNode *next = NULL;
        while (current != NULL) {
            next = current->next;
            pGraph->delete_node_info(current->info);
            free(current);
            current = next;
        }
    }
    free(pGraph->adjacency_lists);
    free(pGraph);
    return GRAPH_RETURN_OK;
}
