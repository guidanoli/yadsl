#ifndef __GRAPH_H__
#define __GRAPH_H__

/**
* A Graph starts with no vertices and, therefore, no edges.
* It is possible to add and remove edges and check neighbourhoods.
*
* Observations:
* - On the documentation of every function, it might be written
* "Possible errors" (which disconsider GRAPH_RETURN_OK), or
* "Possible return values" (which consider all values possible).
* - Undirected graphs store edges differently than directed
* graphs, but still, in such way that graphGetNextOutNeighbour
* or graphGetNextInNeighbour will provide unique edges, for every
* vertex on a graph. That's why it can be used for serialization.
*
* HINT: SET_RETURN_OK will always be 0, therefore it can be used
* as a boolean value to check if a function went OK or not, eg:
* if (setId = setFunction(pSet)) { ... }
*/

typedef enum
{
    // SEMANTIC RETURN VALUES

    /* Everything went as excepted */
    GRAPH_RETURN_OK = 0,

    /* Tried to operate on the same vertex */
    GRAPH_RETURN_SAME_VERTEX,
        
    /* Graph does not contain any vertices whatsoever */
    GRAPH_RETURN_EMPTY,

    /* Graph contains vertex */
    GRAPH_RETURN_CONTAINS_VERTEX,

    /* Graph does not contain vertex */
    GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX,

    /* Graph contains edge */
    GRAPH_RETURN_CONTAINS_EDGE,

    /* Graph does not contain edge */
    GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE,
    
    // ERROR RETURN VALUES

    /* Invalid parameter was provided */
    GRAPH_RETURN_INVALID_PARAMETER,

    /* Could not allocate memory space */
    GRAPH_RETURN_MEMORY,
    
    /* When an internal error is unrecognized */
    GRAPH_RETURN_UNKNOWN_ERROR,

    /* The structure is corrupted and behaviour is unpredictable */
    GRAPH_RETURN_FATAL_ERROR,
}
GraphReturnID;

typedef struct Graph Graph;

/**
* Create an empty graph
* ppGraph           address of pointer to graph
* isDirected        whether graph is directed or not
* cmpVertices,      function that compares two
* cmpEdges          data structures (vertices or edges)
*                   and returns 0 if they are different
*                   (and if else, they are equal)
*                   if NULL, does a shallow comparison
* freeVertex,       function that frees a foreign data
* cmpEdges          structure (vertex or edge) from memory
*                   if NULL, does nothing to vertex
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "ppGraph" is NULL
* GRAPH_RETURN_MEMORY
*/
GraphReturnID graphCreate(Graph **ppGraph,
    int isDirected,
    int (*cmpVertices)(void *a, void *b),
    void (*freeVertex)(void *v),
    int (*cmpEdges)(void *a, void *b),
    void (*freeEdge)(void *e));

/**
* Check whether graph is directed or not
* pGraph        pointer to graph
* pIsDirected   (return) whether graph is directed or not
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "pIsDirected" is NULL
*/
GraphReturnID graphIsDirected(Graph *pGraph, int *pIsDirected);

/**
* Get number of vertices in graph
* pGraph    pointer to graph
* pSize     address of variable that will hold the number of vertices
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "pSize" is NULL
* GRAPH_RETURN_UNKNOWN_ERROR
*/
GraphReturnID graphGetNumberOfVertices(Graph *pGraph, unsigned long *pSize);

/**
* Get next vertex in graph (loops)
* pGraph    pointer to graph
* pV        address of variable that will hold the vertex
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "pV" is NULL
* GRAPH_RETURN_EMPTY
* GRAPH_RETURN_UNKNOWN_ERROR
*/
GraphReturnID graphGetNextVertex(Graph *pGraph, void **pV);

/**
* Get out degree of a given vertex, that is, how many edges come from it
* pGraph    pointer to graph
* v         graph vertex
* pOut      (return) out degree of v
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "pOut" is NULL
* GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
*   - vertex "v" does not exist
* GRAPH_RETURN_UNKNOWN_ERROR
*/
GraphReturnID graphGetVertexOutDegree(Graph *pGraph, void *v, unsigned long *pOut);

/**
* Get in degree of a given vertex, that is, how many edges go to it
* pGraph    pointer to graph
* v         graph vertex
* pIn       (return) in degree of v
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "pIn" is NULL
* GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
*   - vertex "v" does not exist
* GRAPH_RETURN_UNKNOWN_ERROR
*/
GraphReturnID graphGetVertexInDegree(Graph *pGraph, void *v, unsigned long *pIn);

/**
* Get degree of a given vertex, that is, how many edges are incident to it
* It would be the sum of the in degree and out degree for directed graphs
* Or the number of neighbouring vertices for undirected graphs
* pGraph    pointer to graph
* v         graph vertex
* pDegree   (return) degree of v
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "pDegree" is NULL
* GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
*   - vertex "v" does not exist
* GRAPH_RETURN_UNKNOWN_ERROR
*/
GraphReturnID graphGetVertexDegree(Graph *pGraph, void *v, unsigned long *pDegree);

/**
* Get next neighbour of given vertex
* The function will loop through the same vertices until some
* modification is made in relation to this vertex, such as
* adding or removing edges connected to it
* pGraph    pointer to graph
* u         graph vertex
* pV        (return) neighbour of u
* uv        (return) edge between u and v
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "pV" is NULL
*   - "uv" is NULL
* GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
*   - vertex "u" does not exist
* GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE
*   - "u" does not contain neighbours, that is, edges
* GRAPH_RETURN_UNKNOWN_ERROR
*/
GraphReturnID graphGetNextNeighbour(Graph *pGraph, void *u, void **pV, void **uv);

/**
* Get next neighbour that has an edge that incides in a given vertex
* The function will loop through the same vertices until some
* modification is made in relation to this vertex, such as
* adding or removing edges connected to it
* pGraph    pointer to graph
* u         graph vertex
* pV        (return) neighbour of u, through edge "vu"
* uv        (return) edge between u and v
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "pV" is NULL
*   - "uv" is NULL
* GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
*   - vertex "u" does not exist
* GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE
*   - "u" does not contain in-neighbours
* GRAPH_RETURN_UNKNOWN_ERROR
*/
GraphReturnID graphGetNextInNeighbour(Graph *pGraph, void *u, void **pV, void **uv);

/**
* Get next neighbour that has an edge that comes from a given vertex
* The function will loop through the same vertices until some
* modification is made in relation to this vertex, such as
* adding or removing edges connected to it
* pGraph    pointer to graph
* u         graph vertex
* pV        (return) neighbour of u, through edge "uv"
* uv        (return) edge between u and v
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "pV" is NULL
*   - "uv" is NULL
* GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
*   - vertex "u" does not exist
* GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE
*   - "u" does not contain out-neighbours
* GRAPH_RETURN_UNKNOWN_ERROR
*/
GraphReturnID graphGetNextOutNeighbour(Graph *pGraph, void *u, void **pV, void **uv);

/**
* Check whether graph contains vertex or not
* pGraph    pointer to graph
* v         graph vertex
* Possible return values:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
* GRAPH_RETURN_CONTAINS_VERTEX
*   - vertex "v" exists in the graph
* GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
*   - vertex "v" does not exist in the graph
*/
GraphReturnID graphContainsVertex(Graph *pGraph, void *v);

/**
* Add vertex to graph
* pGraph    pointer to graph
* v         graph vertex
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
* GRAPH_RETURN_CONTAINS_VERTEX
*   - graph already contains vertex "v"
* GRAPH_RETURN_UNKNOWN_ERROR
* GRAPH_RETURN_MEMORY
*/
GraphReturnID graphAddVertex(Graph *pGraph, void *v);

/**
* Remove vertex and all edges containing it
* pGraph    pointer to graph
* v         graph vertex
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
* GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
* GRAPH_RETURN_UNKNOWN_ERROR
* GRAPH_RETURN_FATAL_ERROR
*/
GraphReturnID graphRemoveVertex(Graph *pGraph, void *v);

/**
* Check whether graph contains edge or not
* pGraph    pointer to graph
* u, v      graph vertices
* Possible return values:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
* GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
*   - vertex "u" or vertex "v" does not exist
* GRAPH_RETURN_CONTAINS_EDGE
*   - Edge "uv" exists in the graph
* GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE
*   - Edge "uv" does not exist in the graph
* GRAPH_RETURN_UNKNOWN_ERROR
*/
GraphReturnID graphContainsEdge(Graph *pGraph, void *u, void *v);

/**
* Add edge to graph
* pGraph    pointer to graph
* u, v      graph vertices
* uv        edge between u and u
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
* GRAPH_RETURN_SAME_VERTEX
*   - "u" is equal to "v"
* GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
*   - vertex "u" or vertex "v" does not exist
* GRAPH_RETURN_CONTAINS_EDGE
*   - Edge "uv" already exists in the graph
* GRAPH_RETURN_UNKNOWN_ERROR
* GRAPH_RETURN_MEMORY
* GRAPH_RETURN_FATAL_ERROR
* [!] Alters the state of the iterator provided by the
* graphGetNext*Neighbour functions for the manipulated
* vertices u and v, if the function is successful
*/
GraphReturnID graphAddEdge(Graph *pGraph, void *u, void *v, void *uv);

/**
* Obtain edge from vertices provided
* pGraph    pointer to graph
* u, v      graph vertices
* uv        (return) edge between u and v
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
* GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
*   - vertex "u" or vertex "v" does not exist
* GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE
*   - Edge "uv" does not exist in the graph
* GRAPH_RETURN_UNKNOWN_ERROR
*/
GraphReturnID graphGetEdge(Graph *pGraph, void *u, void *v, void **uv);

/**
* Remove edge from graph
* pGraph    pointer to graph
* u, v      graph vertices
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
* GRAPH_RETURN_SAME_VERTEX
*   - "u" is equal to "v"
* GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
*   - vertex "u" or vertex "v" does not exist
* GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE
*   - Edge "uv" does not exist in the graph
* GRAPH_RETURN_UNKNOWN_ERROR
* GRAPH_RETURN_FATAL_ERROR
* [!] Alters the state of the iterator provided by the
* graphGetNext*Neighbour functions for the manipulated
* vertices u and v, if the function is successful
*/
GraphReturnID graphRemoveEdge(Graph *pGraph, void *u, void *v);

/**
* Free graph structure from memory
* pGraph    pointer to graph
*/
void graphDestroy(Graph *pGraph);

#endif
