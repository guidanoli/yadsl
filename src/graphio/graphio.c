#include <graphio/graphio.h>

#include <yadsl/posixstring.h>
#include <assert.h>
#include <stdlib.h>

#include <map/map.h>
#include <memdb/memdb.h>

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

static GraphIoRet _graphWrite(yadsl_GraphHandle *pGraph, FILE *fp,
	int (*writeVertex)(FILE *fp, void *v),
	int (*writeEdge)(FILE *fp, void *e),
	Map *addressMap);

static GraphIoRet _graphRead(yadsl_GraphHandle *pGraph, void **addressMap,
	FILE *fp, size_t vCount, int is_directed,
	int (*readVertex)(FILE *fp, void **ppVertex),
	int (*readEdge)(FILE *fp, void **ppEdge),
	void (*free_vertex_func)(void *v),
	void (*free_edge_func)(void *e));

/* Public functions */

GraphIoRet graphWrite(yadsl_GraphHandle *pGraph, FILE *fp,
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

GraphIoRet graphRead(yadsl_GraphHandle **ppGraph, FILE *fp,
	int (*readVertex)(FILE *fp, void **ppVertex),
	int (*readEdge)(FILE *fp, void **ppEdge),
	int (*cmp_vertices_func)(void *a, void *b),
	void (*free_vertex_func)(void *v),
	int (*cmp_edges_func)(void *a, void *b),
	void (*free_edge_func)(void *e))
{
	unsigned int version;
	size_t vCount;
	GraphIoRet id;
	int is_directed;
	yadsl_GraphHandle *pGraph = NULL;
	void **addressMap;
	READ(fp, VERSION_STR, &version); // Version
	if (version != FILE_FORMAT_VERSION)
		return GRAPH_IO_DEPRECATED_FILE_FORMAT;
	READ(fp, DIRECTED_STR, &is_directed); // Directed
	READ(fp, VCOUNT_STR, &vCount); // Vertex count
	/* Graph to be created */
	if (!(pGraph = yadsl_graph_create(is_directed, cmp_vertices_func,
		free_vertex_func, cmp_edges_func, free_edge_func))) {
		return GRAPH_IO_MEMORY;
	}
	/* Address map to store vertex items */
	addressMap = malloc(vCount * sizeof(void *));
	if (addressMap == NULL) {
		yadsl_graph_destroy(pGraph);
		return GRAPH_IO_MEMORY;
	}
	id = _graphRead(pGraph, addressMap, fp, vCount, is_directed,
		readVertex, readEdge, free_vertex_func, free_edge_func);
	free(addressMap);
	if (id) {
		yadsl_graph_destroy(pGraph);
		return id;
	}
	*ppGraph = pGraph;
	return GRAPH_IO_OK;
}

/* Private functions */

static GraphIoRet _graphWrite(yadsl_GraphHandle *pGraph, FILE *fp,
	int (*writeVertex)(FILE *fp, void *v),
	int (*writeEdge)(FILE *fp, void *e),
	Map *addressMap)
{
	bool is_directed;
	MapRet mapId;
	void *previousValue;
	size_t vCount, nbCount, i, j, index;
	void *pVertex, *pNeighbour, *pEdge;
	if (yadsl_graph_is_directed_check(pGraph, &is_directed)) assert(0);
	if (yadsl_graph_vertex_count_get(pGraph, &vCount)) assert(0);
	WRITENL(fp, VERSION_STR, FILE_FORMAT_VERSION); // Version
	WRITENL(fp, DIRECTED_STR, is_directed); // Type
	WRITE(fp, VCOUNT_STR, vCount); // Vertex count
	for (i = 0; i < vCount; ++i) {
		int flag, overwrite;
		if (yadsl_graph_vertex_iter(pGraph, YADSL_GRAPH_ITER_DIR_NEXT, &pVertex)) assert(0);
		if (mapId = mapPutEntry(addressMap, pVertex, i,
			&overwrite, &previousValue)) {
			assert(!overwrite);
			return GRAPH_IO_MEMORY;
		}
		if (writeVertex(fp, pVertex)) // Vertex item
			return GRAPH_IO_WRITING_FAILURE;
		if (yadsl_graph_vertex_flag_get(pGraph, pVertex, &flag)) assert(0);
		WRITE(fp, VFLAG_STR, flag);
	}
	WRITE(fp, "%c", '\n');
	for (i = vCount; i; --i) {
		if (yadsl_graph_vertex_iter(pGraph, YADSL_GRAPH_ITER_DIR_NEXT, &pVertex)) assert(0);
		if (yadsl_graph_vertex_degree_get(pGraph, pVertex, YADSL_GRAPH_EDGE_DIR_OUT, &nbCount)) assert(0);
		WRITE(fp, NBCOUNT_STR, nbCount); // Vertex degree
		for (j = nbCount; j; --j) {
			if (yadsl_graph_vertex_nb_iter(pGraph, pVertex, YADSL_GRAPH_EDGE_DIR_OUT, YADSL_GRAPH_ITER_DIR_NEXT, &pNeighbour, &pEdge))
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

static GraphIoRet _graphRead(yadsl_GraphHandle *pGraph, void **addressMap,
	FILE *fp, size_t vCount, int is_directed,
	int (*readVertex)(FILE *fp, void **ppVertex),
	int (*readEdge)(FILE *fp, void **ppEdge),
	void (*free_vertex_func)(void *v),
	void (*free_edge_func)(void *e))
{
	size_t nbCount, i, j, index;
	yadsl_GraphRet graphId;
	void *pVertexItem, *pNeighbourItem, *pEdgeItem;
	for (i = 0; i < vCount; ++i) {
		void *pPreviousValue = NULL;
		int flag;
		if (readVertex(fp, &pVertexItem)) // Vertex item
			return GRAPH_IO_CREATION_FAILURE;
		addressMap[i] = pVertexItem;
		if (graphId = yadsl_graph_vertex_add(pGraph, pVertexItem)) {
			if (free_vertex_func)
				free_vertex_func(pVertexItem);
			switch (graphId) {
			case YADSL_GRAPH_RET_MEMORY:
				return GRAPH_IO_MEMORY;
			case YADSL_GRAPH_RET_CONTAINS_VERTEX:
				return GRAPH_IO_SAME_CREATION;
			default:
				assert(0);
			}
		}
		READ(fp, VFLAG_STR, &flag);
		if (yadsl_graph_vertex_flag_set(pGraph, pVertexItem, flag)) assert(0);
	}
	for (i = 0; i < vCount; ++i) {
		pVertexItem = addressMap[i];
		READ(fp, NBCOUNT_STR, &nbCount); // Vertex degree
		for (j = 0; j < nbCount; ++j) {
			READ(fp, NB_IDX_STR, &index); // Neighbour index
			pNeighbourItem = addressMap[index];
			if (readEdge(fp, &pEdgeItem)) // Edge item
				return GRAPH_IO_CREATION_FAILURE;
			if (graphId = yadsl_graph_edge_add(pGraph, pVertexItem, pNeighbourItem,
				pEdgeItem)) {
				if (free_edge_func)
					free_edge_func(pEdgeItem);
				switch (graphId) {
				case YADSL_GRAPH_RET_MEMORY:
					return GRAPH_IO_MEMORY;
				case YADSL_GRAPH_RET_CONTAINS_EDGE:
					return GRAPH_IO_CORRUPTED_FILE_FORMAT;
				default:
					assert(0);
				}
			}
		}
	}
	return GRAPH_IO_OK;
}
