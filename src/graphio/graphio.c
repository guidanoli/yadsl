#include "graphio.h"

#include "posixstring.h"
#include <assert.h>
#include <stdlib.h>

#include "map.h"
#include "memdb.h"

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
# pragma warning(disable : 4022)
#endif

#define WRITE(file, format, arg) do { \
	if (fprintf(file, format, arg) < 0) \
		return GRAPH_IO_FILE_ERROR; \
} while(0)

#define READ(file, format, arg) do { \
	if (fscanf(fp, " " format " ", arg) != 1) \
		return GRAPH_IO_FILE_ERROR; \
} while(0)

#define WRITENL(file, format, arg) \
	WRITE(file, format "\n", arg)

#define FILE_FORMAT_VERSION 5

#define VERSION_STR     "VERSION %u"
#define DIRECTED_STR    "IS_DIRECTED %d"
#define VCOUNT_STR      "%zu "
#define VFLAG_STR       " %d "
#define VERTEX_IDX_STR  "%zu "
#define NBCOUNT_STR	    "%zu"
#define NB_IDX_STR      " %zu "

/* Private functions prototypes */

static GraphIoRet _graphWrite(Graph *pGraph, FILE *fp,
	int (*writeVertex)(FILE *fp, void *v),
	int (*writeEdge)(FILE *fp, void *e),
	Map *addressMap);

static GraphIoRet _graphRead(Graph *pGraph, void **addressMap,
	FILE *fp, size_t vCount, int isDirected,
	int (*readVertex)(FILE *fp, void **ppVertex),
	int (*readEdge)(FILE *fp, void **ppEdge),
	void (*freeVertex)(void *v),
	void (*freeEdge)(void *e));

/* Public functions */

GraphIoRet graphWrite(Graph *pGraph, FILE *fp,
	int (*writeVertex)(FILE *fp, void *v),
	int (*writeEdge)(FILE *fp, void *e))
{
	Map *addressMap;
	GraphIoRet id;
	if (mapCreate(&addressMap, NULL, NULL, NULL))
		return GRAPH_IO_MEMORY;
	id = _graphWrite(pGraph, fp, writeVertex, writeEdge, addressMap);
	mapDestroy(addressMap);
	return id;
}

GraphIoRet graphRead(Graph **ppGraph, FILE *fp,
	int (*readVertex)(FILE *fp, void **ppVertex),
	int (*readEdge)(FILE *fp, void **ppEdge),
	int (*cmpVertices)(void *a, void *b),
	void (*freeVertex)(void *v),
	int (*cmpEdges)(void *a, void *b),
	void (*freeEdge)(void *e))
{
	unsigned int version;
	size_t vCount;
	GraphRet graphId;
	GraphIoRet id;
	int isDirected;
	Graph *pGraph = NULL;
	void **addressMap;
	READ(fp, VERSION_STR, &version); // Version
	if (version != FILE_FORMAT_VERSION)
		return GRAPH_IO_DEPRECATED_FILE_FORMAT;
	READ(fp, DIRECTED_STR, &isDirected); // Directed
	READ(fp, VCOUNT_STR, &vCount); // Vertex count
	/* Graph to be created */
	if (graphId = graphCreate(&pGraph, isDirected, cmpVertices,
		freeVertex, cmpEdges, freeEdge)) {
		assert(graphId == GRAPH_MEMORY);
		return GRAPH_IO_MEMORY;
	}
	/* Address map to store vertex items */
	addressMap = malloc(vCount * sizeof(void *));
	if (addressMap == NULL) {
		graphDestroy(pGraph);
		return GRAPH_IO_MEMORY;
	}
	id = _graphRead(pGraph, addressMap, fp, vCount, isDirected,
		readVertex, readEdge, freeVertex, freeEdge);
	free(addressMap);
	if (id) {
		graphDestroy(pGraph);
		return id;
	}
	*ppGraph = pGraph;
	return GRAPH_IO_OK;
}

/* Private functions */

static GraphIoRet _graphWrite(Graph *pGraph, FILE *fp,
	int (*writeVertex)(FILE *fp, void *v),
	int (*writeEdge)(FILE *fp, void *e),
	Map *addressMap)
{
	int isDirected;
	MapRet mapId;
	void *previousValue;
	size_t vCount, nbCount, i, j, index;
	void *pVertex, *pNeighbour, *pEdge;
	if (graphIsDirected(pGraph, &isDirected)) assert(0);
	if (graphGetNumberOfVertices(pGraph, &vCount)) assert(0);
	WRITENL(fp, VERSION_STR, FILE_FORMAT_VERSION); // Version
	WRITENL(fp, DIRECTED_STR, isDirected); // Type
	WRITE(fp, VCOUNT_STR, vCount); // Vertex count
	for (i = 0; i < vCount; ++i) {
		int flag, overwrite;
		if (graphGetNextVertex(pGraph, &pVertex)) assert(0);
		if (mapId = mapPutEntry(addressMap, pVertex, i,
			&overwrite, &previousValue)) {
			assert(!overwrite);
			return GRAPH_IO_MEMORY;
		}
		if (writeVertex(fp, pVertex)) // Vertex item
			return GRAPH_IO_WRITING_FAILURE;
		if (graphGetVertexFlag(pGraph, pVertex, &flag)) assert(0);
		WRITE(fp, VFLAG_STR, flag);
	}
	WRITE(fp, "%c", '\n');
	for (i = vCount; i; --i) {
		if (graphGetNextVertex(pGraph, &pVertex)) assert(0);
		if (graphGetVertexOutDegree(pGraph, pVertex, &nbCount)) assert(0);
		WRITE(fp, NBCOUNT_STR, nbCount); // Vertex degree
		for (j = nbCount; j; --j) {
			if (graphGetNextOutNeighbour(pGraph, pVertex, &pNeighbour, &pEdge))
				assert(0);
			if (mapGetEntry(addressMap, pNeighbour, &previousValue)) assert(0);
			index = (size_t) previousValue;
			WRITE(fp, NB_IDX_STR, index); // Neighbour index
			if (writeEdge(fp, pEdge)) // Edge item
				return GRAPH_IO_WRITING_FAILURE;
		}
		WRITE(fp, "%c", '\n');
	}
	return GRAPH_IO_OK;
}

static GraphIoRet _graphRead(Graph *pGraph, void **addressMap,
	FILE *fp, size_t vCount, int isDirected,
	int (*readVertex)(FILE *fp, void **ppVertex),
	int (*readEdge)(FILE *fp, void **ppEdge),
	void (*freeVertex)(void *v),
	void (*freeEdge)(void *e))
{
	size_t nbCount, i, j, index;
	GraphRet graphId;
	void *pVertexItem, *pNeighbourItem, *pEdgeItem;
	for (i = 0; i < vCount; ++i) {
		void *pPreviousValue = NULL;
		int flag;
		if (readVertex(fp, &pVertexItem)) // Vertex item
			return GRAPH_IO_CREATION_FAILURE;
		addressMap[i] = pVertexItem;
		if (graphId = graphAddVertex(pGraph, pVertexItem)) {
			if (freeVertex)
				freeVertex(pVertexItem);
			switch (graphId) {
			case GRAPH_MEMORY:
				return GRAPH_IO_MEMORY;
			case GRAPH_CONTAINS_VERTEX:
				return GRAPH_IO_SAME_CREATION;
			default:
				assert(0);
			}
		}
		READ(fp, VFLAG_STR, &flag);
		if (graphSetVertexFlag(pGraph, pVertexItem, flag)) assert(0);
	}
	for (i = 0; i < vCount; ++i) {
		pVertexItem = addressMap[i];
		READ(fp, NBCOUNT_STR, &nbCount); // Vertex degree
		for (j = 0; j < nbCount; ++j) {
			READ(fp, NB_IDX_STR, &index); // Neighbour index
			pNeighbourItem = addressMap[index];
			if (readEdge(fp, &pEdgeItem)) // Edge item
				return GRAPH_IO_CREATION_FAILURE;
			if (graphId = graphAddEdge(pGraph, pVertexItem, pNeighbourItem,
				pEdgeItem)) {
				if (freeEdge)
					freeEdge(pEdgeItem);
				switch (graphId) {
				case GRAPH_MEMORY:
					return GRAPH_IO_MEMORY;
				case GRAPH_CONTAINS_EDGE:
					return GRAPH_IO_CORRUPTED_FILE_FORMAT;
				default:
					assert(0);
				}
			}
		}
	}
	return GRAPH_IO_OK;
}
