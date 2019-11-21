#ifndef __GRAPH_H__
#define __GRAPH_H__

/**
* A Graph starts with a fixed number of vertices and no edges.
* You are able to add and remove edges and check neighbourhoods.
*/

typedef enum
{
    /* All went ok */
    GRAPH_RETURN_OK,

    /* Invalid parameter was provided */
    GRAPH_RETURN_INVALID_PARAMETER,

    /* Could not allocate memory space */
    GRAPH_RETURN_MEMORY,

    /* Tried to operate on the same vertex */
    GRAPH_RETURN_SAME_VERTEX,

    /* When an internal error is unrecognized */
    GRAPH_RETURN_UNKNOWN_ERROR,
}
GraphReturnID;

typedef struct Graph Graph;

GraphReturnID GraphCreate(Graph **ppGraph, size_t number_of_vertices);
GraphReturnID GraphGetNumberOfVertices(Graph *pGraph, size_t *pNumber);
void GraphDestroy(Graph *pGraph);

#endif
