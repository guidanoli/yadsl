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
*               [!] Separation characters (spaces, tabs and
*               line feeds) in the beggining or in the end
*               will be deliberately ignored when reading.
* Possible errors:
* GRAPH_IO_RETURN_INVALID_PARAMETER
*   - "pGraph" is NULL
*   - "writeVertex" is NULL
* GRAPH_IO_RETURN_FILE_ERROR
*   - Could not write to file
* GRAPH_IO_RETURN_UNKNOWN_ERROR
* [!] The module does not take ownership of the file
* pointer. It must be previously opened in writing mode
* and closed afterwards by the caller.
*/
GraphIoReturnID graphWrite(Graph *pGraph, FILE *fp,
    void (*writeVertex)(FILE *fp, void *v));

/**
* Serialize graph structure to file
* ppGraph       adress of pointer to graph
* fp            pointer to file to be read
* readVertex    function responsible for reading
*               one vertex from the file stream
*               returns vertex by reference
*               [!] Cannot return copies, otherwise
*               indicates GRAPH_IO_RETURN_SAME_CREATION
*               returns 0 if successful
*                       else, indicates failure by
*                       GRAPH_IO_RETURN_CREATION_FAILURE
* freeVertex    function responsible for freeing
*               one vertex in case of failure
* Possible errors:
* GRAPH_IO_RETURN_INVALID_PARAMETER
*   - "ppGraph" is NULL
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
    void (*freeVertex)(void *v));

#endif
