#ifndef __GRAPH_IO_H__
#define __GRAPH_IO_H__

#include <stdio.h>

#include "graph.h"

//
//     ______                 __    ________ 
//    / ____/________ _____  / /_  /  _/ __ \
//   / / __/ ___/ __ `/ __ \/ __ \ / // / / /
//  / /_/ / /  / /_/ / /_/ / / / // // /_/ / 
//  \____/_/   \__,_/ .___/_/ /_/___/\____/  
//                 /_/                       
//
// Auxiliary module for Graph I/O utilities
// such as serialization and deserialization
//
// HINTS
// -----
//
//  The specific serialization and deserialization functions are called at for
// each vertex or edge to be serialized to the file pointer provided. It shall
// return 0 in case of success, or anything else on error. Separation characters
// (spaces, tabs and line feeds) in the beggining or in the end will be
// deliberately ignored when reading.
//
//  The module does not take ownership of the file pointer. That means that it
// must be  previously opened in writing or reading mode (depending on the
// function) and closed afterwards by the caller.
//

typedef enum
{
	GRAPH_IO_OK = 0,
	GRAPH_IO_MEMORY,
	GRAPH_IO_WRITING_FAILURE,
	GRAPH_IO_CREATION_FAILURE,
	GRAPH_IO_SAME_CREATION,
	GRAPH_IO_FILE_ERROR,
	GRAPH_IO_DEPRECATED_FILE_FORMAT,
	GRAPH_IO_CORRUPTED_FILE_FORMAT,
}
GraphIoRet;


//  ============= =================================== 
//   graphWrite    Serialize graph structure to file  
//  ============= =================================== 
//   pGraph        pointer to graph                   
//   fp            pointer to file to be written      
//   writeVertex   vertex serialization function      
//   writeEdge     edge serialization function        
//  ============= =================================== 
//  [!] GRAPH_IO_FILE_ERROR: Could not write to file
//  [!] GRAPH_IO_WRITING_FAILURE: Specific serialization failed
//  [!] GRAPH_IO_MEMORY

GraphIoRet graphWrite(Graph *pGraph, FILE *fp,
	int (*writeVertex)(FILE *fp, void *v),
	int (*writeEdge)(FILE *fp, void *e));

//  ============= ======================================= 
//    graphRead    Deserialize graph structure from file  
//  ============= ======================================= 
//   pGraph        pointer to graph                       
//   fp            pointer to file to be read             
//   readVertex    vertex deserialization function        
//   readEdge      edge deserialization function          
//   cmpVertices   vertex comparison function             
//   cmpEdges      edge comparison function               
//   freeVertex    vertex deallocation function           
//   freeEdge      edge deallocation function             
//  ============= ======================================= 
//  [!] GRAPH_IO_FILE_ERROR: Could not read file
//  [!] GRAPH_IO_SAME_CREATION: Vertex deserialized twice
//  [!] GRAPH_IO_CREATION_FAILURE: Specific deserialization failed
//  [!] GRAPH_IO_DEPRECATED_FILE_FORMAT: File format is incompatible
//  [!] GRAPH_IO_CORRUPTED_FILE_FORMAT: File format is corrupted
//  [!] GRAPH_IO_MEMORY

GraphIoRet graphRead(Graph **ppGraph, FILE *fp,
	int (*readVertex)(FILE *fp, void **ppVertex),
	int (*readEdge)(FILE *fp, void **ppEdge),
	int (*cmpVertices)(void *a, void *b),
	void (*freeVertex)(void *v),
	int (*cmpEdges)(void *a, void *b),
	void (*freeEdge)(void *e));

#endif
