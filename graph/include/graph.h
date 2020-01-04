#ifndef __GRAPH_H__
#define __GRAPH_H__

/**
* A Graph starts with a fixed number of vertices and no edges.
* It is possible to add and remove edges and check neighbourhoods.
*/

typedef enum
{
    // SEMANTIC RETURN VALUES

    /* Everything went as excepted */
    GRAPH_RETURN_OK,

    /* Tried to operate on the same vertex */
    GRAPH_RETURN_SAME_VERTEX,

    /* Tried to operate out of boundaries */
    GRAPH_RETURN_OUT_OF_BOUNDS,

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

    /* Something fatal occurred and left the structure unstable */
    GRAPH_RETURN_FATAL_ERROR,
}
GraphReturnID;

typedef struct Graph Graph;

/**
* Create a graph with no edges
* ppGraph   address of pointer to graph
* size      maximum number of vertices
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER:
*   - "ppGraph" is NULL
*   - "size" is 0
* GRAPH_RETURN_MEMORY
* GRAPH_RETURN_UNKNOWN_ERROR
*/
GraphReturnID graphCreate(Graph **ppGraph, size_t size);

/**
* Get number of vertices in graph
* pGraph    pointer to graph
* pSize     address to variable that will hold the number of vertices
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER:
*   - "pGraph" is NULL
*   - "pSize" is NULL
*/
GraphReturnID graphGetNumberOfVertices(Graph *pGraph, size_t *pSize);

/**
* Add edge to graph
* pGraph    pointer to graph
* u, v      graph edges
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
* GRAPH_RETURN_SAME_VERTEX
*   - "u" is equal to "v"
* GRAPH_RETURN_OUT_OF_BOUNDS
*   - "u" or "v" is larger than the number of vertices in the graph
* GRAPH_RETURN_CONTAINS_EDGE
*   - Edge "uv" already exists in the graph
* GRAPH_RETURN_UNKNOWN_ERROR
* GRAPH_RETURN_MEMORY
* GRAPH_RETURN_FATAL_ERROR
*/
GraphReturnID graphAddEdge(Graph *pGraph, size_t u, size_t v);

/**
* Remove edge from graph
* pGraph    pointer to graph
* u, v      graph edges
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
* GRAPH_RETURN_SAME_VERTEX
*   - "u" is equal to "v"
* GRAPH_RETURN_OUT_OF_BOUNDS
*   - "u" or "v" is larger than the number of vertices in the graph
* GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE
*   - Edge "uv" does not exist in the graph
* GRAPH_RETURN_UNKNOWN_ERROR
* GRAPH_RETURN_FATAL_ERROR
*/
GraphReturnID graphRemoveEdge(Graph *pGraph, size_t u, size_t v);

/**
* Check whether graph contains edge or not
* pGraph    pointer to graph
* u, v      graph edges
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
* GRAPH_RETURN_CONTAINS_EDGE
*   - Edge "uv" exists in the graph
* GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE
*   - Edge "uv" does not exist in the graph
* GRAPH_RETURN_OUT_OF_BOUNDS
*   - "u" or "v" is larger than the number of vertices in the graph
* GRAPH_RETURN_UNKNOWN_ERROR
*/
GraphReturnID graphContainsEdge(Graph *pGraph, size_t u, size_t v);

/**
* Get number of neighbours of a given edge
* pGraph    pointer to graph
* u         graph edge
* pNum      adress to variable that will hold the number of neighbours of u
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "pNum" is NULL
* GRAPH_RETURN_OUT_OF_BOUNDS
*   - "u" is larger than the number of vertices in the graph
*/

GraphReturnID graphGetNumberOfNeighbours(Graph *pGraph, size_t u, size_t *pNum);

/**
* Get next neighbour in adjecency list (loops)
* pGraph    pointer to graph
* u         graph edge
* pV        adress to variable that will hold neighbour of u
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "pV" is NULL
* GRAPH_RETURN_OUT_OF_BOUNDS
*   - "u" is larger than the number of vertices in the graph
* GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE
*   - "u" does not contain neighbours, that is, edges
*/
GraphReturnID graphGetNextNeighbour(Graph *pGraph, size_t u, size_t *pV);

/**
* Free graph structure from memory
* pGraph    pointer to graph
*/
void graphDestroy(Graph *pGraph);

#endif
