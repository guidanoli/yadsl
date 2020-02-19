#include "graphio.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "map.h"

#define WRITE(file, format, arg) do { \
	if (fprintf(file, format, arg) < 0) \
		return GRAPH_IO_RETURN_FILE_ERROR; \
} while(0)

#define READ(file, format, arg) do { \
	if (fscanf(fp, " " format " ", arg) != 1) \
		return GRAPH_IO_RETURN_FILE_ERROR; \
} while(0)

#define WRITENL(file, format, arg) \
	WRITE(file, format "\n", arg)

#define FILE_FORMAT_VERSION 5

#define VERSION_STR     "VERSION %u"
#define DIRECTED_STR    "IS_DIRECTED %d"
#define VCOUNT_STR      "%lu "
#define VFLAG_STR       " %d "
#define VERTEX_IDX_STR  "%lu "
#define NBCOUNT_STR	    "%lu"
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
	READ(fp, VERSION_STR, &version); // Version
	if (version != FILE_FORMAT_VERSION)
		return GRAPH_IO_RETURN_DEPRECATED_FILE_FORMAT;
	READ(fp, DIRECTED_STR, &isDirected); // Directed
	READ(fp, VCOUNT_STR, &vCount); // Vertex count
	/* Graph to be created */
	if (graphId = graphCreate(&pGraph, isDirected, cmpVertices,
		freeVertex, cmpEdges, freeEdge)) {
		assert(graphId == GRAPH_RETURN_MEMORY);
		return GRAPH_IO_RETURN_MEMORY;
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
	void *previousValue;
	unsigned long vCount, nbCount, i, j, index;
	void *pVertex, *pNeighbour, *pEdge;
	assert(!graphIsDirected(pGraph, &isDirected));
	assert(!graphGetNumberOfVertices(pGraph, &vCount));
	WRITENL(fp, VERSION_STR, FILE_FORMAT_VERSION); // Version
	WRITENL(fp, DIRECTED_STR, isDirected); // Type
	WRITE(fp, VCOUNT_STR, vCount); // Vertex count
	for (i = 0; i < vCount; ++i) {
		int flag;
		assert(!graphGetNextVertex(pGraph, &pVertex));
		if (mapId = mapPutEntry(addressMap, pVertex, i, &previousValue)) {
			assert(mapId == MAP_RETURN_MEMORY);
			return GRAPH_IO_RETURN_MEMORY;
		}
		if (writeVertex(fp, pVertex)) // Vertex item
			return GRAPH_IO_RETURN_WRITING_FAILURE;
		assert(!graphGetVertexFlag(pGraph, pVertex, &flag));
		WRITE(fp, VFLAG_STR, flag);
	}
	WRITE(fp, "%c", '\n');
	for (i = vCount; i; --i) {
		assert(!graphGetNextVertex(pGraph, &pVertex));
		assert(!graphGetVertexOutDegree(pGraph, pVertex, &nbCount));
		WRITE(fp, NBCOUNT_STR, nbCount); // Vertex degree
		for (j = nbCount; j; --j) {
			assert(!graphGetNextOutNeighbour(pGraph, pVertex, &pNeighbour,
				&pEdge));
			assert(!mapGetEntry(addressMap, pNeighbour, &previousValue));
			index = (unsigned long) previousValue;
			WRITE(fp, NB_IDX_STR, index); // Neighbour index
			if (writeEdge(fp, pEdge)) // Edge item
				return GRAPH_IO_RETURN_WRITING_FAILURE;
		}
		WRITE(fp, "%c", '\n');
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
		int flag;
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
				assert(0);
			}
		}
		READ(fp, VFLAG_STR, &flag);
		assert(!graphSetVertexFlag(pGraph, pVertexItem, flag));
	}
	for (i = 0; i < vCount; ++i) {
		pVertexItem = addressMap[i];
		READ(fp, NBCOUNT_STR, &nbCount); // Vertex degree
		for (j = 0; j < nbCount; ++j) {
			READ(fp, NB_IDX_STR, &index); // Neighbour index
			pNeighbourItem = addressMap[index];
			if (readEdge(fp, &pEdgeItem)) // Edge item
				return GRAPH_IO_RETURN_CREATION_FAILURE;
			if (graphId = graphAddEdge(pGraph, pVertexItem, pNeighbourItem,
				pEdgeItem)) {
				if (freeEdge)
					freeEdge(pEdgeItem);
				switch (graphId) {
				case GRAPH_RETURN_MEMORY:
					return GRAPH_IO_RETURN_MEMORY;
				case GRAPH_RETURN_CONTAINS_EDGE:
					return GRAPH_IO_RETURN_CORRUPTED_FILE_FORMAT;
				default:
					assert(0);
				}
			}
		}
	}
	return GRAPH_IO_RETURN_OK;
}
