#ifndef __GRAPH_H__
#define __GRAPH_H__

/**
* A Graph starts with no vertices and, therefore, no edges.
* It is possible to add and remove edges and check neighbourhoods.
*
* Observation: on the documentation of every function, it might
* be written "Possible errors" (which disconsider GRAPH_RETURN_OK),
* or "Possible return values" (which consider all values possible).
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

typedef enum
{
    /* Undirected graph */
    GRAPH_EDGE_TYPE_UNDIRECTED,

    /* Directed graph */
    GRAPH_EDGE_TYPE_DIRECTED,

    // Technical - don't use this
    GRAPH_EDGE_TYPE_INVALID,
}
GraphEdgeType;

typedef struct Graph Graph;

/**
* Create an empty graph
* ppGraph       address of pointer to graph
* type          type of graph
* cmpVertices   function that compares two
*               vertex data structures and
*               returns 0 if they are different
*               (and if else, they are equal)
*               if NULL, does a shallow comparison
* freeVertex    function that frees vertex
*               data structure from memory
*               if NULL, does nothing to vertex
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "ppGraph" is NULL
*   - "type" is invalid
* GRAPH_RETURN_MEMORY
*/
GraphReturnID graphCreate(Graph **ppGraph, GraphEdgeType type,
    int (*cmpVertices)(void *a, void *b),
    void (*freeVertex)(void *v));

/**
* Get graph type
* pGraph    pointer to graph
* pType     adress of variable that will hold the graph type
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "pType" is NULL
*/
GraphReturnID graphGetType(Graph *pGraph, GraphEdgeType *pType);

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
* Get number of neighbours of a given vertex
* pGraph    pointer to graph
* v         graph vertex
* pNum      adress to variable that will hold the number of neighbours of v
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "pNum" is NULL
* GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
*   - vertex "v" does not exist
* GRAPH_RETURN_UNKNOWN_ERROR
*/
GraphReturnID graphGetNumberOfNeighbours(Graph *pGraph, void *v, unsigned long *pNum);

/**
* Get next neighbour in adjecency list (loops)
* pGraph    pointer to graph
* u         graph vertex
* pV        adress to variable that will hold neighbour of u
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "pV" is NULL
* GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
*   - vertex "v" does not exist
* GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE
*   - "u" does not contain neighbours, that is, edges
*/
GraphReturnID graphGetNextNeighbour(Graph *pGraph, void *v, void **pV);

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
*/
GraphReturnID graphAddEdge(Graph *pGraph, void *u, void *v);

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
*/
GraphReturnID graphRemoveEdge(Graph *pGraph, void *u, void *v);

/**
* Free graph structure from memory
* pGraph    pointer to graph
*/
void graphDestroy(Graph *pGraph);

#endif
