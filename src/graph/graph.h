#ifndef __YADSL_GRAPH_H__
#define __YADSL_GRAPH_H__

/**
 * \defgroup graph Graph
 * @brief Generic graph (directed or undirected)
 * 
 * A Graph starts with no vertices and, therefore, no edges.
 * You are able to add and remove vertices and edges, check
 * if vertices and edges are contained in graph, iterate through
 * vertices and vertex neighbours (in, out or total), obtain
 * vertex count, vertex degrees (in, out or total), and set/get
 * flags in vertices (for searches in graph, coloring...)
 * 
 * Undirected graphs store edges differently than directed
 * graphs, but still, in such way that functions that relate to
 * in or out neighbours will provide diferent edges, for every
 * vertex on a graph. That's why it can be used for serialization.
 * 
 * @{
*/

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Return condition of Graph functions
*/
typedef enum
{
	YADSL_GRAPH_RET_OK = 0, /**< All went ok */
	YADSL_GRAPH_RET_EMPTY, /**< Graph is empty */
	YADSL_GRAPH_RET_CONTAINS_VERTEX, /**< Graph (already) contains vertex */
	YADSL_GRAPH_RET_DOES_NOT_CONTAIN_VERTEX, /**< Graph does not contain vertex */
	YADSL_GRAPH_RET_CONTAINS_EDGE, /**< Graph (already) contains edge */
	YADSL_GRAPH_RET_DOES_NOT_CONTAIN_EDGE, /**< Graph does not contain edge*/
	YADSL_GRAPH_RET_MEMORY, /**< Could not allocate memory */
	YADSL_GRAPH_RET_PARAMETER, /**< Invalid parameter */
}
yadsl_GraphRet;

/**
 * @brief Iteration direction of Graph iterator functions
*/
typedef enum
{
	YADSL_GRAPH_ITER_DIR_NEXT, /**< Iterate to next element */
	YADSL_GRAPH_ITER_DIR_PREVIOUS, /**< Iterate to previous element */
}
yadsl_GraphIterationDirection;

/**
 * @brief Edge direction
*/
typedef enum
{
	YADSL_GRAPH_EDGE_DIR_NONE = 0, /**< No direction */
	YADSL_GRAPH_EDGE_DIR_IN = 1 << 0, /**< In direction */
	YADSL_GRAPH_EDGE_DIR_OUT = 1 << 1, /**< Out direction */
	YADSL_GRAPH_EDGE_DIR_BOTH = (1 << 0) | (1 << 1), /** Both directions */
}
yadsl_GraphEdgeDirection;

typedef void yadsl_GraphHandle; /**< Graph handle */
typedef void yadsl_GraphVertexObject; /**< Graph vertex object (user data) */
typedef void yadsl_GraphEdgeObject; /**< Graph edge object (user data) */
typedef int yadsl_GraphVertexFlag; /**< Graph vertex flag (for colouring, searches...) */

/**
 * @brief Graph vertex object comparison function
 * @param obj1 first object
 * @param obj2 second object
 * @return an integer *n*, where...
 * * *n* > 0 if obj1 > obj2
 * * *n* = 0 if obj1 = obj2
 * * *n* < 0 if obj1 < obj2
*/
typedef int (*yadsl_GraphCmpVertexObjsFunc)(yadsl_GraphVertexObject* obj1, yadsl_GraphVertexObject* obj2);

/**
 * @brief Graph vertex object freeing function
 * @param obj object
*/
typedef void (*yadsl_GraphFreeVertexObjFunc)(yadsl_GraphVertexObject* obj);

/**
 * @brief Graph edge object comparison function
 * @param obj1 first object
 * @param obj2 second object
 * @return an integer *n*, where...
 * * *n* > 0 if obj1 > obj2
 * * *n* = 0 if obj1 = obj2
 * * *n* < 0 if obj1 < obj2
*/
typedef int (*yadsl_GraphCmpEdgeObjsFunc)(yadsl_GraphEdgeObject* obj1, yadsl_GraphEdgeObject* obj2);

/**
 * @brief Graph edge object freeing function
 * @param obj object
*/
typedef void (*yadsl_GraphFreeEdgeObjFunc)(yadsl_GraphEdgeObject* obj);

/**
 * @brief Create an empty graph
 * @param is_directed whether the graph is directed or not
 * @param cmp_vertices_func vertex object comparison function
 * @param free_vertex_func vertex object freeing function
 * @param cmp_edges_func edge object comparison function
 * @param free_edge_func edge object freeing function
 * @return newly created graph or NULL if could not allocate enough memory
*/
yadsl_GraphHandle* yadsl_graph_create(
	bool is_directed,
	yadsl_GraphCmpVertexObjsFunc cmp_vertices_func,
	yadsl_GraphFreeVertexObjFunc free_vertex_func,
	yadsl_GraphCmpEdgeObjsFunc cmp_edges_func,
	yadsl_GraphFreeEdgeObjFunc free_edge_func);

/**
 * @brief Check whether graph is directed or not
 * @param graph graph
 * @param is_directed_ptr whether graph is directed or not
 * @return ::YADSL_GRAPH_RET_OK, and *is_directed_ptr is updated
*/
yadsl_GraphRet yadsl_graph_is_directed_check(
	yadsl_GraphHandle *graph,
	bool *is_directed_ptr);

/**
 * @brief Get number of vertices in graph
 * @param graph graph
 * @param pNumberOfVertices number of vertices
 * @return ::YADSL_GRAPH_RET_OK, and *vertex_cnt_ptr is updated
*/
yadsl_GraphRet yadsl_graph_vertex_count_get(
	yadsl_GraphHandle *graph,
	size_t *vertex_cnt_ptr);

/**
 * @brief Iterate through vertex (cycles through all)
 * @param graph graph
 * @param iter_direction iteration direction
 * @param vertex_ptr current vertex
 * @return
 * * ::YADSL_GRAPH_RET_OK, and *vertex_ptr is updated
 * * ::YADSL_GRAPH_RET_PARAMETER
 * * ::YADSL_GRAPH_RET_EMPTY
*/
yadsl_GraphRet yadsl_graph_vertex_iter(
	yadsl_GraphHandle* graph,
	yadsl_GraphIterationDirection iter_direction,
	yadsl_GraphVertexObject** vertex_ptr);

/**
 * @brief Get vertex degree
 * @param graph graph
 * @param vertex vertex
 * @param edge_direction degree type (sum of in edges, out edges or both)
 * @param degree_ptr vertex degree
 * @return
 * * ::YADSL_GRAPH_RET_COND_OK, and *degree_ptr is updated
 * * ::YADSL_GRAPH_RET_COND_DOES_NOT_CONTAIN_VERTEX
*/
yadsl_GraphRet yadsl_graph_vertex_degree_get(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* vertex,
	yadsl_GraphEdgeDirection edge_direction,
	size_t* degree_ptr);

/**
 * @brief Iterate through neighbour of a given vertex
 * @param graph graph
 * @param vertex vertex
 * @param edge_direction edge direction
 * @param iter_direction iteration direction
 * @param nb_ptr neighbour of vertex
 * @param edge_ptr edge between vertex and its neighbour
 * @return
 * * ::YADSL_GRAPH_RET_OK, and *nb_ptr and *edge_ptr are updated
 * * ::YADSL_GRAPH_RET_DOES_NOT_CONTAIN_VERTEX
 * * ::YADSL_GRAPH_RET_DOES_NOT_CONTAIN_EDGE
 * * ::YADSL_GRAPH_RET_PARAMETER
*/
yadsl_GraphRet yadsl_graph_vertex_nb_iter(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* vertex,
	yadsl_GraphEdgeDirection edge_direction,
	yadsl_GraphIterationDirection iter_direction,
	yadsl_GraphVertexObject** nb_ptr,
	yadsl_GraphEdgeObject** edge_ptr);

/**
 * @brief Check whether vertex exists in a graph or not
 * @param graph graph
 * @param vertex vertex
 * @param contains_ptr whether vertex exists in graph or not
 * @return ::YADSL_GRAPH_RET_ON, and *contains_ptr is updated
*/
yadsl_GraphRet yadsl_graph_vertex_exists_check(
	yadsl_GraphHandle *graph,
	yadsl_GraphVertexObject *vertex,
	bool *contains_ptr);

/**
 * @brief Add vertex to graph
 * @param graph graph
 * @param vertex vertex
 * @return
 * * ::YADSL_GRAPH_RET_OK, and vertex is added
 * * ::YADSL_GRAPH_RET_CONTAINS_VERTEX (if vertex was already added)
 * * ::YADSL_GRAPH_RET_MEMORY
*/
yadsl_GraphRet yadsl_graph_vertex_add(
	yadsl_GraphHandle *graph,
	yadsl_GraphVertexObject* vertex);

/**
 * @brief Remove vertex from graph
 * @param graph graph
 * @param vertex vertex
 * @return
 * * ::YADSL_GRAPH_RET_OK, and vertex is removed
 * * ::YADSL_GRAPH_RET_DOES_NOT_CONTAIN_VERTEX
*/
yadsl_GraphRet yadsl_graph_vertex_remove(
	yadsl_GraphHandle *graph,
	yadsl_GraphVertexObject* vertex);

/**
 * @brief Check if edge exists in a graph or not
 * @param graph graph
 * @param u edge source (if directed)
 * @param v edge destination (if directed)
 * @param contains_ptr whether edge exists in graph or not
 * @return
 * * ::YADSL_GRAPH_RET_OK, and *contains_ptr is updated
 * * ::YADSL_GRAPH_RET_DOES_NOT_CONTAIN_VERTEX
*/
yadsl_GraphRet yadsl_graph_edge_exists_check(
	yadsl_GraphHandle *graph,
	yadsl_GraphVertexObject *u,
	yadsl_GraphVertexObject *v,
	bool *contains_ptr);

/**
 * @brief Add edge to graph
 * @param graph graph
 * @param u edge source (if directed)
 * @param v edge destination (if directed)
 * @param uv edge connecting u and v
 * @return
 * * ::YADSL_GRAPH_RET_OK, and edge is added
 * * ::YADSL_GRAPH_RET_DOES_NOT_CONTAIN_VERTEX
 * * ::YADSL_GRAPH_RET_CONTAINS_EDGE
 * * ::YADSL_GRAPH_RET_MEMORY
 * @attention on success, this function may alter the state of the neighbour
 * iterator functions such as ::yadsl_graph_vertex_nb_total_next_get_internal
*/
yadsl_GraphRet yadsl_graph_edge_add(
	yadsl_GraphHandle *graph,
	yadsl_GraphVertexObject *u,
	yadsl_GraphVertexObject *v,
	yadsl_GraphEdgeObject *uv);

/**
 * @brief Get edge between two vertices in a graph
 * @param graph graph
 * @param u edge source (if directed)
 * @param v edge destination (if directed)
 * @param uv_ptr edge between u and v
 * @return
 * * ::YADSL_GRAPH_RET_OK, and *uv_ptr is updated
 * * ::YADSL_GRAPH_RET_DOES_NOT_CONTAIN_VERTEX
 * * ::YADSL_GRAPH_RET_DOES_NOT_CONTAIN_EDGE
*/
yadsl_GraphRet yadsl_graph_edge_get(
	yadsl_GraphHandle *graph,
	yadsl_GraphVertexObject* u,
	yadsl_GraphVertexObject* v,
	yadsl_GraphEdgeObject** uv_ptr);

/**
 * @brief Remove edge between u and v from graph
 * @param graph graph
 * @param u edge source (if directed)
 * @param v edge destination (if directed)
 * @return
 * * ::YADSL_GRAPH_RET_OK, and edge is removed
 * * ::YADSL_GRAPH_RET_DOES_NOT_CONTAIN_VERTEX
 * * ::YADSL_GRAPH_RET_DOES_NOT_CONTAIN_EDGE
 * @attention on success, this function may alter the state of the neighbour
 * iterator functions such as ::yadsl_graph_vertex_nb_total_next_get_internal
*/
yadsl_GraphRet yadsl_graph_edge_remove(
	yadsl_GraphHandle *graph,
	yadsl_GraphVertexObject* u,
	yadsl_GraphVertexObject* v);

/**
 * @brief Get flag associated with vertex in graph
 * @param graph graph
 * @param v vertex
 * @param flag_ptr flag associated with vertex in graph
 * @return
 * * ::YADSL_GRAPH_RET_OK, and *flag_ptr is updated
 * * ::YADSL_GRAPH_RET_DOES_NOT_CONTAIN_VERTEX
*/
yadsl_GraphRet yadsl_graph_vertex_flag_get(
	yadsl_GraphHandle *graph,
	yadsl_GraphVertexObject* v,
	yadsl_GraphVertexFlag *flag_ptr);

/**
 * @brief Set flag associated with vertex in graph
 * @param graph graph
 * @param v vertex
 * @param flag new flag associated with vertex in graph
 * @return
 * * ::YADSL_GRAPH_RET_OK, and flag is associated with vertex
 * * ::YADSL_GRAPH_RET_DOES_NOT_CONTAIN_VERTEX
*/
yadsl_GraphRet yadsl_graph_vertex_flag_set(
	yadsl_GraphHandle *graph,
	yadsl_GraphVertexObject* v,
	yadsl_GraphVertexFlag flag);

/**
 * @brief Set flag associated with all vertices in graph
 * @param graph graph
 * @param flag new flag
 * @return
 * * ::YADSL_GRAPH_RET_OK, and flag is associated with all vertives
*/
yadsl_GraphRet yadsl_graph_vertex_flag_set_all(
	yadsl_GraphHandle *graph,
	yadsl_GraphVertexFlag flag);

/**
 * @brief Destroys graph
 * @param graph graph
*/
void yadsl_graph_destroy(yadsl_GraphHandle *graph);

/** }@ */

#endif
