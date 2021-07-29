#include <graphio/graphio.h>

#include <string.h>
#include <assert.h>

#include <yadsl/stdlib.h>

#include <map/map.h>

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
# pragma warning(disable : 4022)
#endif

#define YADSL_GRAPHIO_WRITE(file, format, arg) do { \
	if (fprintf(file, format, arg) < 0) \
		return YADSL_GRAPHIO_RET_FILE_ERROR; \
} while(0)

#define YADSL_GRAPHIO_WRITE_NL(file, format, arg) \
	YADSL_GRAPHIO_WRITE(file, format "\n", arg)

#define YADSL_GRAPHIO_READ(file, format, arg) do { \
	if (fscanf(file, " " format " ", arg) != 1) \
		return YADSL_GRAPHIO_RET_FILE_ERROR; \
} while(0)

#define YADSL_GRAPHIO_FILE_FORMAT_VERSION 5

#define YADSL_GRAPHIO_VERSION_STR     "VERSION %u"
#define YADSL_GRAPHIO_DIRECTED_STR    "IS_DIRECTED %d"
#define YADSL_GRAPHIO_VCOUNT_STR      "%zu "
#define YADSL_GRAPHIO_VFLAG_STR       " %d "
#define YADSL_GRAPHIO_VERTEX_IDX_STR  "%zu "
#define YADSL_GRAPHIO_NBCOUNT_STR	   "%zu"
#define YADSL_GRAPHIO_NB_IDX_STR      " %zu "

/* Private functions prototypes */

static yadsl_GraphIoRet yadsl_graphio_write_internal(
	yadsl_GraphHandle* graph,
	FILE* file_ptr,
	yadsl_GraphIoVertexWriteFunc write_vertex_func,
	yadsl_GraphIoEdgeWriteFunc write_edge_func,
	yadsl_MapHandle* address_map);

static yadsl_GraphIoRet yadsl_graphio_read_internal(
	yadsl_GraphHandle* graph,
	void** address_map,
	FILE* file_ptr,
	size_t vertex_count,
	int is_directed,
	yadsl_GraphIoVertexReadFunc read_vertex_func,
	yadsl_GraphIoEdgeReadFunc read_edge_func,
	yadsl_GraphFreeVertexObjFunc free_vertex_func,
	yadsl_GraphFreeEdgeObjFunc free_edge_func);

/* Public functions */

yadsl_GraphIoRet yadsl_graphio_write(
	yadsl_GraphHandle* graph,
	FILE* file_ptr,
	yadsl_GraphIoVertexWriteFunc write_vertex_func,
	yadsl_GraphIoEdgeWriteFunc write_edge_func)
{
	yadsl_MapHandle* adress_map;
	yadsl_GraphIoRet ret;
	if (!(adress_map = yadsl_map_create(NULL, NULL, NULL, NULL)))
		return YADSL_GRAPHIO_RET_MEMORY;
	ret = yadsl_graphio_write_internal(graph, file_ptr, write_vertex_func, write_edge_func, adress_map);
	yadsl_map_destroy(adress_map);
	return ret;
}

yadsl_GraphIoRet yadsl_graphio_read(
	FILE* file_ptr,
	yadsl_GraphIoVertexReadFunc read_vertex_func,
	yadsl_GraphIoEdgeReadFunc read_edge_func,
	yadsl_GraphCmpVertexObjsFunc cmp_vertices_func,
	yadsl_GraphFreeVertexObjFunc free_vertex_func,
	yadsl_GraphCmpEdgeObjsFunc cmp_edges_func,
	yadsl_GraphFreeEdgeObjFunc free_edge_func,
	yadsl_GraphHandle** graph_ptr)
{
	yadsl_GraphIoRet ret;
	unsigned int version;
	size_t vertex_count;
	int is_directed;
	yadsl_GraphHandle* graph;
	void** address_map;

	YADSL_GRAPHIO_READ(file_ptr, YADSL_GRAPHIO_VERSION_STR, &version); /* Version */
	if (version != YADSL_GRAPHIO_FILE_FORMAT_VERSION)
		return YADSL_GRAPHIO_RET_DEPRECATED_FILE_FORMAT;
	YADSL_GRAPHIO_READ(file_ptr, YADSL_GRAPHIO_DIRECTED_STR, &is_directed); /* Directed */
	YADSL_GRAPHIO_READ(file_ptr, YADSL_GRAPHIO_VCOUNT_STR, &vertex_count); /* Vertex count */

	/* Graph to be created */
	if (!(graph = yadsl_graph_create(is_directed, cmp_vertices_func, free_vertex_func, cmp_edges_func, free_edge_func)))
		return YADSL_GRAPHIO_RET_MEMORY;

	/* Address map to store vertex items */
	address_map = malloc(vertex_count * sizeof(void*));
	if (address_map == NULL) {
		yadsl_graph_destroy(graph);
		return YADSL_GRAPHIO_RET_MEMORY;
	}

	ret = yadsl_graphio_read_internal(graph, address_map, file_ptr, vertex_count, is_directed, read_vertex_func, read_edge_func, free_vertex_func, free_edge_func);
	free(address_map);
	if (ret) {
		yadsl_graph_destroy(graph);
		return ret;
	}
	*graph_ptr = graph;
	return YADSL_GRAPHIO_RET_OK;
}

/* Private functions */

static yadsl_GraphIoRet yadsl_graphio_write_internal(
	yadsl_GraphHandle* graph,
	FILE* file_ptr,
	yadsl_GraphIoVertexWriteFunc write_vertex_func,
	yadsl_GraphIoEdgeWriteFunc write_edge_func,
	yadsl_MapHandle* address_map)
{
	bool is_directed;
	yadsl_MapRet map_ret;
	void* previous_value;
	size_t vertex_count, nb_count, i, j, index;
	void* vertex, * nb, * edge;

	if (yadsl_graph_is_directed_check(graph, &is_directed)) assert(0);
	if (yadsl_graph_vertex_count_get(graph, &vertex_count)) assert(0);

	YADSL_GRAPHIO_WRITE_NL(file_ptr, YADSL_GRAPHIO_VERSION_STR, YADSL_GRAPHIO_FILE_FORMAT_VERSION); /* Version */
	YADSL_GRAPHIO_WRITE_NL(file_ptr, YADSL_GRAPHIO_DIRECTED_STR, is_directed); /* Directed */
	YADSL_GRAPHIO_WRITE(file_ptr, YADSL_GRAPHIO_VCOUNT_STR, vertex_count); /* Vertex count */

	for (i = 0; i < vertex_count; ++i) {
		int flag;
		bool overwrite;
		if (yadsl_graph_vertex_iter(graph, YADSL_GRAPH_ITER_DIR_NEXT, &vertex)) assert(0);
		if (map_ret = yadsl_map_entry_add(address_map, vertex, (yadsl_MapEntryValue*) i, &overwrite, &previous_value)) {
			assert(!overwrite);
			return YADSL_GRAPHIO_RET_MEMORY;
		}
		if (write_vertex_func(file_ptr, vertex)) /* Vertex item */
			return YADSL_GRAPHIO_RET_WRITING_FAILURE;
		if (yadsl_graph_vertex_flag_get(graph, vertex, &flag)) assert(0);
		YADSL_GRAPHIO_WRITE(file_ptr, YADSL_GRAPHIO_VFLAG_STR, flag);
	}

	YADSL_GRAPHIO_WRITE(file_ptr, "%c", '\n');
	for (i = vertex_count; i; --i) {
		if (yadsl_graph_vertex_iter(graph, YADSL_GRAPH_ITER_DIR_NEXT, &vertex)) assert(0);
		if (yadsl_graph_vertex_degree_get(graph, vertex, YADSL_GRAPH_EDGE_DIR_OUT, &nb_count)) assert(0);
		YADSL_GRAPHIO_WRITE(file_ptr, YADSL_GRAPHIO_NBCOUNT_STR, nb_count); /* Vertex degree */
		for (j = nb_count; j; --j) {
			if (yadsl_graph_vertex_nb_iter(graph, vertex, YADSL_GRAPH_EDGE_DIR_OUT, YADSL_GRAPH_ITER_DIR_NEXT, &nb, &edge))
				assert(0);
			if (yadsl_map_entry_get(address_map, nb, &previous_value)) assert(0);
			index = (size_t) previous_value;
			YADSL_GRAPHIO_WRITE(file_ptr, YADSL_GRAPHIO_NB_IDX_STR, index); /* Neighbour index */
			if (write_edge_func(file_ptr, edge)) /* Edge item */
				return YADSL_GRAPHIO_RET_WRITING_FAILURE;
		}
		YADSL_GRAPHIO_WRITE(file_ptr, "%c", '\n');
	}
	return YADSL_GRAPHIO_RET_OK;
}

static yadsl_GraphIoRet yadsl_graphio_read_internal(
	yadsl_GraphHandle* graph,
	void** address_map,
	FILE* file_ptr,
	size_t vertex_count,
	int is_directed,
	yadsl_GraphIoVertexReadFunc read_vertex_func,
	yadsl_GraphIoEdgeReadFunc read_edge_func,
	yadsl_GraphFreeVertexObjFunc free_vertex_func,
	yadsl_GraphFreeEdgeObjFunc free_edge_func)
{
	size_t nb_count, i, j, index;
	yadsl_GraphRet graph_ret;
	void* vertex_item, * nb_item, * edge_item;

	for (i = 0; i < vertex_count; ++i) {
		void* pPreviousValue = NULL;
		int flag;
		if (read_vertex_func(file_ptr, &vertex_item)) /* Vertex item */
			return YADSL_GRAPHIO_RET_CREATION_FAILURE;
		address_map[i] = vertex_item;
		if (graph_ret = yadsl_graph_vertex_add(graph, vertex_item)) {
			if (free_vertex_func)
				free_vertex_func(vertex_item);
			switch (graph_ret) {
			case YADSL_GRAPH_RET_MEMORY:
				return YADSL_GRAPHIO_RET_MEMORY;
			case YADSL_GRAPH_RET_CONTAINS_VERTEX:
				return YADSL_GRAPHIO_RET_SAME_CREATION;
			default:
				assert(0);
			}
		}
		YADSL_GRAPHIO_READ(file_ptr, YADSL_GRAPHIO_VFLAG_STR, &flag);
		if (yadsl_graph_vertex_flag_set(graph, vertex_item, flag)) assert(0);
	}

	for (i = 0; i < vertex_count; ++i) {
		vertex_item = address_map[i];
		YADSL_GRAPHIO_READ(file_ptr, YADSL_GRAPHIO_NBCOUNT_STR, &nb_count); /* Vertex degree */
		for (j = 0; j < nb_count; ++j) {
			YADSL_GRAPHIO_READ(file_ptr, YADSL_GRAPHIO_NB_IDX_STR, &index); /* Neighbour index */
			nb_item = address_map[index];
			if (read_edge_func(file_ptr, &edge_item)) /* Edge item */
				return YADSL_GRAPHIO_RET_CREATION_FAILURE;
			if (graph_ret = yadsl_graph_edge_add(graph, vertex_item, nb_item,
				edge_item)) {
				if (free_edge_func)
					free_edge_func(edge_item);
				switch (graph_ret) {
				case YADSL_GRAPH_RET_MEMORY:
					return YADSL_GRAPHIO_RET_MEMORY;
				case YADSL_GRAPH_RET_CONTAINS_EDGE:
					return YADSL_GRAPHIO_RET_CORRUPTED_FILE_FORMAT;
				default:
					assert(0);
				}
			}
		}
	}
	return YADSL_GRAPHIO_RET_OK;
}
