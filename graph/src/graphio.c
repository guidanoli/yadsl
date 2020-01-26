#include <string.h>
#include <stdlib.h>
#include "graphio.h"
#include "map.h"

#define WRITE(file, format, ...) do {           \
    if (fprintf(file, format, __VA_ARGS__) < 0) \
        return GRAPH_IO_RETURN_FILE_ERROR;      \
} while(0)

#define READ(format, arg) do {                      \
    if (fscanf(fp, " " format " ", arg) != 1)       \
        return GRAPH_IO_RETURN_FILE_ERROR;          \
} while(0)

#define WRITENL(file, format, ...) WRITE(file, format "\n", __VA_ARGS__)

#define FILE_FORMAT_VERSION 4

#define VERSION_STR     "VERSION %u"
#define DIRECTED_STR    "IS_DIRECTED %d"
#define VCOUNT_STR      "%lu "
#define VERTEX_IDX_STR  "%lu "
#define NBCOUNT_STR     "%lu"
#define NB_IDX_STR      " %lu "

/* Private functions prototypes */

static GraphIoReturnID _graphWrite(Graph *pGraph, FILE *fp,
    int (*writeVertex)(FILE *fp, void *v),
    int (*writeEdge)(FILE *fp, void *e),
    Map *addressMap);

static GraphIoReturnID _graphRead(Graph *pGraph, void **addressMap,
    FILE *fp, unsigned long vCount, int isDirected,
    int (*readVertex)(FILE *fp, void **ppVertex),
    int (*readEdge)(FILE *fp, void **ppEdge),
    void (*freeVertex)(void *v),
    void (*freeEdge)(void *e));

/* Public functions */

GraphIoReturnID graphWrite(Graph *pGraph, FILE *fp,
    int (*writeVertex)(FILE *fp, void *v),
    int (*writeEdge)(FILE *fp, void *e))
{
    Map *addressMap;
    GraphIoReturnID id;
    if (pGraph == NULL || writeVertex == NULL || writeEdge == NULL)
        return GRAPH_IO_RETURN_INVALID_PARAMETER;
    if (mapCreate(&addressMap, NULL, NULL, NULL))
        return GRAPH_IO_RETURN_MEMORY;
    id = _graphWrite(pGraph, fp, writeVertex, writeEdge, addressMap);
    mapDestroy(addressMap);
    return id;
}

GraphIoReturnID graphRead(Graph **ppGraph, FILE *fp,
    int (*readVertex)(FILE *fp, void **ppVertex),
    int (*readEdge)(FILE *fp, void **ppEdge),
    int (*cmpVertices)(void *a, void *b),
    void (*freeVertex)(void *v),
    int (*cmpEdges)(void *a, void *b),
    void (*freeEdge)(void *e))
{
    unsigned int version;
    unsigned long vCount;
    GraphReturnID graphId;
    GraphIoReturnID id;
    int isDirected;
    Graph *pGraph = NULL;
    void **addressMap;
    if (ppGraph == NULL || readVertex == NULL || readEdge == NULL)
        return GRAPH_IO_RETURN_INVALID_PARAMETER;
    READ(VERSION_STR, &version); // Version
    if (version != FILE_FORMAT_VERSION)
        return GRAPH_IO_RETURN_DEPRECATED_FILE_FORMAT;
    READ(DIRECTED_STR, &isDirected); // Directed
    READ(VCOUNT_STR, &vCount); // Vertex count
    /* Graph to be created */
    if (graphId = graphCreate(&pGraph, isDirected, cmpVertices,
        freeVertex, cmpEdges, freeEdge)) {
        if (graphId == GRAPH_RETURN_MEMORY)
            return GRAPH_IO_RETURN_MEMORY;
        return GRAPH_IO_RETURN_UNKNOWN_ERROR;
    }
    /* Address map to store vertex items */
    addressMap = malloc(vCount * sizeof(void *));
    if (addressMap == NULL) {
        graphDestroy(pGraph);
        return GRAPH_IO_RETURN_MEMORY;
    }
    id = _graphRead(pGraph, addressMap, fp, vCount, isDirected,
        readVertex, readEdge, freeVertex, freeEdge);
    free(addressMap);
    if (id) {
        graphDestroy(pGraph);
        return id;
    }
    *ppGraph = pGraph;
    return GRAPH_IO_RETURN_OK;
}

/* Private functions */

static GraphIoReturnID _graphWrite(Graph *pGraph, FILE *fp,
    int (*writeVertex)(FILE *fp, void *v),
    int (*writeEdge)(FILE *fp, void *e),
    Map *addressMap)
{
    int isDirected;
    MapReturnID mapId;
    unsigned long vCount, nbCount, i, j, index;
    GraphIoReturnID id;
    void *pVertex, *pNeighbour, *pEdge;
    if (graphIsDirected(pGraph, &isDirected))
        return GRAPH_IO_RETURN_UNKNOWN_ERROR;
    if (graphGetNumberOfVertices(pGraph, &vCount))
        return GRAPH_IO_RETURN_UNKNOWN_ERROR;
    WRITENL(fp, VERSION_STR, FILE_FORMAT_VERSION); // Version
    WRITENL(fp, DIRECTED_STR, isDirected); // Type
    WRITE(fp, VCOUNT_STR, vCount); // Vertex count
    for (i = 0; i < vCount; ++i) {
        if (graphGetNextVertex(pGraph, &pVertex))
            return GRAPH_IO_RETURN_UNKNOWN_ERROR;
        if (mapId = mapPutEntry(addressMap, pVertex,
            (void *) i, (void **) &index)) {
            if (mapId == MAP_RETURN_MEMORY)
                return GRAPH_IO_RETURN_MEMORY;
            return GRAPH_IO_RETURN_UNKNOWN_ERROR;
        }
        if (writeVertex(fp, pVertex)) // Vertex item
            return GRAPH_IO_RETURN_WRITING_FAILURE;
        WRITE(fp, " ");
    }
    WRITENL(fp, "");
    for (i = vCount; i; --i) {
        if (graphGetNextVertex(pGraph, &pVertex))
            return GRAPH_IO_RETURN_UNKNOWN_ERROR;
        if (graphGetVertexOutDegree(pGraph, pVertex, &nbCount))
            return GRAPH_IO_RETURN_UNKNOWN_ERROR;
        WRITE(fp, NBCOUNT_STR, nbCount); // Vertex degree
        for (j = nbCount; j; --j) {
            if (graphGetNextOutNeighbour(pGraph, pVertex, &pNeighbour, &pEdge))
                return GRAPH_IO_RETURN_UNKNOWN_ERROR;
            if (mapGetEntry(addressMap, pNeighbour, (void **) &index))
                return GRAPH_IO_RETURN_UNKNOWN_ERROR;
            WRITE(fp, NB_IDX_STR, index); // Neighbour index
            if (writeEdge(fp, pEdge)) // Edge item
                return GRAPH_IO_RETURN_WRITING_FAILURE;
        }
        WRITENL(fp, "");
    }
    return GRAPH_IO_RETURN_OK;
}

static GraphIoReturnID _graphRead(Graph *pGraph, void **addressMap,
    FILE *fp, unsigned long vCount, int isDirected,
    int (*readVertex)(FILE *fp, void **ppVertex),
    int (*readEdge)(FILE *fp, void **ppEdge),
    void (*freeVertex)(void *v),
    void (*freeEdge)(void *e))
{
    unsigned long nbCount, i, j, index;
    GraphReturnID graphId;
    void *pVertexItem, *pNeighbourItem, *pEdgeItem;
    for (i = 0; i < vCount; ++i) {
        void *pPreviousValue = NULL;
        fscanf(fp, " "); // Ignore space characters
        if (readVertex(fp, &pVertexItem)) // Vertex item
            return GRAPH_IO_RETURN_CREATION_FAILURE;
        addressMap[i] = pVertexItem;
        if (graphId = graphAddVertex(pGraph, pVertexItem)) {
            if (freeVertex)
                freeVertex(pVertexItem);
            switch (graphId) {
            case GRAPH_RETURN_MEMORY:
                return GRAPH_IO_RETURN_MEMORY;
            case GRAPH_RETURN_CONTAINS_VERTEX:
                return GRAPH_IO_RETURN_SAME_CREATION;
            default:
                return GRAPH_IO_RETURN_UNKNOWN_ERROR;
            }
        }
    }
    for (i = 0; i < vCount; ++i) {
        pVertexItem = addressMap[i];
        READ(NBCOUNT_STR, &nbCount); // Vertex degree
        for (j = 0; j < nbCount; ++j) {
            READ(NB_IDX_STR, &index); // Neighbour index
            pNeighbourItem = addressMap[index];
            if (readEdge(fp, &pEdgeItem)) // Edge item
                return GRAPH_IO_RETURN_CREATION_FAILURE;
            if (graphId = graphAddEdge(pGraph, pVertexItem, pNeighbourItem, pEdgeItem)) {
                if (freeEdge)
                    freeEdge(pEdgeItem);
                switch (graphId) {
                case GRAPH_RETURN_MEMORY:
                    return GRAPH_IO_RETURN_MEMORY;
                case GRAPH_RETURN_CONTAINS_EDGE:
                    return GRAPH_IO_RETURN_CORRUPTED_FILE_FORMAT;
                default:
                    return GRAPH_IO_RETURN_UNKNOWN_ERROR;
                }
            }
        }
    }
    return GRAPH_IO_RETURN_OK;
}