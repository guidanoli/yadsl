#ifndef __GRAPH_SEARCH_H__
#define __GRAPH_SEARCH_H__

#include <graph/graph.h>

//
//     ______                 __   _____                      __  
//    / ____/________ _____  / /_ / ___/___  ____ ___________/ /_ 
//   / / __/ ___/ __ `/ __ \/ __ \\__ \/ _ \/ __ `/ ___/ ___/ __ \
//  / /_/ / /  / /_/ / /_/ / / / /__/ /  __/ /_/ / /  / /__/ / / /
//  \____/_/   \__,_/ .___/_/ /_/____/\___/\__,_/_/   \___/_/ /_/ 
//                 /_/                                            
//
//  Auxiliary module for searching in Graphs with algorithms such as
// Depth-First Search (DFS) and Breadth-First Search (BFS) in an almost
// fully customizable way, by providing custom callbacks that are triggered
// every time an unvisited vertex or edge is visited.
//

typedef enum
{
	GRAPH_SEARCH_OK = 0,
	GRAPH_SEARCH_DOES_NOT_CONTAIN_VERTEX,
	GRAPH_SEARCH_VERTEX_ALREADY_VISITED,
	GRAPH_SEARCH_MEMORY,
}
GraphSearchRet;

//  ===================== ================================================= 
//        graphDFS         Visit the graph in a depth-first search fashion  
//  ===================== ================================================= 
//   graph                pointer to graph                                 
//   initialVertex         initial vertex                                   
//   visitedFlag           value that will be set to visited vertices       
//   visit_vertex_func   vertex visiting function                         
//   visit_edge_func     edge visiting function                           
//  ===================== ================================================= 
//  [!] GRAPH_SEARCH_VERTEX_ALREADY_VISITED
//  [!] GRAPH_SEARCH_DOES_NOT_CONTAIN_VERTEX

GraphSearchRet graphDFS(yadsl_GraphHandle *graph,
	void *initialVertex,
	int visitedFlag,
	void (*visit_vertex_func)(void *vertex),
	void (*visit_edge_func)(void *source, void *edge, void *dest));

//  ===================== =================================================== 
//        graphBFS         Visit the graph in a breadth-first search fashion  
//  ===================== =================================================== 
//   graph                pointer to graph                                   
//   initialVertex         initial vertex                                     
//   visitedFlag           value that will be set to visited vertices         
//   visit_vertex_func   vertex visiting function                           
//   visit_edge_func     edge visiting function                             
//  ===================== =================================================== 
//  [!] GRAPH_SEARCH_VERTEX_ALREADY_VISITED
//  [!] GRAPH_SEARCH_DOES_NOT_CONTAIN_VERTEX
//  [!] GRAPH_SEARCH_MEMORY

GraphSearchRet graphBFS(yadsl_GraphHandle *graph,
	void *initialVertex,
	int visitedFlag,
	void (*visit_vertex_func)(void *vertex),
	void (*visit_edge_func)(void *source, void *edge, void *dest));

#ifdef _DEBUG
// For memory leak detection use only!
// After a call to BFS, node ref count should be zero
int getGraphSearchNodeRefCount();
#endif

#endif