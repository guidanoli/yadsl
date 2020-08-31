#ifndef __YADSL_GRAPH_IO_H__
#define __YADSL_GRAPH_IO_H__

#include <stdio.h>

#include <graph/graph.h>

/**
 * \defgroup graphio Graph I/O
 * @brief Auxiliary module for Graph I/O utilities
*/

/**
 * @brief Return condition of Graph I/O functions
*/
typedef enum
{
	YADSL_GRAPH_IO_RET_OK = 0, /**< All went ok */
	YADSL_GRAPH_IO_RET_MEMORY, /**< Could not allocate memory */
	YADSL_GRAPH_IO_RET_WRITING_FAILURE, /**< Specific serialization failed */
	YADSL_GRAPH_IO_RET_CREATION_FAILURE, /**< Specific deserialization failed */
	YADSL_GRAPH_IO_RET_SAME_CREATION, /**< Same object deserialized twice */
	YADSL_GRAPH_IO_RET_FILE_ERROR, /**< Could not write to or read from file */
	YADSL_GRAPH_IO_RET_DEPRECATED_FILE_FORMAT, /**< File format is incompatible */
	YADSL_GRAPH_IO_RET_CORRUPTED_FILE_FORMAT, /**< File format is corrupted */
}
yadsl_GraphIoRet;

/**
 * @brief Function responsible for serializing a vertex object
*/
typedef int (*yadsl_GraphIoVertexWriteFunc)(FILE*, yadsl_GraphVertexObject*);

/**
 * @brief Function responsible for serializing an edge object
*/
typedef int (*yadsl_GraphIoEdgeWriteFunc)(FILE*, yadsl_GraphEdgeObject*);

/**
 * @brief Function responsible for deserializing a vertex object
*/
typedef int (*yadsl_GraphIoVertexReadFunc)(FILE*, yadsl_GraphVertexObject**);

/**
 * @brief Function responsible for deserializing an edge object
*/
typedef int (*yadsl_GraphIoEdgeReadFunc)(FILE*, yadsl_GraphEdgeObject**);

/**
 * @brief Serialize graph structure to file
 * @param graph graph
 * @param file_ptr file pointer opened in writing mode
 * @param write_vertex_func vertex serialization function
 * @param write_edge_func edge serialization function
 * @return
 * * ::YADSL_GRAPH_IO_RET_OK, and file is successfuly written
 * * ::YADSL_GRAPH_IO_RET_FILE_ERROR
 * * ::YADSL_GRAPH_IO_RET_WRITING_FAILURE
 * * ::YADSL_GRAPH_IO_RET_MEMORY
*/
yadsl_GraphIoRet yadsl_graph_io_write(
	yadsl_GraphHandle *graph,
	FILE *file_ptr,
	yadsl_GraphIoVertexWriteFunc write_vertex_func,
	yadsl_GraphIoEdgeWriteFunc write_edge_func);

/**
 * @brief Deserialize graph structure from file
 * @param file_ptr file pointer opened for reading
 * @param read_vertex_func vertex deserialization function
 * @param read_edge_func edge deserialization function
 * @param cmp_vertices_func vertex comparison function
 * @param free_vertex_func vertex freeing function
 * @param cmp_edges_func edge comparison function
 * @param free_edge_func edge freeing function
 * @param graph_ptr pointer to graph
 * @return
 * * ::YADSL_GRAPH_IO_RET_OK, and file is successfuly read and *graph_ptr is updated
 * * ::YADSL_GRAPH_IO_RET_FILE_ERROR
 * * ::YADSL_GRAPH_IO_RET_SAME_CREATION
 * * ::YADSL_GRAPH_IO_RET_CREATION_FAILURE
 * * ::YADSL_GRAPH_IO_RET_DEPRECATED_FILE_FORMAT
 * * ::YADSL_GRAPH_IO_RET_CORRUPTED_FILE_FORMAT
 * * ::YADSL_GRAPH_IO_RET_MEMORY
*/
yadsl_GraphIoRet yadsl_graph_io_read(
	FILE *file_ptr,
	yadsl_GraphIoVertexReadFunc read_vertex_func,
	yadsl_GraphIoEdgeReadFunc read_edge_func,
	yadsl_GraphCmpVertexObjsFunc cmp_vertices_func,
	yadsl_GraphFreeVertexObjFunc free_vertex_func,
	yadsl_GraphCmpEdgeObjsFunc cmp_edges_func,
	yadsl_GraphFreeEdgeObjFunc free_edge_func,
	yadsl_GraphHandle **graph_ptr);

#endif
