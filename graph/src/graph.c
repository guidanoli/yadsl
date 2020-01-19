#include <stdlib.h>
#include <stdarg.h>
#include "graph.h"
#include "set.h"

struct GraphVertex
{
    void *item;
    Set *adjList;
};

struct Graph
{
    Set *adjListSet; /* set of struct GraphVertex */
    GraphEdgeType type; /* type of graph (directed or undirected) */
    int (*cmpVertices)(void *a, void *b); /* compares GraphVertex::item (0 = different) */
    void (*freeVertex)(void *v); /* frees GraphVertex::item */
};

/* Private functions prototypes */

struct _cmpVertexItemParam
{
    void *item;
    int (*cmpVertices)(void *a, void *b);
};

static int _cmpVertexItem(struct GraphVertex *pVertex, struct _cmpVertexItemParam *par);
static void _freeVertex(struct GraphVertex *pVertex, void (*freeVertex)(void *v));

#define _parseVertices(...) __parseVertices(__VA_ARGS__, NULL)
static GraphReturnID __parseVertices(Graph *pGraph, void *item,
    struct GraphVertex **ppVertex, ...);

GraphReturnID graphCreate(Graph **ppGraph, GraphEdgeType type,
    int (*cmpVertices)(void *a, void *b),
    void (*freeVertex)(void *v))
{
    struct Graph *pGraph;
    if (ppGraph == NULL || type >= GRAPH_EDGE_TYPE_INVALID)
        return GRAPH_RETURN_INVALID_PARAMETER;
    pGraph = malloc(sizeof(struct Graph));
    if (pGraph == NULL)
        return GRAPH_RETURN_MEMORY;
    if (setCreate(&pGraph->adjListSet)) {
        free(pGraph);
        return GRAPH_RETURN_MEMORY;
    }
    pGraph->type = type;
    pGraph->freeVertex = freeVertex;
    pGraph->cmpVertices = cmpVertices;
    *ppGraph = pGraph;
    return GRAPH_RETURN_OK;
}

GraphReturnID graphGetNumberOfVertices(Graph *pGraph, unsigned long *pSize)
{
    unsigned long size;
    if (pGraph == NULL || pSize == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    if (setGetSize(pGraph->adjListSet, &size))
        return GRAPH_RETURN_UNKNOWN_ERROR;
    *pSize = size;
    return GRAPH_RETURN_OK;
}

// Has the monopoly over the adjListSet cursor!!
// It cannot be manipulated by any other function
// This is because the user must be in control of
// which vertex he thinks the cursor is pointing to!
// Other internal functions can use the cursor too but
// must not alter its state after the call (or can
// alter it coherently)
GraphReturnID graphGetNextVertex(Graph *pGraph, void **pV)
{
    SetReturnID setId;
    struct GraphVertex *pVertex;
    if (pGraph == NULL || pV == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    if (setId = setGetCurrentItem(pGraph->adjListSet, &pVertex)) {
        if (setId == SET_RETURN_EMPTY)
            return GRAPH_RETURN_EMPTY;
        return GRAPH_RETURN_UNKNOWN_ERROR;
    }
    if (setId = setNextItem(pGraph->adjListSet)) {
        if (setId != SET_RETURN_OUT_OF_BOUNDS)
            return GRAPH_RETURN_UNKNOWN_ERROR;
        setFirstItem(pGraph->adjListSet);
    }
    *pV = pVertex->item;
    return GRAPH_RETURN_OK;

}

GraphReturnID graphGetType(Graph *pGraph, GraphEdgeType *pType)
{
    if (pGraph == NULL || pType == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    *pType = pGraph->type;
    return GRAPH_RETURN_OK;
}

GraphReturnID graphContainsVertex(Graph *pGraph, void *v)
{
    void *p; // does nothing with p
    struct _cmpVertexItemParam par = {v, pGraph->cmpVertices};
    if (pGraph == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    switch (setFilterItem(pGraph->adjListSet, _cmpVertexItem, &par, &p)) {
    case SET_RETURN_OK:
        return GRAPH_RETURN_CONTAINS_VERTEX;
    case SET_RETURN_DOES_NOT_CONTAIN:
        return GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX;
    default:
        return GRAPH_RETURN_UNKNOWN_ERROR;
    }
}

GraphReturnID graphAddVertex(Graph *pGraph, void *v)
{
    struct GraphVertex *pVertex;
    GraphReturnID graphId;
    SetReturnID setId;
    if (pGraph == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    graphId = graphContainsVertex(pGraph, v);
    if (graphId != GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX)
        return graphId;
    pVertex = malloc(sizeof(struct GraphVertex));
    if (pVertex == NULL)
        return GRAPH_RETURN_MEMORY;
    pVertex->item = v;
    if (setCreate(&pVertex->adjList)) {
        free(pVertex);
        return GRAPH_RETURN_MEMORY;
    }
    if (setId = setAddItem(pGraph->adjListSet, pVertex)) {
        setDestroy(pVertex->adjList);
        free(pVertex);
        switch (setId) {
        case SET_RETURN_MEMORY:
            return GRAPH_RETURN_MEMORY;
        // can't be SET_RETURN_CONTAINS because it has
        // been already checked with graphContainsVertex
        default:
            return GRAPH_RETURN_UNKNOWN_ERROR;
        }
    }
    return GRAPH_RETURN_OK;
}

GraphReturnID graphRemoveVertex(Graph *pGraph, void *v)
{
    struct GraphVertex *pVertex, *pAux;
    unsigned long count;
    GraphReturnID graphId;
    SetReturnID setId;
    if (pGraph == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    if (graphId = _parseVertices(pGraph, v, &pVertex))
        return graphId;
    /* Removing the edges that connect to the vertex */
    if (pGraph->type == GRAPH_EDGE_TYPE_DIRECTED) {
        // Directed graph
        if (graphId = graphGetNumberOfVertices(pGraph, &count))
            return GRAPH_RETURN_UNKNOWN_ERROR;
        while (count--) {
            if (setId = setGetCurrentItem(pGraph->adjListSet, &pAux))
                // Cannot be SET_RETURN_EMPTY because then count
                // would be 0 and it would not have entered the while loop
                return GRAPH_RETURN_FATAL_ERROR; // Cursor moved
            if (setId = setNextItem(pGraph->adjListSet)) {
                // Cannot be SET_RETURN_EMPTY for the same reason as
                // the one previously described
                if (setId != SET_RETURN_OUT_OF_BOUNDS)
                    return GRAPH_RETURN_FATAL_ERROR; // Cursor moved
                setFirstItem(pGraph->adjListSet);
            }
            if (setId = setRemoveItem(pAux->adjList, pVertex))
                if (setId != SET_RETURN_DOES_NOT_CONTAIN)
                    return GRAPH_RETURN_UNKNOWN_ERROR;
        }
    } else { // Assuming there are only two graph edge types
        // Undirected graph
        if (setId = setGetSize(pVertex->adjList, &count))
            return GRAPH_RETURN_UNKNOWN_ERROR;
        while (count--) {
            if (setId = setGetCurrentItem(pVertex->adjList, &pAux))
                // Cannot be SET_RETURN_EMPTY because then count
                // would be 0 and it would not have entered the while loop
                return GRAPH_RETURN_UNKNOWN_ERROR;
            if (setId = setNextItem(pVertex->adjList)) {
                // Cannot be SET_RETURN_EMPTY for the same reason as
                // the one previously described
                if (setId != SET_RETURN_OUT_OF_BOUNDS)
                    return GRAPH_RETURN_UNKNOWN_ERROR;
                setFirstItem(pVertex->adjList);
            }
            if (setRemoveItem(pAux->adjList, pVertex))
                return GRAPH_RETURN_UNKNOWN_ERROR;
        }
    }
    /* Removing the vertex itself and its dependencies */
    if (setId = setRemoveItem(pGraph->adjListSet, pVertex))
        // Cannot be SET_RETURN_DOES_NOT_CONTAIN because it
        // was obtained from it
        return GRAPH_RETURN_UNKNOWN_ERROR;
    _freeVertex(pVertex, pGraph->freeVertex);
    return GRAPH_RETURN_OK;
}

GraphReturnID graphAddEdge(Graph *pGraph, void *u, void *v)
{
    struct GraphVertex *pVertexU, *pVertexV;
    GraphReturnID graphId;
    SetReturnID setId;
    if (pGraph == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    if (pGraph->cmpVertices(u,v)) /* Assumes simple graph */
        return GRAPH_RETURN_SAME_VERTEX;
    if (graphId = _parseVertices(pGraph, u, &pVertexU, v, &pVertexV))
        return graphId;
    if ((setId = setContainsItem(pVertexU->adjList, pVertexV)) != SET_RETURN_DOES_NOT_CONTAIN) {
        if (setId == SET_RETURN_CONTAINS)
            return GRAPH_RETURN_CONTAINS_EDGE;
        return GRAPH_RETURN_UNKNOWN_ERROR;
    }
    if (setId = setAddItem(pVertexU->adjList, pVertexV)) {
        if (setId == SET_RETURN_MEMORY)
            return GRAPH_RETURN_MEMORY;
        return GRAPH_RETURN_UNKNOWN_ERROR;
    }
    if (pGraph->type == GRAPH_EDGE_TYPE_UNDIRECTED) {
        if (setId = setAddItem(pVertexV->adjList, pVertexU)) {
            if (setId == SET_RETURN_MEMORY) {
                /* Try to revert and delete edge uv */
                if (setId = setRemove(pVertexU->adjList, pVertexV))
                    return GRAPH_RETURN_FATAL_ERROR;
                return GRAPH_RETURN_MEMORY;
            }
            return GRAPH_RETURN_UNKNOWN_ERROR;
        }
    }
    return GRAPH_RETURN_OK;
}

GraphReturnID graphContainsEdge(Graph *pGraph, void *u, void *v)
{
    struct GraphVertex *pVertexU, *pVertexV;
    GraphReturnID graphId;
    if (pGraph == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    if (pGraph->cmpVertices(u,v)) /* Assumes simple graph */
        return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
    if (graphId = _parseVertices(pGraph, u, &pVertexU, v, &pVertexV)) {
        if (graphId == GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX)
            return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
        return graphId;
    }
    switch (setContainsItem(pVertexU->adjList, pVertexV)) {
    case SET_RETURN_CONTAINS:
        return GRAPH_RETURN_CONTAINS_EDGE;
    case SET_RETURN_DOES_NOT_CONTAIN:
        return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
    default:
        return GRAPH_RETURN_UNKNOWN_ERROR;
    }
}

GraphReturnID graphRemoveEdge(Graph *pGraph, void *u, void *v)
{
    struct GraphVertex *pVertexU, *pVertexV;
    GraphReturnID graphId;
    SetReturnID setId;
    if (pGraph == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    if (pGraph->cmpVertices(u,v)) /* Assumes simple graph */
        return GRAPH_RETURN_SAME_VERTEX;
    if (graphId = _parseVertices(pGraph, u, &pVertexU, v, &pVertexV))
        return graphId;
    if ((setId = setContains(pVertexU->adjList, pVertexV)) != SET_RETURN_CONTAINS) {
        if (setId == SET_RETURN_DOES_NOT_CONTAIN)
            return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
        return GRAPH_RETURN_UNKNOWN_ERROR;
    }
    if (setRemove(pVertexU->adjList, pVertexV))
        return GRAPH_RETURN_UNKNOWN_ERROR;
    if (pGraph->type == GRAPH_EDGE_TYPE_UNDIRECTED) {
        if (setRemove(pVertexV->adjList, pVertexU)) {
            /* Try to revert and add edge uv */
            if (setAdd(pVertexU->adjList, pVertexV))
                return GRAPH_RETURN_FATAL_ERROR;
            return GRAPH_RETURN_UNKNOWN_ERROR;
        }
    }
    return GRAPH_RETURN_OK;
}

GraphReturnID graphGetNumberOfNeighbours(Graph *pGraph, void *v, unsigned long *pNum)
{
    struct GraphVertex *pVertex;
    GraphReturnID graphId;
    if (pGraph == NULL || pNum == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    if (graphId = _parseVertices(pGraph, v, &pVertex))
        return graphId;
    if (setGetSize(pVertex->adjList, pNum))
        return GRAPH_RETURN_UNKNOWN_ERROR;
    return GRAPH_RETURN_OK;
}

GraphReturnID graphGetNextNeighbour(Graph *pGraph, void *v, void **pV)
{
    struct GraphVertex *pVertex;
    GraphReturnID graphId;
    SetReturnID setId;
    void *temp;
    if (pGraph == NULL || pV == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    if (graphId = _parseVertices(pGraph, v, &pVertex))
        return graphId;
    if (setId = setGetCurrent(pVertex->adjList, &temp)) {
        if (setId != SET_RETURN_EMPTY)
            return GRAPH_RETURN_UNKNOWN_ERROR;
        return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
    }
    if (setId = setNextItem(pVertex->adjList)) {
        if (setId != SET_RETURN_OUT_OF_BOUNDS)
            return GRAPH_RETURN_UNKNOWN_ERROR;
        setFirstValue(pVertex->adjList);
    }
    *pV = temp;
    return GRAPH_RETURN_OK;
}

void graphDestroy(Graph *pGraph)
{
    if (pGraph == NULL)
        return;
    setDestroyDeep(pGraph->adjListSet, _freeVertex, pGraph->freeVertex);
    free(pGraph);
}

/* Private functions */

// Compares item from graph vertex struct and item
static int _cmpVertexItem(struct GraphVertex *pVertex, struct _cmpVertexItemParam *par)
{
    if (par == NULL || pVertex == NULL) return 0;
    if (par->cmpVertices == NULL)
        return pVertex->item == par->item;
    return par->cmpVertices(pVertex->item, par->item);
}

// Deallocates vertex from memory properly
static void _freeVertex(struct GraphVertex *pVertex, void (*freeVertex)(void *v))
{
    if (freeVertex)
        freeVertex(pVertex->item);
    setDestroy(pVertex->adjList);
    free(pVertex);
}

// Retrieves the vertices through the items contained in them.
// The item and ppVertex arguments must be alternated, and end with NULL, eg:
// __parseVertices(pGrpah, u, &pVertexU, v, &pVertexV, w, &pVertexW, NULL);
// The macro with only one '_' already does the above requirement.
// Possible errors:
// GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
// GRAPH_RETURN_UNKNOWN_ERROR
static GraphReturnID __parseVertices(Graph *pGraph, void *item, struct GraphVertex **ppVertex, ...)
{
    SetReturnID setId;
    va_list va;
    struct _cmpVertexItemParam param = {item, pGraph->cmpVertices};
    va_start(va, ppVertex);
    do {
        if (setId = setFilterItem(pGraph->adjListSet, _cmpVertexItem, &param, ppVertex)) {
            va_end(va);
            if (setId == SET_RETURN_DOES_NOT_CONTAIN)
                return GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX;
            return GRAPH_RETURN_UNKNOWN_ERROR;
        }
        param.item = va_arg(va, void *);
        if (param.item == NULL)
            break; /* Found sentinel */
        ppVertex = va_arg(va, struct GraphVertex **);
    } while(1);
    va_end(va);
    return GRAPH_RETURN_OK;
}