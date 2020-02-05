#include "graph.h"

#include <stdlib.h>
#include <stdarg.h>

#include "set.h"

/******************************************************************************
* Graph data structure rules
*******************************************************************************
* 
* > Given (GraphEdge *) e and (GraphVertex *) u
* e->pSource points to u, iff u->outEdges contain e
* e->pDestination points to u, iff u->inEdges contain e
*
* > If the graph is undirected, an edge always comes from the GraphVertex of
* smallest address to the one with largest address.
* 
* > The ownership of the set cursors are given to specific functions:
* 	- Graph::vertexSet -> graphGetNextVertex
* 	- GraphVertex::outEdges -> graphGetNextNeighbour/graphGetNextOutNeighbour
* 	- GraphVertex::inEdge -> graphGetNextNeighbour/graphGetNextInNeighbour
*
******************************************************************************/

struct Graph
{
	int isDirected; /* whether graph is directed or not */
	Set *vertexSet; /* set of struct GraphVertex */
	int (*cmpVertices)(void *a, void *b); /* compares GraphVertex::item */
	void (*freeVertex)(void *v); /* frees GraphVertex::item */
	int (*cmpEdges)(void *a, void *b); /* compares GraphEdge::item */
	void (*freeEdge)(void *e); /* frees GraphEdge::item */
};

struct GraphVertex
{
	void *item; /* generic portion of vertex */
	int flag; /* flag (for dfs, bfs, coloring...) */
	Set *outEdges; /* edges from which the vertex is SOURCE */
	int outEdgesIterated; /* counter for graphGetNext*Neighbour */
	Set *inEdges; /* edges from which the vertex is DESTINATION */
	int inEdgesIterated; /* counter for graphGetNext*Neighbour */
};

struct GraphEdge
{
	void *item; /* generic portion of edge */
	struct GraphVertex *pSource; /* vertex from which the edge comes from */
	struct GraphVertex *pDestination; /* vertex to with the edge goes to */
}; 

  /**********************************/
 /*  Private functions prototypes  */
/**********************************/

///////////////////////////////////////////////
// Parameter for _cmpVertexItem
// item		 - vertex 
// cmpVertices  - comparison function
//				  between vertices
///////////////////////////////////////////////
struct _cmpVertexItemParam
{
	void *item;
	int (*cmpVertices)(void *a, void *b);
};

///////////////////////////////////////////////
// Callbacks
///////////////////////////////////////////////
static int _cmpEdgeDestination(struct GraphEdge *pEdge,
	struct GraphVertex *pDestination);
static int _cmpVertexItem(struct GraphVertex *pVertex,
	struct _cmpVertexItemParam *par);
static void _freeVertex(struct GraphVertex *pVertex, struct Graph *pGraph);
static void _freeInEdge(struct GraphEdge *pEdge, void (*freeEdge)(void *e));
static void _freeOutEdge(struct GraphEdge *pEdge, void (*freeEdge)(void *e));

///////////////////////////////////////////////
// Internal use
///////////////////////////////////////////////
static void _resetAdjListCounters(struct GraphVertex *pVertex);
static int _cycleSetCursor(Set *pSet, void **pValue);

static GraphReturnID _parseEdge(Graph *pGraph, struct GraphVertex *pVertexU,
	struct GraphVertex *pVertexV, struct GraphVertex **ppSource,
	struct GraphVertex **ppDestination, struct GraphEdge **ppEdgeUV);

#define _parseVertices(...) __parseVertices(__VA_ARGS__, NULL)
static GraphReturnID __parseVertices(Graph *pGraph, void *item,
	struct GraphVertex **ppVertex, ...);

  /**********************/
 /*  Public functions  */
/**********************/

GraphReturnID graphCreate(Graph **ppGraph,
	int isDirected,
	int (*cmpVertices)(void *a, void *b),
	void (*freeVertex)(void *v),
	int (*cmpEdges)(void *a, void *b),
	void (*freeEdge)(void *e))
{
	struct Graph *pGraph;
	if (ppGraph == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	pGraph = malloc(sizeof(struct Graph));
	if (pGraph == NULL)
		return GRAPH_RETURN_MEMORY;
	if (setCreate(&pGraph->vertexSet)) {
		free(pGraph);
		return GRAPH_RETURN_MEMORY;
	}
	pGraph->isDirected = isDirected;
	pGraph->freeVertex = freeVertex;
	pGraph->cmpVertices = cmpVertices;
	pGraph->freeEdge = freeEdge;
	pGraph->cmpEdges = cmpEdges;
	*ppGraph = pGraph;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetNumberOfVertices(Graph *pGraph, unsigned long *pSize)
{
	unsigned long temp;
	if (pGraph == NULL || pSize == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (setGetSize(pGraph->vertexSet, &temp))
		return GRAPH_RETURN_UNKNOWN_ERROR;
	*pSize = temp;
	return GRAPH_RETURN_OK;
}

// Has the monopoly over the vertexSet cursor!!
// It cannot be manipulated by any other function,
// because the user must be in control of which vertex
// he thinks the cursor is pointing to!
// Other internal functions can use the cursor too but
// must not alter its state after the call (or can
// alter it coherently)
GraphReturnID graphGetNextVertex(Graph *pGraph, void **pV)
{
	SetReturnID setId;
	struct GraphVertex *pVertex;
	if (pGraph == NULL || pV == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (setId = setGetCurrentItem(pGraph->vertexSet, &pVertex)) {
		if (setId == SET_RETURN_EMPTY)
			return GRAPH_RETURN_EMPTY;
		return GRAPH_RETURN_UNKNOWN_ERROR;
	}
	if (setId = setNextItem(pGraph->vertexSet)) {
		if (setId != SET_RETURN_OUT_OF_BOUNDS)
			return GRAPH_RETURN_UNKNOWN_ERROR;
		setFirstItem(pGraph->vertexSet);
	}
	*pV = pVertex->item;
	return GRAPH_RETURN_OK;

}

GraphReturnID graphIsDirected(Graph *pGraph, int *pIsDirected)
{
	if (pGraph == NULL || pIsDirected == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	*pIsDirected = pGraph->isDirected;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphContainsVertex(Graph *pGraph, void *v)
{
	void *p; // does nothing with p
	struct _cmpVertexItemParam par = {v, pGraph->cmpVertices};
	if (pGraph == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	switch (setFilterItem(pGraph->vertexSet, _cmpVertexItem, &par, &p)) {
	case SET_RETURN_OK:
		return GRAPH_RETURN_CONTAINS_VERTEX;
	case SET_RETURN_DOES_NOT_CONTAIN:
		return GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX;
	default:
		return GRAPH_RETURN_UNKNOWN_ERROR;
	}
}

GraphReturnID graphAddVertex(Graph *pGraph, void *v)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	SetReturnID setId;
	if (pGraph == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	graphId = graphContainsVertex(pGraph, v);
	if (graphId != GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX)
		return graphId;
	pVertex = malloc(sizeof(struct GraphVertex));
	if (pVertex == NULL)
		return GRAPH_RETURN_MEMORY;
	pVertex->item = v;
	pVertex->flag = 0;
	pVertex->inEdgesIterated = 0;
	pVertex->outEdgesIterated = 0;
	if (setCreate(&pVertex->inEdges)) {
		free(pVertex);
		return GRAPH_RETURN_MEMORY;
	}
	if (setCreate(&pVertex->outEdges)) {
		setDestroy(pVertex->inEdges);
		free(pVertex);
		return GRAPH_RETURN_MEMORY;
	}
	if (setId = setAddItem(pGraph->vertexSet, pVertex)) {
		setDestroy(pVertex->inEdges);
		setDestroy(pVertex->outEdges);
		free(pVertex);
		switch (setId) {
		case SET_RETURN_MEMORY:
			return GRAPH_RETURN_MEMORY;
		// can't be SET_RETURN_CONTAINS because it has
		// been already checked with graphContainsVertex
		default:
			return GRAPH_RETURN_UNKNOWN_ERROR;
		}
	}
	return GRAPH_RETURN_OK;
}

GraphReturnID graphRemoveVertex(Graph *pGraph, void *v)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	if (pGraph == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, v, &pVertex))
		return graphId;
	if (setRemoveItem(pGraph->vertexSet, pVertex))
		return GRAPH_RETURN_UNKNOWN_ERROR;
	_freeVertex(pVertex, pGraph);
	return GRAPH_RETURN_OK;
}

GraphReturnID graphAddEdge(Graph *pGraph, void *u, void *v, void *uv)
{
	struct GraphVertex *pVertexU, *pVertexV;
	struct GraphEdge *pEdgeUV;
	GraphReturnID graphId;
	SetReturnID setId;
	if (pGraph == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, u, &pVertexU, v, &pVertexV))
		return graphId;
	if ((graphId = graphContainsEdge(pGraph, u, v))
		!= GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE)
		return graphId;
	pEdgeUV = malloc(sizeof(struct GraphEdge));
	if (pEdgeUV == NULL)
		return GRAPH_RETURN_MEMORY;
	pEdgeUV->item = uv;
	if (pGraph->isDirected) {
		pEdgeUV->pSource = pVertexU;
		pEdgeUV->pDestination = pVertexV;
	} else {
		pEdgeUV->pSource = pVertexU < pVertexV ? pVertexU : pVertexV;
		pEdgeUV->pDestination = pVertexU < pVertexV ? pVertexV : pVertexU;
	}
	if (setId = setAddItem(pEdgeUV->pSource->outEdges, pEdgeUV)) {
		free(pEdgeUV);
		if (setId == SET_RETURN_MEMORY)
			return GRAPH_RETURN_MEMORY;
		return GRAPH_RETURN_UNKNOWN_ERROR;
	}
	_resetAdjListCounters(pEdgeUV->pSource);
	if (setId = setAddItem(pEdgeUV->pDestination->inEdges, pEdgeUV)) {
		if (setId == SET_RETURN_MEMORY) {
			setId = setRemoveItem(pEdgeUV->pDestination->inEdges, pEdgeUV);
			free(pEdgeUV);
			if (setId)
				return GRAPH_RETURN_FATAL_ERROR;
			return GRAPH_RETURN_MEMORY;
		}
		free(pEdgeUV);
		return GRAPH_RETURN_UNKNOWN_ERROR;
	}
	_resetAdjListCounters(pEdgeUV->pDestination);
	return GRAPH_RETURN_OK;
}

GraphReturnID graphContainsEdge(Graph *pGraph, void *u, void *v)
{
	struct GraphVertex *pVertexU, *pVertexV;
	GraphReturnID graphId;
	if (pGraph == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, u, &pVertexU, v, &pVertexV))
		return graphId;
	return _parseEdge(pGraph, pVertexU, pVertexV, NULL, NULL, NULL);
}

GraphReturnID graphRemoveEdge(Graph *pGraph, void *u, void *v)
{
	struct GraphVertex *pVertexU, *pVertexV, *pSource, *pDestination;
	struct GraphEdge *pEdgeUV;
	GraphReturnID graphId;
	if (pGraph == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, u, &pVertexU, v, &pVertexV))
		return graphId;
	graphId = _parseEdge(pGraph, pVertexU, pVertexV, &pSource, &pDestination,
		&pEdgeUV);
	if (graphId != GRAPH_RETURN_CONTAINS_EDGE)
		return graphId;
	if (setRemoveItem(pSource->outEdges, pEdgeUV))
		return GRAPH_RETURN_UNKNOWN_ERROR;
	_resetAdjListCounters(pSource);
	if (setRemoveItem(pDestination->inEdges, pEdgeUV))
		return GRAPH_RETURN_UNKNOWN_ERROR;
	_resetAdjListCounters(pDestination);
	if (pGraph->freeEdge)
		pGraph->freeEdge(pEdgeUV->item);
	free(pEdgeUV);
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetEdge(Graph *pGraph, void *u, void *v, void **uv)
{
	struct GraphVertex *pVertexU, *pVertexV;
	struct GraphEdge *temp;
	GraphReturnID graphId;
	if (pGraph == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, u, &pVertexU, v, &pVertexV))
		return graphId;
	graphId = _parseEdge(pGraph, pVertexU, pVertexV, NULL, NULL, &temp);
	if (graphId != GRAPH_RETURN_CONTAINS_EDGE)
		return graphId;
	*uv = temp->item;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetVertexOutDegree(Graph *pGraph, void *v,
	unsigned long *pOut)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	unsigned long temp;
	if (pGraph == NULL || pOut == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, v, &pVertex))
		return graphId;
	if (setGetSize(pVertex->outEdges, &temp))
		return GRAPH_RETURN_UNKNOWN_ERROR;
	*pOut = temp;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetVertexInDegree(Graph *pGraph, void *v,
	unsigned long *pIn)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	unsigned long temp;
	if (pGraph == NULL || pIn == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, v, &pVertex))
		return graphId;
	if (setGetSize(pVertex->inEdges, &temp))
		return GRAPH_RETURN_UNKNOWN_ERROR;
	*pIn = temp;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetVertexDegree(Graph *pGraph, void *v,
	unsigned long *pDegree)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	unsigned long in, out;
	if (pGraph == NULL || pDegree == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, v, &pVertex))
		return graphId;
	if (setGetSize(pVertex->inEdges, &in))
		return GRAPH_RETURN_UNKNOWN_ERROR;
	if (setGetSize(pVertex->outEdges, &out))
		return GRAPH_RETURN_UNKNOWN_ERROR;
	*pDegree = in + out;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetNextNeighbour(Graph *pGraph, void *u, void **pV,
	void **uv)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	unsigned long inSize, outSize;
	struct GraphEdge *temp;
	if (pGraph == NULL || pV == NULL || uv == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, u, &pVertex))
		return graphId;
	if (setGetSize(pVertex->inEdges, &inSize))
		return GRAPH_RETURN_UNKNOWN_ERROR;
	if (setGetSize(pVertex->outEdges, &outSize))
		return GRAPH_RETURN_UNKNOWN_ERROR;
	if (inSize == 0 && outSize == 0)
		return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
	if (inSize == 0) {
		// thus, outSize > 0
		if (_cycleSetCursor(pVertex->outEdges, &temp))
			return GRAPH_RETURN_UNKNOWN_ERROR;
		--pVertex->outEdgesIterated;
	} else if (outSize == 0) {
		// thus, inSize > 0
		if (_cycleSetCursor(pVertex->inEdges, &temp))
			return GRAPH_RETURN_UNKNOWN_ERROR;
		--pVertex->inEdgesIterated;
	} else {
		// thus, both are > 0
		if (pVertex->inEdgesIterated == 0 &&
			pVertex->outEdgesIterated == 0) {
			_resetAdjListCounters(pVertex);
			if (_cycleSetCursor(pVertex->outEdges, &temp))
				return GRAPH_RETURN_UNKNOWN_ERROR;
			--pVertex->outEdgesIterated;
		} else if (pVertex->inEdgesIterated == 0) {
			// thus, outEdgesIterated > 0
			if (_cycleSetCursor(pVertex->outEdges, &temp))
				return GRAPH_RETURN_UNKNOWN_ERROR;
			--pVertex->outEdgesIterated;
		} else if (pVertex->outEdgesIterated == 0) {
			// thus, inEdgesIterated > 0
			if (_cycleSetCursor(pVertex->inEdges, &temp))
				return GRAPH_RETURN_UNKNOWN_ERROR;
			--pVertex->inEdgesIterated;
		} else {
			// thus, both are > 0
			if (pVertex->inEdgesIterated >
				pVertex->outEdgesIterated) {
				// more edges out than in
				if (_cycleSetCursor(pVertex->outEdges, &temp))
					return GRAPH_RETURN_UNKNOWN_ERROR;
				--pVertex->outEdgesIterated;
			} else {
				// more (or same # of) edges in than out
				if (_cycleSetCursor(pVertex->inEdges, &temp))
					return GRAPH_RETURN_UNKNOWN_ERROR;
				--pVertex->inEdgesIterated;
			}
		}
	} 
	*pV = temp->pDestination == pVertex ?
		temp->pSource->item : temp->pDestination->item;
	*uv = temp->item;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetNextInNeighbour(Graph *pGraph, void *u, void **pV,
	void **uv)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	unsigned long inSize;
	struct GraphEdge *temp;
	if (pGraph == NULL || pV == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, u, &pVertex))
		return graphId;
	if (setGetSize(pVertex->inEdges, &inSize))
		return GRAPH_RETURN_UNKNOWN_ERROR;
	if (inSize == 0)
		return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
	if (_cycleSetCursor(pVertex->inEdges, &temp))
		return GRAPH_RETURN_UNKNOWN_ERROR;
	*pV = temp->pSource->item;
	*uv = temp->item;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetNextOutNeighbour(Graph *pGraph, void *u, void **pV,
	void **uv)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	unsigned long outSize;
	struct GraphEdge *temp;
	if (pGraph == NULL || pV == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, u, &pVertex))
		return graphId;
	if (setGetSize(pVertex->outEdges, &outSize))
		return GRAPH_RETURN_UNKNOWN_ERROR;
	if (outSize == 0)
		return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
	if (_cycleSetCursor(pVertex->outEdges, &temp))
		return GRAPH_RETURN_UNKNOWN_ERROR;
	*pV = temp->pDestination->item;
	*uv = temp->item;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetVertexFlag(Graph *pGraph, void *v, int *pFlag)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	if (pGraph == NULL || pFlag == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, v, &pVertex))
		return graphId;
	*pFlag = pVertex->flag;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphSetVertexFlag(Graph *pGraph, void *v, int flag)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	if (pGraph == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, v, &pVertex))
		return graphId;
	pVertex->flag = flag;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetVertexComparisonFunc(Graph *pGraph,
	int (**pCmpVertices)(void *a, void *b))
{
	if (pGraph == NULL || pCmpVertices == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	*pCmpVertices = pGraph->cmpVertices;
	return GRAPH_RETURN_OK;
}

void graphDestroy(Graph *pGraph)
{
	if (pGraph == NULL)
		return;
	setDestroyDeep(pGraph->vertexSet, _freeVertex, pGraph);
	free(pGraph);
}

  /**************************************/
 /*  Private functions implementation  */
/**************************************/

// Cycle set cursor (when reaches end, go to first)
// If an error occurs, returns 1, else 0.
// [!] Assumes set is not empty!
static int _cycleSetCursor(Set *pSet, void **pValue)
{
	SetReturnID setId;
	void *temp;
	if (setGetCurrentItem(pSet, &temp))
		return 1;
	if (setId = setNextItem(pSet)) {
		if (setId != SET_RETURN_OUT_OF_BOUNDS)
			return 1;
		if (setFirstItem(pSet))
			return 1;
	}
	*pValue = temp;
	return 0;
}

// Reset set cursors (due to getNeighbour contract)
static void _resetAdjListCounters(struct GraphVertex *pVertex)
{
	setFirstItem(pVertex->inEdges);
	setFirstItem(pVertex->outEdges);
	setGetSize(pVertex->inEdges, &pVertex->inEdgesIterated);
	setGetSize(pVertex->outEdges, &pVertex->outEdgesIterated);
}

// Compares destination from edge and the one provided
static int _cmpEdgeDestination(struct GraphEdge *pEdge,
	struct GraphVertex *pDestination)
{
	return pEdge->pDestination == pDestination;
}

// Compares item from graph vertex struct and item
static int _cmpVertexItem(struct GraphVertex *pVertex,
	struct _cmpVertexItemParam *par)
{
	if (pVertex->item == par->item)
		return 1;
	return par->cmpVertices && par->cmpVertices(pVertex->item, par->item);
}

// Called by setlib while removing pVertex from pGraph->vertexSet
static void _freeVertex(struct GraphVertex *pVertex, struct Graph *pGraph)
{
	if (pGraph->freeVertex)
		pGraph->freeVertex(pVertex->item);
	setDestroyDeep(pVertex->inEdges, _freeInEdge, pGraph->freeEdge);
	setDestroyDeep(pVertex->outEdges, _freeOutEdge, pGraph->freeEdge);
	free(pVertex);
}

// Called by setlib while removing pEdge from pEdge->pDestination->inEdges
static void _freeInEdge(struct GraphEdge *pEdge, void (*freeEdge)(void *e))
{
	setRemoveItem(pEdge->pSource->outEdges, pEdge);
	if (freeEdge)
		freeEdge(pEdge->item);
	free(pEdge);
}

// Called by setlib while removing pEdge from pEdge->pSource->outEdges
static void _freeOutEdge(struct GraphEdge *pEdge, void (*freeEdge)(void *e))
{
	setRemoveItem(pEdge->pDestination->inEdges, pEdge);
	if (freeEdge)
		freeEdge(pEdge->item);
	free(pEdge);
}

// Retrieves the edge uv through the vertices u and v
// ppSource, ppDestination and ppEdgeUV are optional and
// NULL can be parsed without a problem.
// Possible return values:
// GRAPH_RETURN_CONTAINS_EDGE + edge, source and destination by reference
// GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE
// GRAPH_RETURN_UNKNOWN_ERROR
static GraphReturnID _parseEdge(Graph *pGraph, struct GraphVertex *pVertexU,
	struct GraphVertex *pVertexV, struct GraphVertex **ppSource,
	struct GraphVertex **ppDestination, struct GraphEdge **ppEdgeUV)
{
	struct GraphVertex *pSource, *pDestination;
	struct GraphEdge *pEdgeUV;
	SetReturnID setId;
	pSource = (pGraph->isDirected || pVertexU < pVertexV) ?
		pVertexU : pVertexV;
	pDestination = pVertexU == pSource ? pVertexV : pVertexU;
	setId = setFilterItem(pSource->outEdges, _cmpEdgeDestination, pDestination,
		&pEdgeUV);
	switch (setId) {
	case SET_RETURN_OK:
		if (ppSource) *ppSource = pSource;
		if (ppDestination) *ppDestination = pDestination;
		if (ppEdgeUV) *ppEdgeUV = pEdgeUV;
		return GRAPH_RETURN_CONTAINS_EDGE;
	case SET_RETURN_DOES_NOT_CONTAIN:
		return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
	default:
		return GRAPH_RETURN_UNKNOWN_ERROR;
	}
}

// Retrieves the vertices through the items contained in them.
// The item and ppVertex arguments must be alternated, and end with NULL, eg:
// __parseVertices(pGrpah, u, &pVertexU, v, &pVertexV, w, &pVertexW, NULL);
// The macro with only one '_' already does the above requirement.
// Does not alter the vertexSet cursor pointer, only the current pointer,
// which is of internal use, and doesn't interfeer with NextVertex iteration.
// Possible errors:
// GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
// GRAPH_RETURN_UNKNOWN_ERROR
static GraphReturnID __parseVertices(Graph *pGraph, void *item,
	struct GraphVertex **ppVertex, ...)
{
	SetReturnID setId;
	va_list va;
	struct _cmpVertexItemParam param = {item, pGraph->cmpVertices};
	va_start(va, ppVertex);
	do {
		if (setId = setFilterItem(pGraph->vertexSet, _cmpVertexItem, &param,
			ppVertex)) {
			va_end(va);
			if (setId == SET_RETURN_DOES_NOT_CONTAIN)
				return GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX;
			return GRAPH_RETURN_UNKNOWN_ERROR;
		}
		param.item = va_arg(va, void *);
		if (param.item == NULL)
			break; /* Found sentinel */
		ppVertex = va_arg(va, struct GraphVertex **);
	} while(1);
	va_end(va);
	return GRAPH_RETURN_OK;
}