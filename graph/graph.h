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

    /* Tried to operate out of boundaries */
    GRAPH_RETURN_OUT_OF_BOUNDS,

    /* Graph contains edge */
    GRAPH_RETURN_CONTAINS_EDGE,

    /* Graph does not contain edge */
    GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE,

    /* When an internal error is unrecognized */
    GRAPH_RETURN_UNKNOWN_ERROR,

    /* Something fatal occurred and left the structure unstable */
    GRAPH_RETURN_FATAL_ERROR,
}
GraphReturnID;

typedef struct Graph Graph;

GraphReturnID graphCreate(Graph **ppGraph, size_t size);
GraphReturnID graphGetNumberOfVertices(Graph *pGraph, size_t *pSize);
GraphReturnID graphAddEdge(Graph *pGraph, size_t u, size_t v);
GraphReturnID graphRemoveEdge(Graph *pGraph, size_t u, size_t v);
GraphReturnID graphContainsEdge(Graph *pGraph, size_t u, size_t v);
void graphDestroy(Graph *pGraph);

#endif
