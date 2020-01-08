#include <stdlib.h>
#include <stdio.h>
#include "graph.h"
#include "set.h"

#define SERIALIZED_FORMAT_VERSION (1)
#define WRITE(file, format, ...) do {                           \
    if (fprintf(file, format __VA_OPT__(,) __VA_ARGS__) < 0)    \
        return GRAPH_RETURN_FILE_ERROR;                         \
} while(0)
#define WRITENL(file, format, ...) WRITE(file, format "\n" __VA_OPT__(,) __VA_ARGS__)

struct Graph
{
    Set **adjs; /* adjacency sets */
    unsigned long size; /* number of vertices */
    GraphEdgeType type; /* type of graph */
};

static SetReturnID setID;

GraphReturnID graphCreate(Graph **ppGraph, unsigned long size, GraphEdgeType type)
{
    struct Graph *pGraph;
    unsigned long i, j;
    if (ppGraph == NULL || size == 0 || type >= GRAPH_EDGE_TYPE_INVALID)
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
            return GRAPH_RETURN_MEMORY;
        }
    }
    pGraph->size = size;
    pGraph->type = type;
    *ppGraph = pGraph;
    return GRAPH_RETURN_OK;
}

GraphReturnID graphGetNumberOfVertices(struct Graph *pGraph, unsigned long *pSize)
{
    if (pGraph == NULL || pSize == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    *pSize = pGraph->size;
    return GRAPH_RETURN_OK;
}

GraphReturnID graphGetType(Graph *pGraph, GraphEdgeType *pType)
{
    if (pGraph == NULL || pType == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    *pType = pGraph->type;
    return GRAPH_RETURN_OK;
}

GraphReturnID graphAddEdge(struct Graph *pGraph, unsigned long u, unsigned long v)
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
    if (pGraph->type == GRAPH_EDGE_TYPE_UNDIRECTED) {
        if ((setID = setAdd(pGraph->adjs[v], u)) != SET_RETURN_OK) {
            if (setID == SET_RETURN_MEMORY) {
                if ((setID = setRemove(pGraph->adjs[u], v)) != SET_RETURN_OK)
                    return GRAPH_RETURN_FATAL_ERROR;
                return GRAPH_RETURN_MEMORY;
            }
            return GRAPH_RETURN_UNKNOWN_ERROR;
        }
    }
    return GRAPH_RETURN_OK;
}

GraphReturnID graphContainsEdge(struct Graph *pGraph, unsigned long u, unsigned long v)
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

GraphReturnID graphRemoveEdge(struct Graph *pGraph, unsigned long u, unsigned long v)
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
    if (pGraph->type == GRAPH_EDGE_TYPE_UNDIRECTED) {
        if ((setID = setRemove(pGraph->adjs[v], u)) != SET_RETURN_OK) {
            if ((setID = setAdd(pGraph->adjs[u], v)) != SET_RETURN_OK)
                return GRAPH_RETURN_FATAL_ERROR;
            return GRAPH_RETURN_UNKNOWN_ERROR;
        }
    }
    return GRAPH_RETURN_OK;
}

GraphReturnID graphGetNumberOfNeighbours(Graph *pGraph, unsigned long u, unsigned long *pNum)
{
    if (pGraph == NULL || pNum == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    if (u >= pGraph->size)
        return GRAPH_RETURN_OUT_OF_BOUNDS;
    setGetSize(pGraph->adjs[u], pNum);
    return GRAPH_RETURN_OK;
}

GraphReturnID graphGetNextNeighbour(Graph *pGraph, unsigned long u, unsigned long *pV)
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

GraphReturnID graphWrite(Graph *pGraph, FILE *fp)
{
    unsigned long i, j, value, setSize;
    if (pGraph == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    /* File header */
    WRITENL(fp, "VERSION %d", SERIALIZED_FORMAT_VERSION);
    WRITENL(fp, "TYPE %s", pGraph->type == GRAPH_EDGE_TYPE_DIRECTED ? "DIRECTED" : "UNDIRECTED");
    WRITENL(fp, "VERTICES %lu", pGraph->size);
    /* Adjecency list serialization */
    for (i = 0; i < pGraph->size; ++i) {
        Set *pSet = pGraph->adjs[i];
        setID = setGetSize(pSet, &setSize);
        if (setID != SET_RETURN_OK)
            return GRAPH_RETURN_UNKNOWN_ERROR;
        if (setSize > 0) {
            WRITE(fp, "%lu ", i);
            if (setID = setFirstValue(pSet))
                return GRAPH_RETURN_UNKNOWN_ERROR;
            for (j = 0; j < setSize; j++) {
                if (setID = setGetCurrent(pSet, &value))
                    return GRAPH_RETURN_UNKNOWN_ERROR;
                WRITE(fp, "%lu ", value);
                if (j < setSize - 1)
                    if (setID = setNextValue(pSet))
                        return GRAPH_RETURN_UNKNOWN_ERROR;
            }
            WRITENL(fp, "-1");
        }
    }
    WRITENL(fp, "END");
    return GRAPH_RETURN_OK;
}

void graphDestroy(struct Graph *pGraph)
{
    unsigned long i;
    if (pGraph == NULL)
        return;
    for (i = 0; i < pGraph->size; ++i)
        setDestroy(pGraph->adjs[i]);
    free(pGraph->adjs);
    free(pGraph);
}
