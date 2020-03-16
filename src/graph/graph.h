#ifndef __GRAPH_H__
#define __GRAPH_H__

//
//     ______                 __
//    / ____/________ _____  / /_
//   / / __/ ___/ __ `/ __ \/ __ \
//  / /_/ / /  / /_/ / /_/ / / / /
//  \____/_/   \__,_/ .___/_/ /_/
//                 /_/
//
//  A Graph starts with no vertices and, therefore, no edges.
// You are able to add and remove vertices and edges, check
// if vertices and edges are contained in graph, iterate through
// vertices and vertex neighbours (in, out or all), obtain
// vertex count, vertex degrees (in, out or total), and set/get
// flags set to vertices (for searches in graph, coloring...)
//
// HINTS
// -----
//
//  Undirected graphs store edges differently than directed
// graphs, but still, in such way that functions that relate to
// in or out neighbours will provide diferent edges, for every
// vertex on a graph. That's why it can be used for serialization.
//

typedef enum
{
	GRAPH_OK = 0,
	GRAPH_EMPTY,
	GRAPH_CONTAINS_VERTEX,
	GRAPH_DOES_NOT_CONTAIN_VERTEX,
	GRAPH_CONTAINS_EDGE,
	GRAPH_DOES_NOT_CONTAIN_EDGE,
	GRAPH_MEMORY,
}
GraphRet;

typedef struct Graph Graph;

#include <stddef.h>

//  ============= ============================================
//   graphCreate            Create an empty graph          
//  ============= ============================================
//   ppGraph       (ret) pointer to graph                
//   cmpVertices   (opt) comparison function between vertices  
//   cmpEdges      (opt) comparison function between edges     
//   freeVertex    (opt) deallocation function for vertices    
//   freeEdge      (opt) deallocation function for edges       
//  ============= ============================================
//  [!] GRAPH_MEMORY

GraphRet graphCreate(Graph **ppGraph,
	int isDirected,
	int (*cmpVertices)(void *a, void *b),
	void (*freeVertex)(void *v),
	int (*cmpEdges)(void *a, void *b),
	void (*freeEdge)(void *e));

//  ================= ======================================== 
//   graphIsDirected   Check whether graph is directed or not  
//  ================= ======================================== 
//   pGraph            pointer to graph                        
//   pIsDirected       (ret) whether graph is directed or not  
//  ================= ======================================== 

GraphRet graphIsDirected(Graph *pGraph, int *pIsDirected);

//  ========================== ================================= 
//   graphGetNumberOfVertices   Get number of vertices in graph  
//  ========================== ================================= 
//   pGraph                     pointer to graph                 
//   pNumberOfVertices          (ret) number of vertices         
//  ========================== ================================= 

GraphRet graphGetNumberOfVertices(Graph *pGraph, size_t *pNumberOfVertices);

//  ==================== ================================= 
//   graphGetNextVertex   Get next vertex (loops)
//  ==================== ================================= 
//   pGraph               pointer to graph                 
//   pNextVertex          (ret) next vertex                
//  ==================== ================================= 
//  [!] GRAPH_EMPTY

GraphRet graphGetNextVertex(Graph *pGraph, void **pNextVertex);

//  ======================== ============================= 
//   graphGetPreviousVertex   Get previous vertex (loops)  
//  ======================== ============================= 
//   pGraph                   pointer to graph             
//   pPreviousVertex          (ret) previous vertex        
//  ======================== ============================= 
//  [!] GRAPH_EMPTY

GraphRet graphGetPreviousVertex(Graph *pGraph, void **pPreviousVertex);

//  ========================= ========================= 
//   graphGetVertexOutDegree    Get vertex out degree   
//  ========================= ========================= 
//   pGraph                    pointer to graph         
//   vertex                    graph vertex             
//   pOutDegree                (ret) vertex out degree  
//  ========================= ========================= 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX

GraphRet graphGetVertexOutDegree(Graph *pGraph,
	void *vertex, size_t *pOutDegree);

//  ======================== ======================== 
//   graphGetVertexInDegree    Get vertex in degree   
//  ======================== ======================== 
//   pGraph                   pointer to graph        
//   vertex                   graph vertex            
//   pInDegree                (ret) vertex in degree  
//  ======================== ======================== 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX

GraphRet graphGetVertexInDegree(Graph *pGraph,
	void *vertex, size_t *pInDegree);

//  ======================== =========================== 
//   graphGetVertexInDegree   Get vertex (total) degree  
//  ======================== =========================== 
//   pGraph                   pointer to graph           
//   vertex                   graph vertex               
//   pDegree                  (ret) vertex degree        
//  ======================== =========================== 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX

GraphRet graphGetVertexDegree(Graph *pGraph,
	void *vertex, size_t *pDegree);

//  ======================= ============================================= 
//   graphGetNextNeighbour       Get next neighbour of a given vertex       
//  ======================= ============================================= 
//   pGraph                  pointer to graph                             
//   vertex                  graph vertex                                 
//   pNextNeighbour          (ret) next neighbour of vertex               
//   pEdge                   (ret) edge between vertex and its neighbour  
//  ======================= ============================================= 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX
//  [!] GRAPH_DOES_NOT_CONTAIN_EDGE

GraphRet graphGetNextNeighbour(Graph *pGraph,
	void *vertex, void **pNextNeighbour, void **pEdge);

//  =========================== ============================================= 
//   graphGetPreviousNeighbour     Get previous neighbour of a given vertex     
//  =========================== ============================================= 
//   pGraph                      pointer to graph                             
//   vertex                      graph vertex                                 
//   pPreviousNeighbour          (ret) previous neighbour of vertex           
//   pEdge                       (ret) edge between vertex and its neighbour  
//  =========================== ============================================= 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX
//  [!] GRAPH_DOES_NOT_CONTAIN_EDGE

GraphRet graphGetPreviousNeighbour(Graph *pGraph,
	void *u, void **pV, void **uv);

//  ========================= ============================================= 
//   graphGetNextInNeighbour     Get next in neighbour of a given vertex    
//  ========================= ============================================= 
//   pGraph                    pointer to graph                             
//   vertex                    graph vertex                                 
//   pNextInNeighbour          (ret) next in neighbour of vertex            
//   pEdge                     (ret) edge between vertex and its neighbour  
//  ========================= ============================================= 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX
//  [!] GRAPH_DOES_NOT_CONTAIN_EDGE

GraphRet graphGetNextInNeighbour(Graph *pGraph,
	void *vertex, void **pNextInNeighbour, void **pEdge);

//  ============================= ============================================= 
//   graphGetPreviousInNeighbour   Get previous in neighbour of a given vertex  
//  ============================= ============================================= 
//   pGraph                        pointer to graph                             
//   vertex                        graph vertex                                 
//   pPreviousInNeighbour          (ret) previous in neighbour of vertex        
//   pEdge                         (ret) edge between vertex and its neighbour  
//  ============================= ============================================= 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX
//  [!] GRAPH_DOES_NOT_CONTAIN_EDGE

GraphRet graphGetPreviousInNeighbour(Graph *pGraph,
	void *vertex, void **pPreviousInNeighbour, void **pEdge);

//  ========================== ============================================= 
//   graphGetNextOutNeighbour    Get next out neighbour of a given vertex    
//  ========================== ============================================= 
//   pGraph                     pointer to graph                             
//   vertex                     graph vertex                                 
//   pNextOutNeighbour          (ret) next out neighbour of vertex           
//   pEdge                      (ret) edge between vertex and its neighbour  
//  ========================== ============================================= 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX
//  [!] GRAPH_DOES_NOT_CONTAIN_EDGE

GraphRet graphGetNextOutNeighbour(Graph *pGraph,
	void *vertex, void **pNextOutNeighbour, void **pEdge);

//  ============================== ============================================= 
//   graphGetPreviousOutNeighbour   Get previous out neighbour of a vertex  
//  ============================== ============================================= 
//   pGraph                         pointer to graph                              
//   vertex                         graph vertex                                  
//   pPreviousOutNeighbour          (ret) previous out neighbour of vertex        
//   pEdge                          (ret) edge between vertex and its neighbour   
//  ============================== ============================================= 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX
//  [!] GRAPH_DOES_NOT_CONTAIN_EDGE

GraphRet graphGetPreviousOutNeighbour(Graph *pGraph,
	void *vertex, void **pPreviousOutNeighbour, void **pEdge);

//  ===================== ============================================ 
//   graphContainsVertex   Check whether graph contains vertex or not  
//  ===================== ============================================ 
//   pGraph                pointer to graph                            
//   vertex                graph vertex                                
//   pContains             (ret) whether graph contains vertex or not  
//  ===================== ============================================ 

GraphRet graphContainsVertex(Graph *pGraph, void *vertex, int *pContains);

//  ================ ===================== 
//   graphAddVertex   Add vertex to graph  
//  ================ ===================== 
//   pGraph           pointer to graph     
//   vertex           graph vertex         
//  ================ ===================== 
//  [!] GRAPH_CONTAINS_VERTEX
//  [!] GRAPH_MEMORY

GraphRet graphAddVertex(Graph *pGraph, void *vertex);

//  =================== ======================== 
//   graphRemoveVertex   Remove vertex to graph  
//  =================== ======================== 
//   pGraph              pointer to graph        
//   vertex              graph vertex            
//  =================== ======================== 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX

GraphRet graphRemoveVertex(Graph *pGraph, void *vertex);

//  =================== ========================================== 
//   graphContainsEdge   Check whether graph contains edge or not  
//  =================== ========================================== 
//   pGraph              pointer to graph                          
//   u, v                graph vertices                            
//   pContains           (ret) whether graph contains edge or not  
//  =================== ========================================== 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX

GraphRet graphContainsEdge(Graph *pGraph, void *u, void *v, int *pContains);

//  ============== ====================== 
//   graphAddEdge    Add edge to graph    
//  ============== ====================== 
//   pGraph         pointer to graph      
//   u, v           graph vertices        
//   uv             edge between u and v  
//  ============== ====================== 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX
//  [!] GRAPH_CONTAINS_EDGE
//  [!] GRAPH_MEMORY
//  [!] Alters the state of the neighbour iterator functions

GraphRet graphAddEdge(Graph *pGraph, void *u, void *v, void *uv);

//  ============== =========================================== 
//   graphGetEdge   Obtain edge between two vertices in graph  
//  ============== =========================================== 
//   pGraph         pointer to graph                           
//   u, v           graph vertices                             
//   pEdge          (ret) edge between u and v                 
//  ============== =========================================== 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX
//  [!] GRAPH_DOES_NOT_CONTAIN_EDGE

GraphRet graphGetEdge(Graph *pGraph, void *u, void *v, void **pEdge);

//  ================= ======================== 
//   graphRemoveEdge   Remove edge from graph  
//  ================= ======================== 
//   pGraph            pointer to graph        
//   u, v              graph vertices          
//  ================= ======================== 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX
//  [!] GRAPH_DOES_NOT_CONTAIN_EDGE
//  [!] Alters the state of the neighbour iterator functions

GraphRet graphRemoveEdge(Graph *pGraph, void *u, void *v);

//  ==================== ======================================== 
//   graphGetVertexFlag   Get flag associated to vertex in graph  
//  ==================== ======================================== 
//   pGraph               pointer to graph                        
//   vertex               graph vertex                            
//   pFlag                (ret) flag associated to vertex         
//  ==================== ======================================== 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX

GraphRet graphGetVertexFlag(Graph *pGraph, void *v, int *pFlag);

//  ==================== ======================================== 
//   graphSetVertexFlag   Set flag associated to vertex in graph  
//  ==================== ======================================== 
//   pGraph               pointer to graph                        
//   vertex               graph vertex                            
//   flag                 flag to be associated to vertex         
//  ==================== ======================================== 
//  [!] GRAPH_DOES_NOT_CONTAIN_VERTEX

GraphRet graphSetVertexFlag(Graph *pGraph, void *v, int flag);

//  ========================= ======================================= 
//   graphSetAllVerticesFlag    Sets flag to all vertices in graph    
//  ========================= ======================================= 
//   pGraph                    pointer to graph                       
//   flag                      flag to be associated to all vertices  
//  ========================= ======================================= 

GraphRet graphSetAllVerticesFlags(Graph *pGraph, int flag);

//  ============== ========================================= 
//   graphDestroy   Deallocates graph structure from memory  
//  ============== ========================================= 
//   pGraph         pointer to graph                         
//  ============== ========================================= 

void graphDestroy(Graph *pGraph);

#endif
