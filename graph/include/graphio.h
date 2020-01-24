#ifndef __GRAPH_IO_H__
#define __GRAPH_IO_H__

#include <stdio.h>
#include "graph.h"

/**
* Graph IO library
*/

typedef enum
{
    // SEMANTIC RETURN VALUES

    /* Everything went as excepted */
    GRAPH_IO_RETURN_OK = 0,
    
    // ERROR RETURN VALUES

    /* Invalid parameter was provided */
    GRAPH_IO_RETURN_INVALID_PARAMETER,

    /* Could not allocate memory space */
    GRAPH_IO_RETURN_MEMORY,

    /* Could not write vertex to file */
    GRAPH_IO_RETURN_WRITING_FAILURE,

    /* Could not create specific vertex */
    GRAPH_IO_RETURN_CREATION_FAILURE,

    /* The "readVertex" created vertex copies */
    GRAPH_IO_RETURN_SAME_CREATION,

    /* Could not write to or read from file */
    GRAPH_IO_RETURN_FILE_ERROR,

    /* File format is deprecated */
    GRAPH_IO_RETURN_DEPRECATED_FILE_FORMAT,

    /* File format corruption detected */
    GRAPH_IO_RETURN_CORRUPTED_FILE_FORMAT,

    /* When an internal error is unrecognized */
    GRAPH_IO_RETURN_UNKNOWN_ERROR,

    /* The structure is corrupted and behaviour is unpredictable */
    GRAPH_IO_RETURN_FATAL_ERROR,
}
GraphIoReturnID;

/**
* Serialize graph structure to file
* pGraph        pointer to graph
* fp            pointer to file to be written
* writeVertex   callback function called to write to the
*               file "fp" the contents of the vertex "v"
*               Returns 0 on success and else on failure
*               [!] Separation characters (spaces, tabs and
*               line feeds) in the beggining or in the end
*               will be deliberately ignored when reading.
* writeEdge     callback function called to write to the
*               file "fp" the contents of the edge "e"
*               Returns 0 on success and else on failure
*               [!] Separation characters (spaces, tabs and
*               line feeds) in the beggining or in the end
*               will be deliberately ignored when reading.
* Possible errors:
* GRAPH_IO_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "writeVertex" is NULL
*   - "writeEdge" is NULL
* GRAPH_IO_RETURN_FILE_ERROR
*   - Could not write to file
* GRAPH_IO_RETURN_WRITING_FAILURE
* GRAPH_IO_RETURN_UNKNOWN_ERROR
* [!] The module does not take ownership of the file
* pointer. It must be previously opened in writing mode
* and closed afterwards by the caller.
*/
GraphIoReturnID graphWrite(Graph *pGraph, FILE *fp,
    int (*writeVertex)(FILE *fp, void *v),
    int (*writeEdge)(FILE *fp, void *e));

/**
* Serialize graph structure to file
* ppGraph       address of pointer to graph
* fp            pointer to file to be read
* readVertex    function responsible for reading
*               one vertex from the file stream.
*               Returns vertex by reference
*               [!] Cannot return copies, otherwise
*               indicates GRAPH_IO_RETURN_SAME_CREATION
*               returns 0 if successful, else, failed
* readEdge      function responsible for reading
*               one edge from the file stream.
*               Returns edge by reference
*               returns 0 if successful, else, failed
* cmpVertices   function responsible for comparing
*               vertices (0 = different, else, equal)
* freeVertex    function responsible for freeing
*               one vertex
* cmpEdges      function responsible for comparing
*               edges (0 = different, else, equal)
* freeEdge      function responsible for freeing
*               one edge
* Possible errors:
* GRAPH_IO_RETURN_INVALID_PARAMETER
*   - "ppGraph" is NULL
*   - "readVertex" is NULL
*   - "readEdge" is NULL
* GRAPH_IO_RETURN_FILE_ERROR
*   - Could not read file
* GRAPH_IO_RETURN_SAME_CREATION
* GRAPH_IO_RETURN_CREATION_FAILURE
* GRAPH_IO_RETURN_DEPRECATED_FILE_FORMAT
* GRAPH_IO_RETURN_CORRUPTED_FILE_FORMAT
* GRAPH_IO_RETURN_MEMORY
* GRAPH_IO_RETURN_FATAL_ERROR
* GRAPH_IO_RETURN_UNKNOWN_ERROR
* [!] The module does not take ownership of the file
* pointer. It must be previously opened in reading mode
* and closed afterwards by the caller.
*/
GraphIoReturnID graphRead(Graph **ppGraph, FILE *fp,
    int (*readVertex)(FILE *fp, void **ppVertex),
    int (*readEdge)(FILE *fp, void **ppEdge),
    int (*cmpVertices)(void *a, void *b),
    void (*freeVertex)(void *v),
    int (*cmpEdges)(void *a, void *b),
    void (*freeEdge)(void *e));

#endif
