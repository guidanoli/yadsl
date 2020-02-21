#include "graph.h"

#include <stdlib.h>
#include <stdarg.h>

#include <common/assert.h>

#include "set.h"

/******************************************************************************
* Graph data structure invariants
*******************************************************************************
*
* I) Given (GraphEdge *) e and (GraphVertex *) u
* e->pSource points to u, iff u->outEdges contain e
* e->pDestination points to u, iff u->inEdges contain e
*
* II) If the graph is undirected, an edge always comes from the GraphVertex of
* smallest address to the one with largest address.
*
* III) The ownership of the set cursors are given to specific functions:
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
	int (*cmpEdges)(void *a, void *b); /* compares GraphEdge::item */
	void (*freeVertex)(void *v); /* frees GraphVertex::item */
	void (*freeEdge)(void *e); /* frees GraphEdge::item */
};

struct GraphVertex
{
	void *item; /* generic portion of vertex */
	int flag; /* flag (for dfs, bfs, coloring...) */
	Set *outEdges; /* edges from which the vertex is SOURCE */
	Set *inEdges; /* edges from which the vertex is DESTINATION */
	unsigned long outEdgesIterated; /* counter for graphGetNext*Neighbour */
	unsigned long inEdgesIterated; /* counter for graphGetNext*Neighbour */
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
// item         - vertex 
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
static int _setVertexFlag(struct GraphVertex *pVertex, int *flag);

///////////////////////////////////////////////
// Internal use
///////////////////////////////////////////////
static void _resetAdjListCounters(struct GraphVertex *pVertex);
static void _cycleSetCursor(Set *pSet, void **pValue);

static GraphReturnID _parseEdge(Graph *pGraph, struct GraphVertex *pVertexU,
	struct GraphVertex *pVertexV, struct GraphVertex **ppSource,
	struct GraphVertex **ppDestination, struct GraphEdge **ppEdgeUV);

#define _parseVertices(...) __parseVertices(__VA_ARGS__, NULL, NULL)
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
	pGraph->freeEdge = freeEdge;
	pGraph->cmpVertices = cmpVertices;
	pGraph->cmpEdges = cmpEdges;
	*ppGraph = pGraph;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetNumberOfVertices(Graph *pGraph, unsigned long *pSize)
{
	unsigned long temp;
	if (pGraph == NULL || pSize == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	_assert(!setGetSize(pGraph->vertexSet, &temp));
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
	struct GraphVertex *pVertex = NULL;
	if (pGraph == NULL || pV == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (setId = setGetCurrentItem(pGraph->vertexSet, &pVertex)) {
		_assert(setId == SET_RETURN_EMPTY);
		return GRAPH_RETURN_EMPTY;
	}
	if (setId = setNextItem(pGraph->vertexSet)) {
		_assert(setId == SET_RETURN_OUT_OF_BOUNDS);
		_assert(!setFirstItem(pGraph->vertexSet));
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

GraphReturnID graphContainsVertex(Graph *pGraph, void *v, int *pContains)
{
	void *p; // does nothing with p
	struct _cmpVertexItemParam par = { v, pGraph->cmpVertices };
	if (pGraph == NULL || pContains == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	switch (setFilterItem(pGraph->vertexSet, _cmpVertexItem, &par, &p)) {
	case SET_RETURN_OK:
		*pContains = 1;
		break;
	case SET_RETURN_DOES_NOT_CONTAIN:
		*pContains = 0;
		break;
	default:
		_assert(0);
	}
	return GRAPH_RETURN_OK;
}

GraphReturnID graphAddVertex(Graph *pGraph, void *v)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	SetReturnID setId;
	int containsVertex;
	if (graphId = graphContainsVertex(pGraph, v, &containsVertex))
		return graphId;
	if (containsVertex)
		return GRAPH_RETURN_CONTAINS_VERTEX;
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
		_assert(setId == SET_RETURN_MEMORY);
		return GRAPH_RETURN_MEMORY;
	}
	return GRAPH_RETURN_OK;
}

GraphReturnID graphRemoveVertex(Graph *pGraph, void *v)
{
	struct GraphVertex *pVertex = NULL;
	GraphReturnID graphId;
	if (graphId = _parseVertices(pGraph, v, &pVertex))
		return graphId;
	_assert(!setRemoveItem(pGraph->vertexSet, pVertex));
	_freeVertex(pVertex, pGraph);
	return GRAPH_RETURN_OK;
}

GraphReturnID graphAddEdge(Graph *pGraph, void *u, void *v, void *uv)
{
	struct GraphVertex *pVertexU = NULL, *pVertexV = NULL;
	struct GraphEdge *pEdgeUV = NULL;
	GraphReturnID graphId;
	SetReturnID setId;
	int containsEdge;
	if (graphId = _parseVertices(pGraph, u, &pVertexU, v, &pVertexV))
		return graphId;
	_assert(!graphContainsEdge(pGraph, u, v, &containsEdge));
	if (containsEdge)
		return GRAPH_RETURN_CONTAINS_EDGE;
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
	_assert(pEdgeUV->pSource != NULL);
	if (setId = setAddItem(pEdgeUV->pSource->outEdges, pEdgeUV)) {
		free(pEdgeUV);
		_assert(setId == SET_RETURN_MEMORY);
		return GRAPH_RETURN_MEMORY;
	}
	_resetAdjListCounters(pEdgeUV->pSource);
	_assert(pEdgeUV->pDestination != NULL);
	if (setId = setAddItem(pEdgeUV->pDestination->inEdges, pEdgeUV)) {
		_assert(!setRemoveItem(pEdgeUV->pSource->outEdges, pEdgeUV));
		free(pEdgeUV);
		_assert(setId == SET_RETURN_MEMORY);
		return GRAPH_RETURN_MEMORY;
	}
	_resetAdjListCounters(pEdgeUV->pDestination);
	return GRAPH_RETURN_OK;
}

GraphReturnID graphContainsEdge(Graph *pGraph, void *u, void *v, int *pContains)
{
	struct GraphVertex *pVertexU = NULL, *pVertexV = NULL;
	GraphReturnID graphId;
	if (pContains == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, u, &pVertexU, v, &pVertexV))
		return graphId;
	switch (_parseEdge(pGraph, pVertexU, pVertexV, NULL, NULL, NULL)) {
	case GRAPH_RETURN_OK:
		*pContains = 1;
		break;
	case GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE:
		*pContains = 0;
		break;
	default:
		_assert(0);
	}
	return GRAPH_RETURN_OK;
}

GraphReturnID graphRemoveEdge(Graph *pGraph, void *u, void *v)
{
	struct GraphVertex *pVertexU = NULL, *pVertexV = NULL,
		*pSource = NULL, *pDestination = NULL;
	struct GraphEdge *pEdgeUV = NULL;
	GraphReturnID graphId;
	if (graphId = _parseVertices(pGraph, u, &pVertexU, v, &pVertexV))
		return graphId;
	if (graphId = _parseEdge(pGraph, pVertexU, pVertexV, &pSource,
		&pDestination, &pEdgeUV))
		return graphId;
	_assert(!setRemoveItem(pSource->outEdges, pEdgeUV));
	_resetAdjListCounters(pSource);
	_assert(!setRemoveItem(pDestination->inEdges, pEdgeUV));
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
	if (graphId = _parseVertices(pGraph, u, &pVertexU, v, &pVertexV))
		return graphId;
	if (graphId = _parseEdge(pGraph, pVertexU, pVertexV, NULL, NULL, &temp)) {
		_assert(graphId != GRAPH_RETURN_INVALID_PARAMETER);
		return graphId;
	}
	*uv = temp->item;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetVertexOutDegree(Graph *pGraph, void *v,
	unsigned long *pOut)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	unsigned long temp;
	if (pOut == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, v, &pVertex))
		return graphId;
	_assert(!setGetSize(pVertex->outEdges, &temp));
	*pOut = temp;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetVertexInDegree(Graph *pGraph, void *v,
	unsigned long *pIn)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	unsigned long temp;
	if (pIn == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, v, &pVertex))
		return graphId;
	_assert(!setGetSize(pVertex->inEdges, &temp));
	*pIn = temp;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetVertexDegree(Graph *pGraph, void *v,
	unsigned long *pDegree)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	unsigned long in, out;
	if (pDegree == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, v, &pVertex))
		return graphId;
	_assert(!setGetSize(pVertex->inEdges, &in));
	_assert(!setGetSize(pVertex->outEdges, &out));
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
	_assert(!setGetSize(pVertex->inEdges, &inSize));
	_assert(!setGetSize(pVertex->outEdges, &outSize));
	if (inSize == 0 && outSize == 0)
		return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
	if (inSize == 0) {
		// thus, outSize > 0
		_cycleSetCursor(pVertex->outEdges, &temp);
		--pVertex->outEdgesIterated;
	} else if (outSize == 0) {
		// thus, inSize > 0
		_cycleSetCursor(pVertex->inEdges, &temp);
		--pVertex->inEdgesIterated;
	} else {
		// thus, both are > 0
		if (pVertex->inEdgesIterated == 0 &&
			pVertex->outEdgesIterated == 0) {
			_resetAdjListCounters(pVertex);
			_cycleSetCursor(pVertex->outEdges, &temp);
			--pVertex->outEdgesIterated;
		} else if (pVertex->inEdgesIterated == 0) {
			// thus, outEdgesIterated > 0
			_cycleSetCursor(pVertex->outEdges, &temp);
			--pVertex->outEdgesIterated;
		} else if (pVertex->outEdgesIterated == 0) {
			// thus, inEdgesIterated > 0
			_cycleSetCursor(pVertex->inEdges, &temp);
			--pVertex->inEdgesIterated;
		} else {
			// thus, both are > 0
			if (pVertex->inEdgesIterated >
				pVertex->outEdgesIterated) {
				// more edges out than in
				_cycleSetCursor(pVertex->outEdges, &temp);
				--pVertex->outEdgesIterated;
			} else {
				// more (or same # of) edges in than out
				_cycleSetCursor(pVertex->inEdges, &temp);
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
	if (pV == NULL || uv == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, u, &pVertex))
		return graphId;
	_assert(!setGetSize(pVertex->inEdges, &inSize));
	if (inSize == 0)
		return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
	_cycleSetCursor(pVertex->inEdges, &temp);
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
	if (pV == NULL || uv == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, u, &pVertex))
		return graphId;
	_assert(!setGetSize(pVertex->outEdges, &outSize));
	if (outSize == 0)
		return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
	_cycleSetCursor(pVertex->outEdges, &temp);
	*pV = temp->pDestination->item;
	*uv = temp->item;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetVertexFlag(Graph *pGraph, void *v, int *pFlag)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	if (pFlag == NULL)
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
	if (graphId = _parseVertices(pGraph, v, &pVertex))
		return graphId;
	pVertex->flag = flag;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphSetAllVerticesFlags(Graph *pGraph, int flag)
{
	void *temp;
	if (pGraph == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	_assert(setFilterItem(pGraph->vertexSet, _setVertexFlag, &flag, &temp)
		== SET_RETURN_DOES_NOT_CONTAIN); // _setVertexFlag only returns 0
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
static void _cycleSetCursor(Set *pSet, void **pValue)
{
	SetReturnID setId;
	void *temp;
	_assert(!setGetCurrentItem(pSet, &temp));
	if (setId = setNextItem(pSet)) {
		_assert(setId == SET_RETURN_OUT_OF_BOUNDS);
		_assert(!setFirstItem(pSet));
	}
	*pValue = temp;
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
	if (par->cmpVertices)
		return par->cmpVertices(par->item, pVertex->item);
	return pVertex->item == par->item;
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
// Possible errors:
// GRAPH_RETURN_INVALID_PARAMETER
//	- "pGraph" is NULL
//	- "pVertexU" is NULL
//	- "pVertexV" is NULL
// GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE
static GraphReturnID _parseEdge(Graph *pGraph, struct GraphVertex *pVertexU,
	struct GraphVertex *pVertexV, struct GraphVertex **ppSource,
	struct GraphVertex **ppDestination, struct GraphEdge **ppEdgeUV)
{
	struct GraphVertex *pSource, *pDestination;
	struct GraphEdge *pEdgeUV;
	SetReturnID setId;
	if (pGraph == NULL || pVertexU == NULL || pVertexV == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	pSource = (pGraph->isDirected || pVertexU < pVertexV) ?
		pVertexU : pVertexV;
	pDestination = pVertexU == pSource ? pVertexV : pVertexU;
	setId = setFilterItem(
		pSource->outEdges,   /* pSet */
		_cmpEdgeDestination, /* func */
		pDestination,        /* arg */
		&pEdgeUV);           /* pItem */
	switch (setId) {
	case SET_RETURN_OK:
		if (ppSource) *ppSource = pSource;
		if (ppDestination) *ppDestination = pDestination;
		if (ppEdgeUV) *ppEdgeUV = pEdgeUV;
		break;
	case SET_RETURN_DOES_NOT_CONTAIN:
		return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
	default:
		_assert(0);
	}
	return GRAPH_RETURN_OK;;
}

// Retrieves the vertices through the items contained in them.
// The item and ppVertex arguments must be alternated, and end with two NULLs:
// __parseVertices(pGrpah, u, &pU, v, &pV, w, &pW, NULL, NULL);
// The macro with only one '_' already does the above requirement.
// Does not alter the vertexSet cursor pointer, only the current pointer,
// which is of internal use, and doesn't interfeer with NextVertex iteration.
// Possible errors:
// GRAPH_RETURN_INVALID_PARAMETER
//  - "pGraph" is NULL
//  - "ppVertex" is NULL
// GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX
static GraphReturnID __parseVertices(Graph *pGraph, void *item,
	struct GraphVertex **ppVertex, ...)
{
	SetReturnID setId;
	va_list va;
	struct _cmpVertexItemParam param;
	if (pGraph == NULL || ppVertex == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	param.item = item;
	param.cmpVertices = pGraph->cmpVertices;
	va_start(va, ppVertex);
	do {
		if (setId = setFilterItem(
			pGraph->vertexSet, /* pSet */
			_cmpVertexItem,    /* func */
			&param,            /* arg */
			ppVertex)) {       /* pItem */
			va_end(va);
			_assert(setId == SET_RETURN_DOES_NOT_CONTAIN);
			return GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX;
		}
		param.item = va_arg(va, void *);
		if (!(ppVertex = va_arg(va, struct GraphVertex **)))
			break; /* sentinel */
	} while (1);
	va_end(va);
	return GRAPH_RETURN_OK;
}

// set vertex flag to *flag
static int _setVertexFlag(struct GraphVertex *pVertex, int *flag)
{
	pVertex->flag = *flag;
	return 0;
}
