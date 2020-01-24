#include <string.h>
#include <stdlib.h>
#include "graphio.h"
#include "map.h"

#define WRITE(file, format, ...) do {           \
    if (fprintf(file, format, __VA_ARGS__) < 0) \
        return GRAPH_IO_RETURN_FILE_ERROR;      \
} while(0)

#define READ(format, arg) do {                      \
    if (fscanf(fp, " " format " ", arg) != 1) {     \
        _housekeep(pGraph, pMap);                   \
        return GRAPH_IO_RETURN_FILE_ERROR;          \
    }                                               \
} while(0)

#define WRITENL(file, format, ...) WRITE(file, format "\n", __VA_ARGS__)

#define FILE_FORMAT_VERSION 3

struct EdgeType {
    char label;
    int isDirected;
};

static struct EdgeType edgeTypes[] = {
    { 'D', 1 },
    { 'U', 0 },
    { (char) 0, -1 },
};

#define VERSION_STR         "VERSION %u"
#define TYPE_STR            "EDGE_TYPE %c"
#define VERTICES_STR        "VERTEX_COUNT %lu"
#define VERTEX_ADDR_STR     "%x "
#define NBCOUNT_STR         "%lu"
#define NB_ADDR_STR         " %x "

/* Private functions prototypes */

static int _charToType(const char c);
static char _typeToChar(int type);
static void _housekeep(Graph *pGraph, Map *pMap);

/* Public functions */

GraphIoReturnID graphWrite(Graph *pGraph, FILE *fp,
    int (*writeVertex)(FILE *fp, void *v),
    int (*writeEdge)(FILE *fp, void *e))
{
    int isDirected;
    unsigned long vCount, nbCount, i, j;
    void *pVertex, *pNeighbour, *pEdge;
    if (pGraph == NULL || writeVertex == NULL || writeEdge == NULL)
        return GRAPH_IO_RETURN_INVALID_PARAMETER;
    if (graphIsDirected(pGraph, &isDirected))
        return GRAPH_IO_RETURN_UNKNOWN_ERROR;
    if (graphGetNumberOfVertices(pGraph, &vCount))
        return GRAPH_IO_RETURN_UNKNOWN_ERROR;
    WRITENL(fp, VERSION_STR, FILE_FORMAT_VERSION); // Version
    WRITENL(fp, TYPE_STR, _typeToChar(isDirected)); // Type
    WRITENL(fp, VERTICES_STR, vCount); // Vertex count
    for (i = vCount; i; --i) {
        if (graphGetNextVertex(pGraph, &pVertex))
            return GRAPH_IO_RETURN_UNKNOWN_ERROR;
        WRITE(fp, VERTEX_ADDR_STR, pVertex); // Vertex address
        if (writeVertex(fp, pVertex)) // Vertex data serialization
            return GRAPH_IO_RETURN_WRITING_FAILURE;
        WRITENL(fp, "");
    }
    for (i = vCount; i; --i) {
        if (graphGetNextVertex(pGraph, &pVertex))
            return GRAPH_IO_RETURN_UNKNOWN_ERROR;
        if (graphGetVertexOutDegree(pGraph, pVertex, &nbCount))
            return GRAPH_IO_RETURN_UNKNOWN_ERROR;
        WRITE(fp, VERTEX_ADDR_STR, pVertex); // Vertex address
        WRITE(fp, NBCOUNT_STR, nbCount); // Vertex degree
        for (j = nbCount; j; --j) {
            if (graphGetNextOutNeighbour(pGraph, pVertex, &pNeighbour, &pEdge))
                return GRAPH_IO_RETURN_UNKNOWN_ERROR;
            WRITE(fp, NB_ADDR_STR, pNeighbour); // Neighbour address
            if (writeEdge(fp, pEdge)) // Edge data serialization
                return GRAPH_IO_RETURN_WRITING_FAILURE;
        }
        WRITENL(fp, "");
    }
    return GRAPH_IO_RETURN_OK;
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
    unsigned long vCount, nbCount, i, j;
    char typeChar;
    GraphReturnID graphId;
    MapReturnID mapId;
    int isDirected;
    Graph *pGraph = NULL;
    Map *pMap = NULL;
    void *pVertexAddress, *pNeighbourAddress,
        *pVertexItem, *pNeighbourItem, *pEdge;
    if (ppGraph == NULL || readVertex == NULL || readEdge == NULL)
        return GRAPH_IO_RETURN_INVALID_PARAMETER;
    READ(VERSION_STR, &version); // Version
    if (version != FILE_FORMAT_VERSION)
        return GRAPH_IO_RETURN_DEPRECATED_FILE_FORMAT;
    READ(TYPE_STR, &typeChar); // Type
    if ((isDirected = _charToType(typeChar)) == -1)
        return GRAPH_IO_RETURN_CORRUPTED_FILE_FORMAT;
    READ(VERTICES_STR, &vCount); // Vertex count
    /* Map where key = vertex address and values = vertex data */
    if (mapId = mapCreate(&pMap, NULL, NULL, NULL)) {
        if (mapId == MAP_RETURN_MEMORY)
            return GRAPH_IO_RETURN_MEMORY;
        return GRAPH_IO_RETURN_UNKNOWN_ERROR;
    }
    /* Graph to be created */
    if (graphId = graphCreate(&pGraph, isDirected, cmpVertices,
        freeVertex, cmpEdges, freeEdge)) {
        mapDestroy(pMap);
        if (graphId == GRAPH_RETURN_MEMORY)
            return GRAPH_IO_RETURN_MEMORY;
        return GRAPH_IO_RETURN_UNKNOWN_ERROR;
    }
    for (i = vCount; i; --i) {
        void *pPreviousValue = NULL;
        READ(VERTEX_ADDR_STR, &pVertexAddress); // Vertex address
        if (readVertex(fp, &pVertexItem)) { // Vertex data serialization
            _housekeep(pGraph, pMap);
            return GRAPH_IO_RETURN_CREATION_FAILURE;
        }
        if (mapId = mapPutEntry(pMap, pVertexAddress, pVertexItem, &pPreviousValue)) {
            freeVertex(pVertexItem);
            _housekeep(pGraph, pMap);
            switch(mapId) {
            case MAP_RETURN_OVERWROTE_ENTRY:
                freeVertex(pPreviousValue); // old entry
                return GRAPH_IO_RETURN_CORRUPTED_FILE_FORMAT;
            case MAP_RETURN_MEMORY:
                return GRAPH_IO_RETURN_MEMORY;
            default:
                return GRAPH_IO_RETURN_UNKNOWN_ERROR;
            }
        }
        if (graphId = graphAddVertex(pGraph, pVertexItem)) {
            freeVertex(pVertexItem);
            _housekeep(pGraph, pMap);
            switch (graphId) {
            case GRAPH_RETURN_MEMORY:
                return GRAPH_IO_RETURN_MEMORY;
            case GRAPH_RETURN_CONTAINS_VERTEX:
                // Repeated vertex data
                return GRAPH_IO_RETURN_SAME_CREATION;
            default:
                return GRAPH_IO_RETURN_UNKNOWN_ERROR;
            }
        }
    }
    for (i = vCount; i; --i) {
        unsigned long arcCount = 0;
        READ(VERTEX_ADDR_STR, &pVertexAddress); // Vertex address
        if (mapId = mapGetEntry(pMap, pVertexAddress, &pVertexItem)) {
            _housekeep(pGraph, pMap);
            return GRAPH_IO_RETURN_UNKNOWN_ERROR;
        }
        READ(NBCOUNT_STR, &nbCount); // Vertex degree
        for (j = nbCount; j; --j) {
            READ(NB_ADDR_STR, &pNeighbourAddress); // Neighbour address
            if (readEdge(fp, &pEdge)) {
                _housekeep(pGraph, pMap);
                return GRAPH_IO_RETURN_CREATION_FAILURE;
            }
            if (mapId = mapGetEntry(pMap, pNeighbourAddress, &pNeighbourItem)) {
                freeEdge(pEdge);
                _housekeep(pGraph, pMap);
                return GRAPH_IO_RETURN_UNKNOWN_ERROR;
            }
            if (graphId = graphAddEdge(pGraph, pVertexItem, pNeighbourItem, pEdge)) {
                freeEdge(pEdge);
                switch (graphId) {
                case GRAPH_RETURN_MEMORY:
                    _housekeep(pGraph, pMap);
                    return GRAPH_IO_RETURN_MEMORY;
                case GRAPH_RETURN_CONTAINS_EDGE:
                    if (pVertexAddress == pNeighbourAddress && ++arcCount > 1) {
                        _housekeep(pGraph, pMap);
                        return GRAPH_IO_RETURN_CORRUPTED_FILE_FORMAT;
                    }
                    break;
                default:
                    _housekeep(pGraph, pMap);
                    return GRAPH_IO_RETURN_UNKNOWN_ERROR;
                }
            }
        }
    }
    mapDestroy(pMap);
    *ppGraph = pGraph;
    return GRAPH_IO_RETURN_OK;
}

/* Private functions */

// Converts character to type
static int _charToType(const char c)
{
    switch (c) {
    case 'U':
        return 0;
    case 'D':
        return 1;
    default:
        return -1;
    }
}

// Converts type to character
static char _typeToChar(int type)
{
    return type ? 'D' : 'U';
}

static void _housekeep(Graph *pGraph, Map *pMap)
{
    if (pGraph)
        graphDestroy(pGraph);
    if (pMap)
        mapDestroy(pMap);
}