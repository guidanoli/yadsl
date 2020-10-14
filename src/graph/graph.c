#include <graph/graph.h>

#include <assert.h>
#include <stdarg.h>

#ifdef YADSL_DEBUG
#include <memdb/memdb.h>
#else
#include <stdlib.h>
#endif

#include <set/set.h>

/*******************************************************************************
* Graph data structure invariants
********************************************************************************
*
* I) Given an edge e and vertex u, it is always true that:
* - e->source points to u, iff u->out_edges contain e
* - e->destination points to u, iff u->in_edges contain e
*
* II) If the graph is undirected, an edge always comes from the vertex
* of smallest address to the one with largest address.
*
* III) The ownership of the set cursors are given to specific functions:
* 	- yadsl_Graph::vertex_set -> yadsl_graph_get_*_vertex
* 	- yadsl_GraphVertex::out_edges -> yadsl_graph_get_*_total_nb_iter /
*                                     yadsl_graph_get_*_out_nb_iter
* 	- yadsl_GraphVertex::inEdge -> yadsl_graph_get_*_total_nb_iter /
*                                  yadsl_graph_get_*_in_nb_iter
*   Where * stands for either 'next' or 'previous'
*
* IV) yadsl_GraphVertex::in_edges_to_iterate must always be in between 0 and the
* number of edges from which the vertex is DESTINATION
*
* V) yadsl_GraphVertex::out_edges_to_iterate must always be in between 0 and the
* number of edges from which the vertex is SOURCE
*
* VI) yadsl_GraphVertex::out_edges_to_iterate must never be lower than
* yadsl_GraphVertex::in_edges_to_iterate.
*
*******************************************************************************/

typedef struct
{
	bool is_directed; /**< whether graph is directed or not */
	yadsl_SetHandle* vertex_set; /**< set of yadsl_GraphVertex */
	yadsl_GraphCmpVertexObjsFunc cmp_vertices_func; /**< compares yadsl_GraphVertex::item */
	yadsl_GraphCmpEdgeObjsFunc cmp_edges_func; /**< compares yadsl_GraphEdge::item */
	yadsl_GraphFreeVertexObjFunc free_vertex_func; /**< frees yadsl_GraphVertex::item */
	yadsl_GraphFreeEdgeObjFunc free_edge_func; /**< frees yadsl_GraphEdge::item */
}
yadsl_Graph;

typedef struct
{
	yadsl_GraphVertexObject* item; /**< generic portion of vertex */
	yadsl_GraphVertexFlag flag; /**< flag (for dfs, bfs, coloring...) */
	yadsl_SetHandle* out_edges; /**< edges from which the vertex is SOURCE */
	yadsl_SetHandle* in_edges; /**< edges from which the vertex is DESTINATION */
	size_t out_edges_to_iterate; /**< counter for graphGet*Neighbour */
	size_t in_edges_to_iterate; /**< counter for graphGet*Neighbour */
}
yadsl_GraphVertex;

typedef struct
{
	yadsl_GraphEdgeObject* item; /**< generic portion of edge */
	yadsl_GraphVertex* source; /**< vertex from which the edge comes from */
	yadsl_GraphVertex* destination; /**< vertex to with the edge goes to */
}
yadsl_GraphEdge;

/**********************************/
/*  Private functions prototypes  */
/**********************************/

///////////////////////////////////////////////
// Parameter for yadsl_GraphVertexItemCmpParam
// item         - vertex 
// cmp_vertices_func  - comparison function
//				  between vertices
///////////////////////////////////////////////

typedef struct
{
	yadsl_GraphVertexObject* item;
	yadsl_GraphCmpVertexObjsFunc cmp_vertices_func;
}
yadsl_GraphVertexItemCmpParam;

///////////////////////////////////////////////
// Callbacks
///////////////////////////////////////////////

static bool yadsl_graph_edge_dest_compare_internal(
	yadsl_GraphEdge* edge,
	yadsl_GraphVertex* destination);

static bool yadsl_graph_vertex_item_compare_internal(
	yadsl_GraphVertex* vertex,
	yadsl_GraphVertexItemCmpParam* par);

static void yadsl_graph_vertex_free_internal(
	yadsl_GraphVertex* vertex,
	yadsl_Graph* graph);

static void yadsl_graph_vertex_in_free_internal(
	yadsl_GraphEdge* edge,
	yadsl_GraphFreeEdgeObjFunc free_edge_func);

static void yadsl_graph_vertex_out_free_internal(
	yadsl_GraphEdge* edge,
	yadsl_GraphFreeEdgeObjFunc free_edge_func);

static bool yadsl_graph_vertex_flag_set_internal(
	yadsl_GraphVertex* vertex,
	yadsl_GraphVertexFlag* flag);

///////////////////////////////////////////////
// Internal use
///////////////////////////////////////////////

static yadsl_GraphRet yadsl_graph_vertex_nb_total_next_get_internal(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* curr,
	yadsl_GraphVertexObject** next_nb_ptr,
	yadsl_GraphEdgeObject** edge_ptr);

static yadsl_GraphRet yadsl_graph_vertex_nb_total_prev_get_internal(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* curr,
	yadsl_GraphVertexObject** previous_nb_ptr,
	yadsl_GraphEdgeObject** edge_ptr);

static void yadsl_graph_adj_list_counters_reset_internal(
	yadsl_GraphVertex* vertex,
	int orientation);

static yadsl_GraphRet yadsl_graph_set_cursor_cycle_internal(
	yadsl_SetHandle* set,
	void** value_ptr,
	yadsl_GraphIterationDirection iter_direction);

static yadsl_GraphRet yadsl_graph_vertex_nb_get_internal(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* u,
	yadsl_GraphEdgeDirection edge_direction,
	yadsl_GraphIterationDirection iter_direction,
	yadsl_GraphVertexObject** v_ptr,
	yadsl_GraphEdgeObject** uv_ptr);

static yadsl_GraphRet yadsl_graph_edge_find_internal(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertex* u_vertex,
	yadsl_GraphVertex* v_vertex,
	yadsl_GraphVertex** source_ptr,
	yadsl_GraphVertex** destination_ptr,
	yadsl_GraphEdge** uv_edge_ptr);

#define YADSL_GRAPH_VERTICES_FIND(...) \
yadsl_graph_vertices_find_internal(__VA_ARGS__, NULL, NULL)

static yadsl_GraphRet yadsl_graph_vertices_find_internal(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* item,
	yadsl_GraphVertex** vertex_ptr, ...);

/**********************/
/*  Public functions  */
/**********************/

yadsl_GraphHandle* yadsl_graph_create(
	bool is_directed,
	yadsl_GraphCmpVertexObjsFunc cmp_vertices_func,
	yadsl_GraphFreeVertexObjFunc free_vertex_func,
	yadsl_GraphCmpEdgeObjsFunc cmp_edges_func,
	yadsl_GraphFreeEdgeObjFunc free_edge_func)
{
	yadsl_Graph* graph = malloc(sizeof(*graph));
	if (graph) {
		if (!(graph->vertex_set = yadsl_set_create())) {
			free(graph);
			return NULL;
		}
		graph->is_directed = is_directed;
		graph->free_vertex_func = free_vertex_func;
		graph->free_edge_func = free_edge_func;
		graph->cmp_vertices_func = cmp_vertices_func;
		graph->cmp_edges_func = cmp_edges_func;
	}
	return graph;
}

yadsl_GraphRet yadsl_graph_vertex_count_get(
	yadsl_GraphHandle* graph,
	size_t* size_ptr)
{
	size_t temp;
	if (yadsl_set_size_get(((yadsl_Graph*) graph)->vertex_set, &temp))
		assert(0);
	*size_ptr = temp;
	return YADSL_GRAPH_RET_OK;
}

yadsl_GraphRet yadsl_graph_vertex_iter(
	yadsl_GraphHandle* graph,
	yadsl_GraphIterationDirection iter_direction,
	yadsl_GraphVertexObject** curr_ptr)
{
	size_t vertex_count;
	yadsl_GraphVertex* vertex;
	yadsl_GraphRet graph_ret;
	yadsl_SetHandle* vertex_set = ((yadsl_Graph*) graph)->vertex_set;
	if (yadsl_set_size_get(vertex_set, &vertex_count))
		assert(0);
	if (vertex_count == 0)
		return YADSL_GRAPH_RET_EMPTY;
	if (graph_ret = yadsl_graph_set_cursor_cycle_internal(vertex_set, &vertex, iter_direction))
		return graph_ret;
	*curr_ptr = vertex->item;
	return YADSL_GRAPH_RET_OK;
}

yadsl_GraphRet yadsl_graph_is_directed_check(
	yadsl_GraphHandle* graph,
	bool* is_directed_ptr)
{
	*is_directed_ptr = ((yadsl_Graph*) graph)->is_directed;
	return YADSL_GRAPH_RET_OK;
}

yadsl_GraphRet yadsl_graph_vertex_exists_check(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* curr,
	bool* contains_ptr)
{
	void* p; // does nothing with p
	yadsl_GraphVertexItemCmpParam par;
	par.item = curr;
	par.cmp_vertices_func = ((yadsl_Graph*) graph)->cmp_vertices_func;
	switch (yadsl_set_item_filter(((yadsl_Graph*) graph)->vertex_set, yadsl_graph_vertex_item_compare_internal, &par, &p)) {
	case YADSL_SET_RET_OK:
		*contains_ptr = 1;
		break;
	case YADSL_SET_RET_DOES_NOT_CONTAIN:
		*contains_ptr = 0;
		break;
	default:
		assert(0);
	}
	return YADSL_GRAPH_RET_OK;
}

yadsl_GraphRet yadsl_graph_vertex_add(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* curr)
{
	yadsl_GraphVertex* vertex;
	yadsl_GraphRet graph_ret;
	yadsl_SetRet set_ret;
	bool contains_vertex;
	if (graph_ret = yadsl_graph_vertex_exists_check(graph, curr, &contains_vertex))
		return graph_ret;
	if (contains_vertex)
		return YADSL_GRAPH_RET_CONTAINS_VERTEX;
	vertex = malloc(sizeof(yadsl_GraphVertex));
	if (vertex == NULL)
		return YADSL_GRAPH_RET_MEMORY;
	vertex->item = curr;
	vertex->flag = 0;
	vertex->in_edges_to_iterate = 0;
	vertex->out_edges_to_iterate = 0;
	if (!(vertex->in_edges = yadsl_set_create())) {
		free(vertex);
		return YADSL_GRAPH_RET_MEMORY;
	}
	if (!(vertex->out_edges = yadsl_set_create())) {
		yadsl_set_destroy(vertex->in_edges, NULL, NULL);
		free(vertex);
		return YADSL_GRAPH_RET_MEMORY;
	}
	if (set_ret = yadsl_set_item_add(((yadsl_Graph*) graph)->vertex_set, vertex)) {
		yadsl_set_destroy(vertex->in_edges, NULL, NULL);
		yadsl_set_destroy(vertex->out_edges, NULL, NULL);
		free(vertex);
		assert(set_ret == YADSL_SET_RET_MEMORY);
		return YADSL_GRAPH_RET_MEMORY;
	}
	return YADSL_GRAPH_RET_OK;
}

yadsl_GraphRet yadsl_graph_vertex_remove(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* curr)
{
	yadsl_GraphVertex* vertex = NULL;
	yadsl_GraphRet graph_ret;
	if (graph_ret = YADSL_GRAPH_VERTICES_FIND(graph, curr, &vertex))
		return graph_ret;
	if (yadsl_set_item_remove(((yadsl_Graph*) graph)->vertex_set, vertex)) assert(0);
	yadsl_graph_vertex_free_internal(vertex, graph);
	return YADSL_GRAPH_RET_OK;
}

yadsl_GraphRet yadsl_graph_edge_add(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* u,
	yadsl_GraphVertexObject* v,
	yadsl_GraphEdgeObject* uv)
{
	yadsl_GraphVertex* u_vertex = NULL, * v_vertex = NULL;
	yadsl_GraphEdge* uv_edge = NULL;
	yadsl_GraphRet graph_ret;
	yadsl_SetRet set_ret;
	bool contains_edge;
	if (graph_ret = YADSL_GRAPH_VERTICES_FIND(graph, u, &u_vertex, v, &v_vertex))
		return graph_ret;
	if (yadsl_graph_edge_exists_check(graph, u, v, &contains_edge))
		assert(0);
	if (contains_edge)
		return YADSL_GRAPH_RET_CONTAINS_EDGE;
	uv_edge = malloc(sizeof(yadsl_GraphEdge));
	if (uv_edge == NULL)
		return YADSL_GRAPH_RET_MEMORY;
	uv_edge->item = uv;
	if (((yadsl_Graph*) graph)->is_directed) {
		uv_edge->source = u_vertex;
		uv_edge->destination = v_vertex;
	} else {
		uv_edge->source = u_vertex < v_vertex ? u_vertex : v_vertex;
		uv_edge->destination = u_vertex < v_vertex ? v_vertex : u_vertex;
	}
	assert(uv_edge->source != NULL);
	if (set_ret = yadsl_set_item_add(uv_edge->source->out_edges, uv_edge)) {
		free(uv_edge);
		assert(set_ret == YADSL_SET_RET_MEMORY);
		return YADSL_GRAPH_RET_MEMORY;
	}
	yadsl_graph_adj_list_counters_reset_internal(uv_edge->source, 1);
	assert(uv_edge->destination != NULL);
	if (set_ret = yadsl_set_item_add(uv_edge->destination->in_edges, uv_edge)) {
		if (yadsl_set_item_remove(uv_edge->source->out_edges, uv_edge)) assert(0);
		free(uv_edge);
		assert(set_ret == YADSL_SET_RET_MEMORY);
		return YADSL_GRAPH_RET_MEMORY;
	}
	yadsl_graph_adj_list_counters_reset_internal(uv_edge->destination, 1);
	return YADSL_GRAPH_RET_OK;
}

yadsl_GraphRet yadsl_graph_edge_exists_check(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* u,
	yadsl_GraphVertexObject* v,
	bool* contains_ptr)
{
	yadsl_GraphVertex* u_vertex = NULL, * v_vertex = NULL;
	yadsl_GraphRet graph_ret;
	if (graph_ret = YADSL_GRAPH_VERTICES_FIND(graph, u, &u_vertex, v, &v_vertex))
		return graph_ret;
	switch (yadsl_graph_edge_find_internal(graph, u_vertex, v_vertex, NULL, NULL, NULL)) {
	case YADSL_GRAPH_RET_OK:
		*contains_ptr = 1;
		break;
	case YADSL_GRAPH_RET_DOES_NOT_CONTAIN_EDGE:
		*contains_ptr = 0;
		break;
	default:
		assert(0);
	}
	return YADSL_GRAPH_RET_OK;
}

yadsl_GraphRet yadsl_graph_edge_remove(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* u,
	yadsl_GraphVertexObject* v)
{
	yadsl_GraphVertex* u_vertex = NULL, * v_vertex = NULL,
		* source = NULL, * destination = NULL;
	yadsl_GraphEdge* uv_edge = NULL;
	yadsl_GraphRet graph_ret;
	if (graph_ret = YADSL_GRAPH_VERTICES_FIND(graph, u, &u_vertex, v, &v_vertex))
		return graph_ret;
	if (graph_ret = yadsl_graph_edge_find_internal(graph, u_vertex, v_vertex, &source,
		&destination, &uv_edge))
		return graph_ret;
	if (yadsl_set_item_remove(source->out_edges, uv_edge)) assert(0);
	yadsl_graph_adj_list_counters_reset_internal(source, 1);
	if (yadsl_set_item_remove(destination->in_edges, uv_edge)) assert(0);
	yadsl_graph_adj_list_counters_reset_internal(destination, 1);
	if (((yadsl_Graph*) graph)->free_edge_func)
		((yadsl_Graph*) graph)->free_edge_func(uv_edge->item);
	free(uv_edge);
	return YADSL_GRAPH_RET_OK;
}

yadsl_GraphRet yadsl_graph_edge_get(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* u,
	yadsl_GraphVertexObject* v,
	yadsl_GraphEdgeObject** uv_ptr)
{
	yadsl_GraphVertex* u_vertex, * v_vertex;
	yadsl_GraphEdge* temp;
	yadsl_GraphRet graph_ret;
	if (graph_ret = YADSL_GRAPH_VERTICES_FIND(graph, u, &u_vertex, v, &v_vertex))
		return graph_ret;
	if (graph_ret = yadsl_graph_edge_find_internal(graph, u_vertex, v_vertex, NULL, NULL, &temp))
		return graph_ret;
	*uv_ptr = temp->item;
	return YADSL_GRAPH_RET_OK;
}

yadsl_GraphRet yadsl_graph_vertex_degree_get(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* curr,
	yadsl_GraphEdgeDirection edge_direction,
	size_t* degree_ptr)
{
	yadsl_GraphVertex* vertex;
	yadsl_GraphRet graph_ret;
	size_t in, out;
	if (graph_ret = YADSL_GRAPH_VERTICES_FIND(graph, curr, &vertex))
		return graph_ret;
	if (edge_direction & YADSL_GRAPH_EDGE_DIR_IN) {
		if (yadsl_set_size_get(vertex->in_edges, &in))
			assert(0);
	} else {
		in = 0;
	}
	if (edge_direction & YADSL_GRAPH_EDGE_DIR_OUT) {
		if (yadsl_set_size_get(vertex->out_edges, &out))
			assert(0);
	} else {
		out = 0;
	}
	*degree_ptr = in + out;
	return YADSL_GRAPH_RET_OK;
}

yadsl_GraphRet yadsl_graph_vertex_nb_iter(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* vertex,
	yadsl_GraphEdgeDirection edge_direction,
	yadsl_GraphIterationDirection iter_direction,
	yadsl_GraphVertexObject** nb_ptr,
	yadsl_GraphEdgeObject** edge_ptr)
{
	if (edge_direction == YADSL_GRAPH_EDGE_DIR_BOTH) {
		switch (iter_direction) {
		case YADSL_GRAPH_ITER_DIR_NEXT:
			return yadsl_graph_vertex_nb_total_next_get_internal(
				graph, vertex, nb_ptr, edge_ptr);
		case YADSL_GRAPH_ITER_DIR_PREVIOUS:
			return yadsl_graph_vertex_nb_total_prev_get_internal(
				graph, vertex, nb_ptr, edge_ptr);
		default:
			return YADSL_GRAPH_RET_PARAMETER;
		}
	} else {
		return yadsl_graph_vertex_nb_get_internal(
			graph, vertex, edge_direction, iter_direction, nb_ptr, edge_ptr);
	}
}

yadsl_GraphRet yadsl_graph_vertex_flag_get(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* curr,
	yadsl_GraphVertexFlag* flag_ptr)
{
	yadsl_GraphVertex* vertex;
	yadsl_GraphRet graph_ret;
	if (graph_ret = YADSL_GRAPH_VERTICES_FIND(graph, curr, &vertex))
		return graph_ret;
	*flag_ptr = vertex->flag;
	return YADSL_GRAPH_RET_OK;
}

yadsl_GraphRet yadsl_graph_vertex_flag_set(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* curr,
	yadsl_GraphVertexFlag flag)
{
	yadsl_GraphVertex* vertex;
	yadsl_GraphRet graph_ret;
	if (graph_ret = YADSL_GRAPH_VERTICES_FIND(graph, curr, &vertex))
		return graph_ret;
	vertex->flag = flag;
	return YADSL_GRAPH_RET_OK;
}

yadsl_GraphRet yadsl_graph_vertex_flag_set_all(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexFlag flag)
{
	void* temp;
	if (yadsl_set_item_filter(((yadsl_Graph*) graph)->vertex_set, yadsl_graph_vertex_flag_set_internal, &flag, &temp)
		!= YADSL_SET_RET_DOES_NOT_CONTAIN) // yadsl_graph_vertex_flag_set_internal only returns 0
		assert(0);
	return YADSL_GRAPH_RET_OK;
}

void yadsl_graph_destroy(yadsl_GraphHandle* graph)
{
	if (graph == NULL)
		return;
	yadsl_set_destroy(((yadsl_Graph*) graph)->vertex_set, yadsl_graph_vertex_free_internal, graph);
	free(graph);
}

/**************************************/
/*  Private functions implementation  */
/**************************************/

yadsl_GraphRet yadsl_graph_vertex_nb_total_next_get_internal(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* curr,
	yadsl_GraphVertexObject** next_nb_ptr,
	yadsl_GraphEdgeObject** edge_ptr)
{
	yadsl_GraphVertex* vertex;
	yadsl_GraphRet graph_ret;
	size_t in_size, out_size;
	yadsl_GraphEdge* temp;
	if (graph_ret = YADSL_GRAPH_VERTICES_FIND(graph, curr, &vertex))
		return graph_ret;
	if (yadsl_set_size_get(vertex->in_edges, &in_size)) assert(0);
	if (yadsl_set_size_get(vertex->out_edges, &out_size)) assert(0);
	if (in_size == 0 && out_size == 0)
		return YADSL_GRAPH_RET_DOES_NOT_CONTAIN_EDGE;
	if (in_size == 0) {
		// thus, out_size > 0
		yadsl_graph_set_cursor_cycle_internal(vertex->out_edges, &temp, YADSL_GRAPH_ITER_DIR_NEXT);
	} else if (out_size == 0) {
		// thus, in_size > 0
		yadsl_graph_set_cursor_cycle_internal(vertex->in_edges, &temp, YADSL_GRAPH_ITER_DIR_NEXT);
	} else {
		// thus, both are > 0
		if (vertex->in_edges_to_iterate == 0 &&
			vertex->out_edges_to_iterate == 0) {
			yadsl_graph_adj_list_counters_reset_internal(vertex, 1);
			goto flag;
		} else if (vertex->in_edges_to_iterate == 0) {
			// thus, outEdgesIterated > 0
			yadsl_graph_set_cursor_cycle_internal(vertex->out_edges, &temp, YADSL_GRAPH_ITER_DIR_NEXT);
			--vertex->out_edges_to_iterate;
		} else {
		flag:
			// thus, in_edges_to_iterate > 0
			yadsl_graph_set_cursor_cycle_internal(vertex->in_edges, &temp, YADSL_GRAPH_ITER_DIR_NEXT);
			--vertex->in_edges_to_iterate;
		}
	}
	*next_nb_ptr = temp->destination == vertex ?
		temp->source->item : temp->destination->item;
	*edge_ptr = temp->item;
	return YADSL_GRAPH_RET_OK;
}

yadsl_GraphRet yadsl_graph_vertex_nb_total_prev_get_internal(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* curr,
	yadsl_GraphVertexObject** previous_nb_ptr,
	yadsl_GraphEdgeObject** edge_ptr)
{
	yadsl_GraphVertex* vertex;
	yadsl_GraphRet graph_ret;
	size_t in_size, out_size;
	yadsl_GraphEdge* temp;
	if (graph_ret = YADSL_GRAPH_VERTICES_FIND(graph, curr, &vertex))
		return graph_ret;
	if (yadsl_set_size_get(vertex->in_edges, &in_size)) assert(0);
	if (yadsl_set_size_get(vertex->out_edges, &out_size)) assert(0);
	if (in_size == 0 && out_size == 0)
		return YADSL_GRAPH_RET_DOES_NOT_CONTAIN_EDGE;
	if (in_size == 0) {
		// thus, out_size > 0
		if (yadsl_graph_set_cursor_cycle_internal(vertex->out_edges, &temp, YADSL_GRAPH_ITER_DIR_PREVIOUS))
			assert(0);
	} else if (out_size == 0) {
		// thus, in_size > 0
		if (yadsl_graph_set_cursor_cycle_internal(vertex->in_edges, &temp, YADSL_GRAPH_ITER_DIR_PREVIOUS))
			assert(0);
	} else {
		// thus, both are > 0
		if (vertex->in_edges_to_iterate == in_size) {
			yadsl_graph_adj_list_counters_reset_internal(vertex, -1);
			goto flag;
		} else if (vertex->out_edges_to_iterate == out_size) {
			// thus, in_edges_to_iterate < in_size
			if (yadsl_graph_set_cursor_cycle_internal(vertex->in_edges, &temp, YADSL_GRAPH_ITER_DIR_PREVIOUS))
				assert(0);
			++vertex->in_edges_to_iterate;
		} else {
		flag:
			// thus, out_edges_to_iterate < out_size
			if (yadsl_graph_set_cursor_cycle_internal(vertex->out_edges, &temp, YADSL_GRAPH_ITER_DIR_PREVIOUS))
				assert(0);
			++vertex->out_edges_to_iterate;
		}
	}
	*previous_nb_ptr = temp->destination == vertex ?
		temp->source->item : temp->destination->item;
	*edge_ptr = temp->item;
	return YADSL_GRAPH_RET_OK;
}

// Cycle set cursor (when reaches end, go to first)
// [!] Assumes set is not empty!
yadsl_GraphRet yadsl_graph_set_cursor_cycle_internal(
	yadsl_SetHandle* set,
	void** value_ptr,
	yadsl_GraphIterationDirection iter_direction)
{
	yadsl_SetRet set_ret;
	yadsl_SetRet(*cycle_func)(yadsl_SetHandle*);
	yadsl_SetRet(*reset_func)(yadsl_SetHandle*);
	void* temp;
	switch (iter_direction) {
	case YADSL_GRAPH_ITER_DIR_NEXT:
		cycle_func = yadsl_set_cursor_next;
		reset_func = yadsl_set_cursor_first;
		break;
	case YADSL_GRAPH_ITER_DIR_PREVIOUS:
		cycle_func = yadsl_set_cursor_previous;
		reset_func = yadsl_set_cursor_last;
		break;
	default:
		return YADSL_GRAPH_RET_PARAMETER;
	}
	if (yadsl_set_cursor_get(set, &temp)) assert(0);
	if (set_ret = cycle_func(set)) {
		assert(set_ret == YADSL_SET_RET_OUT_OF_BOUNDS);
		if (reset_func(set)) assert(0);
	}
	*value_ptr = temp;
	return YADSL_GRAPH_RET_OK;
}

// Reset set cursors (due to getNeighbour contract)
void yadsl_graph_adj_list_counters_reset_internal(
	yadsl_GraphVertex* vertex,
	int orientation)
{
	assert(orientation == 1 || orientation == -1);
	if (orientation == 1) {
		yadsl_set_cursor_first(vertex->in_edges);
		yadsl_set_cursor_first(vertex->out_edges);
		yadsl_set_size_get(vertex->in_edges, &vertex->in_edges_to_iterate);
		yadsl_set_size_get(vertex->out_edges, &vertex->out_edges_to_iterate);
	} else {
		yadsl_set_cursor_last(vertex->in_edges);
		yadsl_set_cursor_last(vertex->out_edges);
		vertex->in_edges_to_iterate = 0;
		vertex->out_edges_to_iterate = 0;
	}
}

// Compares destination from edge and the one provided
bool yadsl_graph_edge_dest_compare_internal(
	yadsl_GraphEdge* edge_ptr,
	yadsl_GraphVertex* destination)
{
	return edge_ptr->destination == destination;
}

// Compares item from graph vertex struct and item
bool yadsl_graph_vertex_item_compare_internal(
	yadsl_GraphVertex* vertex,
	yadsl_GraphVertexItemCmpParam* par)
{
	if (par->cmp_vertices_func)
		return par->cmp_vertices_func(par->item, vertex->item);
	return vertex->item == par->item;
}

// Called by setlib while removing vertex from graph->vertex_set
void yadsl_graph_vertex_free_internal(
	yadsl_GraphVertex* vertex,
	yadsl_Graph* graph)
{
	if (graph->free_vertex_func)
		graph->free_vertex_func(vertex->item);
	yadsl_set_destroy(vertex->in_edges, yadsl_graph_vertex_in_free_internal, graph->free_edge_func);
	yadsl_set_destroy(vertex->out_edges, yadsl_graph_vertex_out_free_internal, graph->free_edge_func);
	free(vertex);
}

// Called by setlib while removing edge from edge->destination->in_edges
void yadsl_graph_vertex_in_free_internal(
	yadsl_GraphEdge* edge_ptr,
	yadsl_GraphFreeEdgeObjFunc free_edge_func)
{
	yadsl_set_item_remove(edge_ptr->source->out_edges, edge_ptr);
	if (free_edge_func)
		free_edge_func(edge_ptr->item);
	free(edge_ptr);
}

// Called by setlib while removing edge from edge->source->out_edges
void yadsl_graph_vertex_out_free_internal(
	yadsl_GraphEdge* edge_ptr,
	yadsl_GraphFreeEdgeObjFunc free_edge_func)
{
	yadsl_set_item_remove(edge_ptr->destination->in_edges, edge_ptr);
	if (free_edge_func)
		free_edge_func(edge_ptr->item);
	free(edge_ptr);
}

// Retrieves the edge uv through the vertices u and vertex
// source_ptr, destination_ptr and uv_edge_ptr are optional and
// NULL can be parsed without a problem.
// Possible errors:
// GRAPH_INVALID_PARAMETER
//	- "graph" is NULL
//	- "u_vertex" is NULL
//	- "v_vertex" is NULL
// GRAPH_DOES_NOT_CONTAIN_EDGE
yadsl_GraphRet yadsl_graph_edge_find_internal(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertex* u_vertex,
	yadsl_GraphVertex* v_vertex,
	yadsl_GraphVertex** source_ptr,
	yadsl_GraphVertex** destination_ptr,
	yadsl_GraphEdge** uv_edge_ptr)
{
	yadsl_GraphVertex* source, * destination;
	yadsl_GraphEdge* uv_edge;
	yadsl_SetRet set_ret;
	source = (((yadsl_Graph*) graph)->is_directed || u_vertex < v_vertex) ?
		u_vertex : v_vertex;
	destination = u_vertex == source ? v_vertex : u_vertex;
	set_ret = yadsl_set_item_filter(
		source->out_edges,                      /* set */
		yadsl_graph_edge_dest_compare_internal, /* func */
		destination,                            /* arg */
		&uv_edge);                              /* item_ptr */
	switch (set_ret) {
	case YADSL_SET_RET_OK:
		if (source_ptr) *source_ptr = source;
		if (destination_ptr) *destination_ptr = destination;
		if (uv_edge_ptr) *uv_edge_ptr = uv_edge;
		break;
	case YADSL_SET_RET_DOES_NOT_CONTAIN:
		return YADSL_GRAPH_RET_DOES_NOT_CONTAIN_EDGE;
	default:
		assert(0);
	}
	return YADSL_GRAPH_RET_OK;;
}

// Retrieves the vertices through the items contained in them.
// The item and curr_ptr arguments must be alternated, and end with two NULLs:
// yadsl_graph_vertices_find_internal(graph, u, &u_vertex_ptr, v, &v_vertex_ptr, w, &w_vertex_ptr, NULL, NULL);
// The macro YADSL_GRAPH_RET_VERTICES_FIND already adds the trailing two NULLs.
// Does not alter the vertex_set cursor pointer, only the current pointer,
// which is of internal use, and doesn't interfeer with NextVertex iteration.
// Possible errors:
// GRAPH_INVALID_PARAMETER
//  - "graph" is NULL
//  - "curr_ptr" is NULL
// GRAPH_DOES_NOT_CONTAIN_VERTEX
yadsl_GraphRet yadsl_graph_vertices_find_internal(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* item,
	yadsl_GraphVertex** vertex_ptr, ...)
{
	yadsl_SetRet set_ret;
	va_list va;
	yadsl_GraphVertexItemCmpParam param;
	param.item = item;
	param.cmp_vertices_func = ((yadsl_Graph*) graph)->cmp_vertices_func;
	va_start(va, vertex_ptr);
	do {
		if (set_ret = yadsl_set_item_filter(
			((yadsl_Graph*) graph)->vertex_set,        /* set */
			yadsl_graph_vertex_item_compare_internal,  /* func */
			&param,                                    /* arg */
			vertex_ptr)) {                             /* item_ptr */
			va_end(va);
			assert(set_ret == YADSL_SET_RET_DOES_NOT_CONTAIN);
			return YADSL_GRAPH_RET_DOES_NOT_CONTAIN_VERTEX;
		}
		param.item = va_arg(va, yadsl_GraphVertexObject*);
		if (!(vertex_ptr = va_arg(va, yadsl_GraphVertex**)))
			break; /* sentinel */
	} while (1);
	va_end(va);
	return YADSL_GRAPH_RET_OK;
}

// set vertex flag to *flag
bool yadsl_graph_vertex_flag_set_internal(
	yadsl_GraphVertex* vertex,
	yadsl_GraphVertexFlag* flag)
{
	vertex->flag = *flag;
	return 0;
}

// Generic function for obtaining neighbour of a defined edge set
yadsl_GraphRet yadsl_graph_vertex_nb_get_internal(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* u,
	yadsl_GraphEdgeDirection edge_direction,
	yadsl_GraphIterationDirection iter_direction,
	yadsl_GraphVertexObject** v_ptr,
	yadsl_GraphEdgeObject** uv_ptr)
{
	yadsl_GraphVertex* vertex;
	yadsl_SetHandle* set;
	yadsl_GraphRet graph_ret;
	size_t set_size;
	yadsl_GraphEdge* temp;
	if (graph_ret = YADSL_GRAPH_VERTICES_FIND(graph, u, &vertex))
		return graph_ret;
	switch (edge_direction) {
	case YADSL_GRAPH_EDGE_DIR_IN:
		set = vertex->in_edges;
		break;
	case YADSL_GRAPH_EDGE_DIR_OUT:
		set = vertex->out_edges;
		break;
	default:
		return YADSL_GRAPH_RET_PARAMETER;
	}
	if (yadsl_set_size_get(set, &set_size)) assert(0);
	if (set_size == 0)
		return YADSL_GRAPH_RET_DOES_NOT_CONTAIN_EDGE;
	if (graph_ret = yadsl_graph_set_cursor_cycle_internal(set, &temp, iter_direction))
		return graph_ret;
	*v_ptr = edge_direction == YADSL_GRAPH_EDGE_DIR_IN ? temp->source->item : temp->destination->item;
	*uv_ptr = temp->item;
	return YADSL_GRAPH_RET_OK;
}
