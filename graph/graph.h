#ifndef __GRAPH_H__
#define __GRAPH_H__

typedef enum
{
    /* All went ok */
    GRAPH_RETURN_OK,

    /* Invalid parameter was provided */
    GRAPH_RETURN_INVALID_PARAMETER,

    /* Could not allocate memory space */
    GRAPH_RETURN_MEMORY,
}
GraphReturnID;

typedef struct Graph Graph;

GraphReturnID GraphCreate(Graph **ppGraph, size_t number_of_vertices,
                                            void (*delete_node_info)(void *));
GraphReturnID GraphDestroy(Graph *pGraph);

#endif
