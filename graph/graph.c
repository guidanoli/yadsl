#include <stdlib.h>
#include "graph.h"
#include "set.h"

struct Graph
{
    Set **adjacency_sets;
    size_t number_of_vertices;
};

static SetReturnID lid;

GraphReturnID GraphCreate(struct Graph **ppGraph, size_t number_of_vertices)
{
    if (ppGraph == NULL || number_of_vertices == 0)
        return GRAPH_RETURN_INVALID_PARAMETER;
    struct Graph *pGraph = malloc(sizeof(struct Graph));
    if (pGraph == NULL)
        return GRAPH_RETURN_MEMORY;
    pGraph->adjacency_sets = malloc(sizeof(Set *)*number_of_vertices);
    if (pGraph->adjacency_sets == NULL) {
        free(pGraph);
        return GRAPH_RETURN_MEMORY;
    }
    for (size_t i = 0; i < number_of_vertices; ++i) {
        if ((lid = SetCreate(&(pGraph->adjacency_sets[i]))) != SET_RETURN_OK) {
            for (size_t j = 0; j < i; ++j)
                SetDestroy(pGraph->adjacency_sets[j]);
            free(pGraph->adjacency_sets);
            free(pGraph);
            if (lid == SET_RETURN_MEMORY)
                return GRAPH_RETURN_MEMORY;
            return GRAPH_RETURN_UNKNOWN_ERROR;
        }
    }
    pGraph->number_of_vertices = number_of_vertices;
    *ppGraph = pGraph;
    return GRAPH_RETURN_OK;
}

GraphReturnID GraphGetNumberOfVertices(struct Graph *pGraph, size_t *pNumber)
{
    if (pGraph == NULL || pNumber == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    *pNumber = pGraph->number_of_vertices;
    return GRAPH_RETURN_OK;
}

void GraphDestroy(struct Graph *pGraph)
{
    if (pGraph == NULL)
        return;
    for (size_t i = 0; i < pGraph->number_of_vertices; ++i)
        SetDestroy(pGraph->adjacency_sets[i]);
    free(pGraph->adjacency_sets);
    free(pGraph);
}
