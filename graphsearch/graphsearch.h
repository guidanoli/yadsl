#ifndef __GRAPH_SEARCH_H__
#define __GRAPH_SEARCH_H__

#include "graph.h"

//
//     ______                 __   _____                      __  
//    / ____/________ _____  / /_ / ___/___  ____ ___________/ /_ 
//   / / __/ ___/ __ `/ __ \/ __ \\__ \/ _ \/ __ `/ ___/ ___/ __ \
//  / /_/ / /  / /_/ / /_/ / / / /__/ /  __/ /_/ / /  / /__/ / / /
//  \____/_/   \__,_/ .___/_/ /_/____/\___/\__,_/_/   \___/_/ /_/ 
//                 /_/                                            
//
// Auxiliary module for searching in Graphs with algorithms such as
// Depth-First Search (DFS) and Breadth-First Search (BFS) in an almost
// fully customizable way, by providing custom callbacks that are triggered
// every time an unvisited vertex or edge is visited.
//

typedef enum
{
	// SEMANTIC RETURN VALUES

	/* Everything went as excepted */
	GRAPH_SEARCH_RETURN_OK = 0,

	/* Graph does not contain vertex */
	GRAPH_SEARCH_RETURN_DOES_NOT_CONTAIN_VERTEX,

	/* Started search on an already visited vertex */
	GRAPH_SEARCH_RETURN_VERTEX_ALREADY_VISITED,

	// ERROR RETURN VALUES

	/* Invalid parameter was provided */
	GRAPH_SEARCH_RETURN_INVALID_PARAMETER,

	/* Could not allocate memory space */
	GRAPH_SEARCH_RETURN_MEMORY,

	/* When an internal error is unrecognized */
	GRAPH_SEARCH_RETURN_UNKNOWN_ERROR,
}
GraphSearchReturnID;

/**
* Visit every neighbour in the graph that can be accessed from
* the initial vertex, in a depth-first-search
* pGraph                pointer to graph
* initialVertex         initial vertex
* visitedFlag           value that will be set to visited vertices
* visitVertexCallback   function that will be called after the
*                       vertex is visited first by the dfs
* visitEdgeCallback     function that will be called after the
*                       edge is visited first by the dfs
* Possible error values:
* GRAPH_SEARCH_RETURN_INVALID_PARAMETER
*	- "pGraph" is NULL
* GRAPH_SEARCH_RETURN_VERTEX_ALREADY_VISITED
* GRAPH_SEARCH_RETURN_DOES_NOT_CONTAIN_VERTEX
* GRAPH_SEARCH_RETURN_UNKNOWN_ERROR
*/
GraphSearchReturnID graphDFS(Graph *pGraph,
	void *initialVertex,
	int visitedFlag,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest));


/**
* Visit every neighbour in the graph that can be accessed from
* the initial vertex, in a breadth-first-search
* pGraph                pointer to graph
* initialVertex         initial vertex
* visitedFlag           value that will be set to visited vertices
* visitVertexCallback   function that will be called after the
*                       vertex is visited first by the bfs
* visitEdgeCallback     function that will be called after the
*                       edge is visited first by the bfs
* Callbacks can be ommitted by NULL
* Possible error values:
* GRAPH_SEARCH_RETURN_INVALID_PARAMETER
*	- "pGraph" is NULL
* GRAPH_SEARCH_RETURN_VERTEX_ALREADY_VISITED
* GRAPH_SEARCH_RETURN_DOES_NOT_CONTAIN_VERTEX
* GRAPH_SEARCH_RETURN_UNKNOWN_ERROR
* GRAPH_SEARCH_RETURN_MEMORY
*/
GraphSearchReturnID graphBFS(Graph *pGraph,
	void *initialVertex,
	int visitedFlag,
	void (*visitVertexCallback)(void *vertex),
	void (*visitEdgeCallback)(void *source, void *edge, void *dest));

#ifdef _DEBUG
// For memory leak detection use only!
// After a call to BFS, node ref count should be zero
int getGraphSearchNodeRefCount();
#endif

#endif