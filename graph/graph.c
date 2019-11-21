#include <stdlib.h>
#include "graph.h"
#include "set.h"

struct Graph
{
    Set **adjs; /* adjacency sets */
    size_t size; /* number of vertices */
};

static SetReturnID lid;

GraphReturnID graphCreate(struct Graph **ppGraph, size_t size)
{
    if (ppGraph == NULL || size == 0)
        return GRAPH_RETURN_INVALID_PARAMETER;
    struct Graph *pGraph = malloc(sizeof(struct Graph));
    if (pGraph == NULL)
        return GRAPH_RETURN_MEMORY;
    pGraph->adjs = malloc(sizeof(Set *)*size);
    if (pGraph->adjs == NULL) {
        free(pGraph);
        return GRAPH_RETURN_MEMORY;
    }
    for (size_t i = 0; i < size; ++i) {
        if ((lid = setCreate(&(pGraph->adjs[i]))) != SET_RETURN_OK) {
            for (size_t j = 0; j < i; ++j)
                setDestroy(pGraph->adjs[j]);
            free(pGraph->adjs);
            free(pGraph);
            if (lid == SET_RETURN_MEMORY)
                return GRAPH_RETURN_MEMORY;
            return GRAPH_RETURN_UNKNOWN_ERROR;
        }
    }
    pGraph->size = size;
    *ppGraph = pGraph;
    return GRAPH_RETURN_OK;
}

GraphReturnID graphGetNumberOfVertices(struct Graph *pGraph, size_t *pSize)
{
    if (pGraph == NULL || pSize == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    *pSize = pGraph->size;
    return GRAPH_RETURN_OK;
}

void graphDestroy(struct Graph *pGraph)
{
    if (pGraph == NULL)
        return;
    for (size_t i = 0; i < pGraph->size; ++i)
        setDestroy(pGraph->adjs[i]);
    free(pGraph->adjs);
    free(pGraph);
}
