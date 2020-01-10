#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "graph.h"
#include "set.h"

#define WRITE(file, format, ...) do {           \
    if (fprintf(file, format, __VA_ARGS__) < 0) \
        return GRAPH_RETURN_FILE_ERROR;         \
} while(0)

#define READ(file, n, ptr, format, ...) do {            \
    if (fscanf(file, format " ", __VA_ARGS__) != n) {   \
        if (ptr != NULL)                                \
            graphDestroy(ptr);                          \
        return GRAPH_RETURN_FILE_ERROR;                 \
    }                                                   \
} while(0)

#define WRITENL(file, format, ...) WRITE(file, format "\n", __VA_ARGS__)

#define FILE_FORMAT_VERSION 2
#define BUFFER_SIZE 128

#define DIRECTED_STR "DIRECTED"
#define UNDIRECTED_STR "UNDIRECTED"
#define INVALID_STR "INVALID"

#define VERSION_STR "VERSION %u"
#define TYPE_STR "TYPE %s"
#define VERTICES_STR "VERTICES %lu"
#define REFVERTICES_STR "REF_VERTICES %lu"

struct Graph
{
    Set **adjs; /* adjacency sets */
    unsigned long size; /* number of vertices */
    GraphEdgeType type; /* type of graph */
};

/* Auxiliary variable */
static SetReturnID setID;

/* Private functions */
static GraphEdgeType strToType(const char *str);
static const char *typeToStr(GraphEdgeType type);

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
    if (setID = setAdd(pGraph->adjs[u], v)) {
        if (setID == SET_RETURN_MEMORY)
            return GRAPH_RETURN_MEMORY;
        return GRAPH_RETURN_UNKNOWN_ERROR;
    }
    if (pGraph->type == GRAPH_EDGE_TYPE_UNDIRECTED) {
        if (setID = setAdd(pGraph->adjs[v], u)) {
            if (setID == SET_RETURN_MEMORY) {
                if (setID = setRemove(pGraph->adjs[u], v))
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
    unsigned long u, v, i, setSize, refVertices = 0;
    GraphReturnID id;
    if (pGraph == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    for (u = 0; u < pGraph->size; ++u) {
        if (id = graphGetNumberOfNeighbours(pGraph, u, &setSize))
            return id;
        if (setSize > 0) {
            if (pGraph->type == GRAPH_EDGE_TYPE_UNDIRECTED) {
                for (i = 0; i < setSize; ++i) {
                    if (id = graphGetNextNeighbour(pGraph, u, &v))
                        return id;
                    if (u <= v) {
                        ++refVertices;
                        break;
                    }
                }
            } else {
                ++refVertices;
            }
        }
    }
    WRITENL(fp, VERSION_STR, FILE_FORMAT_VERSION);
    WRITENL(fp, TYPE_STR, typeToStr(pGraph->type));
    WRITENL(fp, VERTICES_STR, pGraph->size);
    WRITENL(fp, REFVERTICES_STR, refVertices);
    for (u = 0; u < pGraph->size; ++u) {
        int hasNeighbour = 0; /* auxiliary flag */
        if (id = graphGetNumberOfNeighbours(pGraph, u, &setSize))
            return id;
        if (setSize > 0) {
            for (i = 0; i < setSize; ++i) {
                if (id = graphGetNextNeighbour(pGraph, u, &v))
                    return id;
                if (pGraph->type == GRAPH_EDGE_TYPE_DIRECTED || u <= v) {
                    if (!hasNeighbour) {
                        WRITE(fp, "%lu ", u);
                        hasNeighbour = 1;
                    }
                    WRITE(fp, "%lu ", v);
                }
            }
            if (hasNeighbour)
                WRITENL(fp, "%d", -1);
        }
    }
    return GRAPH_RETURN_OK;
}

GraphReturnID graphRead(Graph **ppGraph, FILE *fp)
{
    unsigned int version;
    char buffer[BUFFER_SIZE];
    Graph *pGraph = NULL;
    GraphEdgeType type;
    GraphReturnID id;
    unsigned long i, size, refSize, u, v;
    if (ppGraph == NULL)
        return GRAPH_RETURN_INVALID_PARAMETER;
    READ(fp, 1, pGraph, VERSION_STR, &version);
    if (version != FILE_FORMAT_VERSION)
        return GRAPH_RETURN_DEPRECATED_FILE_FORMAT;
    READ(fp, 1, pGraph, TYPE_STR, buffer);
    if ((type = strToType(buffer)) == GRAPH_EDGE_TYPE_INVALID)
        return GRAPH_RETURN_CORRUPTED_FILE_FORMAT;
    READ(fp, 1, pGraph, VERTICES_STR, &size);
    if (size == 0)
        return GRAPH_RETURN_CORRUPTED_FILE_FORMAT;
    if (id = graphCreate(&pGraph, size, type))
        return id;
    READ(fp, 1, pGraph, REFVERTICES_STR, &refSize);
    for (i = 0; i < refSize; ++i) {
        READ(fp, 1, pGraph, "%lu ", &u);
        while (1) {
            READ(fp, 1, pGraph, "%lu ", &v);
            if (v == -1L)
                break;
            if (id = graphAddEdge(pGraph, u, v)) {
                graphDestroy(pGraph);
                if (id == GRAPH_RETURN_CONTAINS_EDGE ||
                    id == GRAPH_RETURN_OUT_OF_BOUNDS ||
                    id == GRAPH_RETURN_SAME_VERTEX)
                    return GRAPH_RETURN_CORRUPTED_FILE_FORMAT;
                return id;
            }
        }
    }
    *ppGraph = pGraph;
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

static GraphEdgeType strToType(const char *str) {
    if (strcmp(str, DIRECTED_STR) == 0)
        return GRAPH_EDGE_TYPE_DIRECTED;
    else if (strcmp(str, UNDIRECTED_STR) == 0)
        return GRAPH_EDGE_TYPE_UNDIRECTED;
    return GRAPH_EDGE_TYPE_INVALID;
}

static const char *typeToStr(GraphEdgeType type) {
    switch (type) {
    case GRAPH_EDGE_TYPE_DIRECTED:
        return DIRECTED_STR;
    case GRAPH_EDGE_TYPE_UNDIRECTED:
        return UNDIRECTED_STR;
    default:
        return INVALID_STR;
    }
}
