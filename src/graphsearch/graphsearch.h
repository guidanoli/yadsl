#ifndef __YADSL_GRAPHSEARCH_H__
#define __YADSL_GRAPHSEARCH_H__

/**
 * \defgroup graphsearch Graph Search
 * @brief Auxiliary module for searching in Graphs.
 * @{
*/

#include <graph/graph.h>

/**
 * @brief Return condition of Graph Search functions
*/
typedef enum
{
	YADSL_GRAPHSEARCH_RET_OK = 0, /**< All went ok */
	YADSL_GRAPHSEARCH_RET_DOES_NOT_CONTAIN_VERTEX, /**< Graph doesn't contain vertex */
	YADSL_GRAPHSEARCH_RET_VERTEX_ALREADY_VISITED, /**< Vertex was already visited */
	YADSL_GRAPHSEARCH_RET_MEMORY, /**< Could not allocate memory */
}
yadsl_GraphSearchRet;

/**
 * @brief Function responsible for visiting each vertex in a graph search
 * @param vertex visited vertex
*/
typedef void (*yadsl_GraphSearchVertexVisitFunc)(yadsl_GraphVertexObject* vertex);

/**
 * @brief Function responsible for visiting each edge in a graph search
 * @param source edge source vertex
 * @param edge visited edge
 * @param dest edge destination vertex
*/
typedef void (*yadsl_GraphSearchEdgeVisitFunc)(yadsl_GraphVertexObject* source, yadsl_GraphEdgeObject* edge, yadsl_GraphVertexObject* dest);

/**
 * @brief Visit the graph in a depth-first search fashion
 * @param graph graph
 * @param initial_vertex initial vertex
 * @param visited_flag value that will be set to visited vertices
 * @param visit_vertex_func vertex visiting function
 * @param visit_edge_func edge visiting function
 * @return
 * * ::YADSL_GRAPHSEARCH_RET_OK
 * * ::YADSL_GRAPHSEARCH_RET_VERTEX_ALREADY_VISITED
 * * ::YADSL_GRAPHSEARCH_RET_DOES_NOT_CONTAIN_VERTEX
*/
yadsl_GraphSearchRet yadsl_graphsearch_dfs(
	yadsl_GraphHandle *graph,
	yadsl_GraphVertexObject *initial_vertex,
	yadsl_GraphVertexFlag visited_flag,
	yadsl_GraphSearchVertexVisitFunc visit_vertex_func,
	yadsl_GraphSearchEdgeVisitFunc visit_edge_func);

/**
 * @brief Visit the graph in a breadth-first search fashion
 * @param graph graph
 * @param initial_vertex initial vertex
 * @param visited_flag value that will be set to visited vertices
 * @param visit_vertex_func vertex visiting function
 * @param visit_edge_func edge visiting function
 * @return
 * * ::YADSL_GRAPHSEARCH_RET_OK
 * * ::YADSL_GRAPHSEARCH_RET_VERTEX_ALREADY_VISITED
 * * ::YADSL_GRAPHSEARCH_RET_DOES_NOT_CONTAIN_VERTEX
 * * ::YADSL_GRAPHSEARCH_RET_MEMORY
*/
yadsl_GraphSearchRet yadsl_graphsearch_bfs(
	yadsl_GraphHandle* graph,
	yadsl_GraphVertexObject* initial_vertex,
	yadsl_GraphVertexFlag visited_flag,
	yadsl_GraphSearchVertexVisitFunc visit_vertex_func,
	yadsl_GraphSearchEdgeVisitFunc visit_edge_func);

#ifdef _DEBUG

/**
 * @brief Get graph search node reference count
 * Used for memory leak detection.
 * @return graph search node reference count
*/
int yadsl_graphsearch_get_node_ref_count();

#endif

/** @} */

#endif