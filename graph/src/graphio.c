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
#define BUFFER_SIZE 128

struct EdgeType {
    char label;
    GraphEdgeType type;
};

static struct EdgeType edgeTypes[] = {
    { 'D', GRAPH_EDGE_TYPE_DIRECTED },
    { 'U', GRAPH_EDGE_TYPE_UNDIRECTED },
    { (char) 0, (GraphEdgeType) -1 },
};

#define VERSION_STR         "VERSION %u"
#define TYPE_STR            "EDGE_TYPE %c"
#define VERTICES_STR        "VERTEX_COUNT %lu"
#define VERTEX_ADDR_STR     "%x "
#define NBCOUNT_STR         "%lu"
#define NB_ADDR_STR         " %x"

/* Private functions prototypes */

static GraphEdgeType _charToType(const char c);
static char _typeToChar(GraphEdgeType type);

static GraphIoReturnID _read(FILE *fp, Graph *pGraph, struct Vertex *pVertices,
    const char *format, void *arg, void (*freeVertex)(void *v));
static void _housekeep(Graph *pGraph, Map *pMap);

static void _freeEntry(void *address, void *data, void (*freeVertex)(void *v));

/* Public functions */

GraphIoReturnID graphWrite(Graph *pGraph, FILE *fp,
    int (*writeVertex)(FILE *fp, void *v))
{
    GraphEdgeType type;
    unsigned long vCount, nbCount, i, j;
    void *pVertex, *pNeighbour;
    if (pGraph == NULL || writeVertex == NULL)
        return GRAPH_IO_RETURN_INVALID_PARAMETER;
    if (graphGetType(pGraph, &type))
        return GRAPH_IO_RETURN_UNKNOWN_ERROR;
    if (graphGetNumberOfVertices(pGraph, &vCount))
        return GRAPH_IO_RETURN_UNKNOWN_ERROR;
    WRITENL(fp, VERSION_STR, FILE_FORMAT_VERSION);
    WRITENL(fp, TYPE_STR, _typeToChar(type));
    WRITENL(fp, VERTICES_STR, vCount);
    for (i = vCount; i; --i) {
        if (graphGetNextVertex(pGraph, &pVertex))
            return GRAPH_IO_RETURN_UNKNOWN_ERROR;
        WRITE(fp, VERTEX_ADDR_STR, pVertex);
        if (writeVertex(fp, pVertex))
            return GRAPH_IO_RETURN_WRITING_FAILURE;
        WRITENL(fp, "");
    }
    for (i = vCount; i; --i) {
        if (graphGetNextVertex(pGraph, &pVertex))
            return GRAPH_IO_RETURN_UNKNOWN_ERROR;
        if (graphGetNumberOfNeighbours(pGraph, pVertex, &nbCount))
            return GRAPH_IO_RETURN_UNKNOWN_ERROR;
        WRITE(fp, VERTEX_ADDR_STR, pVertex);
        WRITE(fp, NBCOUNT_STR, nbCount);
        for (j = nbCount; j; --j) {
            if (graphGetNextNeighbour(pGraph, pVertex, &pNeighbour))
                return GRAPH_IO_RETURN_UNKNOWN_ERROR;
            WRITE(fp, NB_ADDR_STR, pNeighbour);
        }
        WRITENL(fp, "");
    }
    return GRAPH_IO_RETURN_OK;
}

GraphIoReturnID graphRead(Graph **ppGraph, FILE *fp,
    int (*readVertex)(FILE *fp, void **ppVertex),
    int (*cmpVertices)(void *a, void *b),
    void (*freeVertex)(void *v))
{
    unsigned int version;
    unsigned long vCount, nbCount, i, j;
    char typeChar;
    GraphReturnID graphId;
    MapReturnID mapId;
    GraphEdgeType type;
    Graph *pGraph = NULL;
    Map *pMap = NULL;
    void *pVertexAddress, *pNeighbourAddress,
        *pVertexItem, *pNeighbourItem;
    if (ppGraph == NULL || readVertex == NULL)
        return GRAPH_IO_RETURN_INVALID_PARAMETER;
    /* File format version */
    READ(VERSION_STR, &version);
    if (version != FILE_FORMAT_VERSION)
        return GRAPH_IO_RETURN_DEPRECATED_FILE_FORMAT;
    /* Graph edge type */
    READ(TYPE_STR, &typeChar);
    if ((type = _charToType(typeChar)) == GRAPH_EDGE_TYPE_INVALID)
        return GRAPH_IO_RETURN_CORRUPTED_FILE_FORMAT;
    /* Vertex count */
    READ(VERTICES_STR, &vCount);
    /* Map allocation: keys = addresses, values = vertex */
    if (mapId = mapCreate(&pMap, NULL, _freeEntry, freeVertex)) {
        if (mapId == MAP_RETURN_MEMORY)
            return GRAPH_IO_RETURN_MEMORY;
        return GRAPH_IO_RETURN_UNKNOWN_ERROR;
    }
    /* Graph to be created */
    if (graphId = graphCreate(&pGraph, type, cmpVertices, freeVertex)) {
        mapDestroy(pMap);
        if (graphId == GRAPH_RETURN_MEMORY)
            return GRAPH_IO_RETURN_MEMORY;
        return GRAPH_IO_RETURN_UNKNOWN_ERROR;
    }
    for (i = vCount; i; --i) {
        void *pPreviousValue = NULL;
        READ(VERTEX_ADDR_STR, &pVertexAddress);
        if (readVertex(fp, &pVertexItem)) {
            _housekeep(pGraph, pMap);
            return GRAPH_IO_RETURN_CREATION_FAILURE;
        }
        switch (mapPutEntry(pMap, pVertexAddress, pVertexItem, &pPreviousValue)) {
        case MAP_RETURN_OK:
            /* New entry (new address) */
            break;
        case MAP_RETURN_OVERWROTE_ENTRY:
            /* Repeated vertex address */
            freeVertex(pPreviousValue);
            _housekeep(pGraph, pMap);
            return GRAPH_IO_RETURN_CORRUPTED_FILE_FORMAT;
        case MAP_RETURN_MEMORY:
            _housekeep(pGraph, pMap);
            return GRAPH_IO_RETURN_MEMORY;
        default:
            _housekeep(pGraph, pMap);
            return GRAPH_IO_RETURN_UNKNOWN_ERROR;
        }
        if (graphId = graphAddVertex(pGraph, pVertexItem)) {
            _housekeep(pGraph, pMap);
            switch (graphId) {
            case GRAPH_RETURN_MEMORY:
                return GRAPH_IO_RETURN_MEMORY;
            case GRAPH_RETURN_CONTAINS_VERTEX:
                /* Repeated vertex */
                return GRAPH_IO_RETURN_SAME_CREATION;
            default:
                return GRAPH_IO_RETURN_UNKNOWN_ERROR;
            }
        }
    }
    for (i = vCount; i; --i) {
        READ(VERTEX_ADDR_STR, &pVertexAddress);
        if (mapId = mapGetEntry(pMap, pVertexAddress, &pVertexItem)) {
            _housekeep(pGraph, pMap);
            return GRAPH_IO_RETURN_UNKNOWN_ERROR;
        }
        READ(NBCOUNT_STR, &nbCount);
        for (j = nbCount; j; --j) {
            READ(NB_ADDR_STR, &pNeighbourAddress);
            if (mapId = mapGetEntry(pMap, pNeighbourAddress, &pNeighbourItem)) {
                _housekeep(pGraph, pMap);
                return GRAPH_IO_RETURN_UNKNOWN_ERROR;
            }
            if (graphId = graphAddEdge(pGraph, pVertexItem, pNeighbourItem)) {
                _housekeep(pGraph, pMap);
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

/* Private functions */

// Converts character to type
static GraphEdgeType _charToType(const char c)
{
    char label;
    struct EdgeType *pEdgeType = edgeTypes;
    for (; label = pEdgeType->label; ++pEdgeType)
        if (label == c)
            return pEdgeType->type;
    return GRAPH_EDGE_TYPE_INVALID;
}

// Converts type to character
static char _typeToChar(GraphEdgeType type)
{
    struct EdgeType *pEdgeType = edgeTypes;
    for (; pEdgeType->label; ++pEdgeType)
        if (pEdgeType->type == type)
            return pEdgeType->label;
    return 'I';
}

static void _housekeep(Graph *pGraph, Map *pMap)
{
    if (pGraph)
        graphDestroy(pGraph);
    if (pMap)
        mapDestroy(pMap);
}

static void _freeEntry(void *address, void *data, void (*freeVertex)(void *v))
{
    if (freeVertex)
        freeVertex(data);
}