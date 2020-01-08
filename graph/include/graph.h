#ifndef __GRAPH_H__
#define __GRAPH_H__

#include <stdio.h>

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

    /* Could not write to or read from file */
    GRAPH_RETURN_FILE_ERROR,

    /* File format is deprecated */
    GRAPH_RETURN_DEPRECATED_FILE_FORMAT,

    /* File format corruption detected */
    GRAPH_RETURN_CORRUPTED_FILE_FORMAT,

    /* When an internal error is unrecognized */
    GRAPH_RETURN_UNKNOWN_ERROR,

    /* Something fatal occurred and left the structure unstable */
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
* Create a graph with no edges
* ppGraph   address of pointer to graph
* size      maximum number of vertices
* type      type of graph
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER:
*   - "ppGraph" is NULL
*   - "size" is 0
* GRAPH_RETURN_MEMORY
* GRAPH_RETURN_UNKNOWN_ERROR
*/
GraphReturnID graphCreate(Graph **ppGraph, unsigned long size, GraphEdgeType type);

/**
* Get number of vertices in graph
* pGraph    pointer to graph
* pSize     address of variable that will hold the number of vertices
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER:
*   - "pGraph" is NULL
*   - "pSize" is NULL
*/
GraphReturnID graphGetNumberOfVertices(Graph *pGraph, unsigned long *pSize);

/**
* Get graph type
* pGraph    pointer to graph
* pType     adress of variable that will hold the graph type
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER:
*   - "pGraph" is NULL
*   - "pType" is NULL
*/
GraphReturnID graphGetType(Graph *pGraph, GraphEdgeType *pType);

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
GraphReturnID graphAddEdge(Graph *pGraph, unsigned long u, unsigned long v);

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
GraphReturnID graphRemoveEdge(Graph *pGraph, unsigned long u, unsigned long v);

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
GraphReturnID graphContainsEdge(Graph *pGraph, unsigned long u, unsigned long v);

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

GraphReturnID graphGetNumberOfNeighbours(Graph *pGraph, unsigned long u, unsigned long *pNum);

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
GraphReturnID graphGetNextNeighbour(Graph *pGraph, unsigned long u, unsigned long *pV);

/**
* Serialize graph structure to file
* pGraph    pointer to graph
* f         pointer to file to be written
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
* GRAPH_RETURN_FILE_ERROR
*   - Could not write to file
* GRAPH_RETURN_UNKNOWN_ERROR
* [!] The module does not take ownership of the file
* pointer. It must be previously opened in writing mode
* and closed afterwards by the caller.
*/
GraphReturnID graphWrite(Graph *pGraph, FILE *fp);

/**
* Serialize graph structure to file
* ppGraph   adress of pointer to graph
* f         pointer to file to be read
* Possible errors:
* GRAPH_RETURN_INVALID_PARAMETER
*   - "ppGraph" is NULL
* GRAPH_RETURN_FILE_ERROR
*   - Could not read file
* GRAPH_RETURN_DEPRECATED_FILE_FORMAT
* GRAPH_RETURN_CORRUPTED_FILE_FORMAT
* GRAPH_RETURN_MEMORY
* GRAPH_RETURN_FATAL_ERROR
* GRAPH_RETURN_UNKNOWN_ERROR
* [!] The module does not take ownership of the file
* pointer. It must be previously opened in reading mode
* and closed afterwards by the caller.
*/
GraphReturnID graphRead(Graph **ppGraph, FILE *fp);

/**
* Free graph structure from memory
* pGraph    pointer to graph
*/
void graphDestroy(Graph *pGraph);

#endif
