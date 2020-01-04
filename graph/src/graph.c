#include <stdlib.h>
#include "graph.h"
#include "set.h"

struct Graph
{
    Set **adjs; /* adjacency sets */
    size_t size; /* number of vertices */
};

static SetReturnID setID;

GraphReturnID graphCreate(struct Graph **ppGraph, size_t size)
{
    struct Graph *pGraph;
    size_t i, j;
    if (ppGraph == NULL || size == 0)
        return GRAPH_RETURN_INVALID_PARAMETER;
    pGraph = (struct Graph *) malloc(sizeof(struct Graph));
    if (pGraph == NULL)
        return GRAPH_RETURN_MEMORY;
    if (size > ((size_t) -1) / sizeof(Set *)) {
        free(pGraph);
        return GRAPH_RETURN_MEMORY;
    }
    pGraph->adjs = (Set **) malloc(sizeof(Set *)*size);
    if (pGraph->adjs == NULL) {
        free(pGraph);
        return GRAPH_RETURN_MEMORY;
    }
    for (i = 0; i < size; ++i) {
        if ((setID = setCreate(&(pGraph->adjs[i]))) != SET_RETURN_OK) {
            for (j = 0; j < i; ++j)
                setDestroy(pGraph->adjs[j]);
            free(pGraph->adjs);
            free(pGraph);
            if (setID == SET_RETURN_MEMORY)
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

GraphReturnID graphAddEdge(struct Graph *pGraph, size_t u, size_t v)
{
    if (pGraph == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    if (u == v)
        return GRAPH_RETURN_SAME_VERTEX;
    if (u >= pGraph->size || v >= pGraph->size)
        return GRAPH_RETURN_OUT_OF_BOUNDS;
    if ((setID = setContains(pGraph->adjs[u], v)) != SET_RETURN_DOES_NOT_CONTAIN) {
        if (setID == SET_RETURN_CONTAINS)
            return GRAPH_RETURN_CONTAINS_EDGE;
        return GRAPH_RETURN_UNKNOWN_ERROR;
    }
    if ((setID = setAdd(pGraph->adjs[u], v)) != SET_RETURN_OK) {
        if (setID == SET_RETURN_MEMORY)
            return GRAPH_RETURN_MEMORY;
        return GRAPH_RETURN_UNKNOWN_ERROR;
    }
    if ((setID = setAdd(pGraph->adjs[v], u)) != SET_RETURN_OK) {
        if (setID == SET_RETURN_MEMORY) {
            if ((setID = setRemove(pGraph->adjs[u], v)) != SET_RETURN_OK)
                return GRAPH_RETURN_FATAL_ERROR;
            return GRAPH_RETURN_MEMORY;
        }
        return GRAPH_RETURN_UNKNOWN_ERROR;
    }
    return GRAPH_RETURN_OK;
}

GraphReturnID graphContainsEdge(struct Graph *pGraph, size_t u, size_t v)
{
    if (pGraph == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    if (u == v)
        return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
    if (u >= pGraph->size || v >= pGraph->size)
        return GRAPH_RETURN_OUT_OF_BOUNDS;
    setID = setContains(pGraph->adjs[u], v);
    switch (setID) {
        case SET_RETURN_CONTAINS:
            return GRAPH_RETURN_CONTAINS_EDGE;
        case SET_RETURN_DOES_NOT_CONTAIN:
            return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
        default:
            return GRAPH_RETURN_UNKNOWN_ERROR;
    }
}

GraphReturnID graphRemoveEdge(struct Graph *pGraph, size_t u, size_t v)
{
    if (pGraph == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    if (u == v)
        return GRAPH_RETURN_SAME_VERTEX;
    if (u >= pGraph->size || v >= pGraph->size)
        return GRAPH_RETURN_OUT_OF_BOUNDS;
    if ((setID = setContains(pGraph->adjs[u], v)) != SET_RETURN_CONTAINS) {
        if (setID == SET_RETURN_DOES_NOT_CONTAIN)
            return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
        return GRAPH_RETURN_UNKNOWN_ERROR;
    }
    if ((setID = setRemove(pGraph->adjs[u], v)) != SET_RETURN_OK)
        return GRAPH_RETURN_UNKNOWN_ERROR;
    if ((setID = setRemove(pGraph->adjs[v], u)) != SET_RETURN_OK) {
        if ((setID = setAdd(pGraph->adjs[u], v)) != SET_RETURN_OK)
            return GRAPH_RETURN_FATAL_ERROR;
        return GRAPH_RETURN_UNKNOWN_ERROR;
    }
    return GRAPH_RETURN_OK;
}

GraphReturnID graphGetNumberOfNeighbours(Graph *pGraph, size_t u, size_t *pNum)
{
    if (pGraph == NULL || pNum == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    if (u >= pGraph->size)
        return GRAPH_RETURN_OUT_OF_BOUNDS;
    setGetSize(pGraph->adjs[u], pNum);
    return GRAPH_RETURN_OK;
}

GraphReturnID graphGetNextNeighbour(Graph *pGraph, size_t u, size_t *pV)
{
    Set *adjList;
    if (pGraph == NULL || pV == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    if (u >= pGraph->size)
        return GRAPH_RETURN_OUT_OF_BOUNDS;
    adjList = pGraph->adjs[u];
    if (setGetCurrent(adjList, pV) == SET_RETURN_EMPTY)
        return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
    if (setNextValue(adjList) == SET_RETURN_OUT_OF_BOUNDS)
        setFirstValue(adjList);
    return GRAPH_RETURN_OK;
}

void graphDestroy(struct Graph *pGraph)
{
    size_t i;
    if (pGraph == NULL)
        return;
    for (i = 0; i < pGraph->size; ++i)
        setDestroy(pGraph->adjs[i]);
    free(pGraph->adjs);
    free(pGraph);
}
