#include "graph.h"

#include <stdlib.h>
#include <stdarg.h>

#include <common/assert.h>

#include "set.h"
#include "memdb.h"

/*******************************************************************************
* Graph data structure invariants
********************************************************************************
*
* I) Given (GraphEdge *) e and (GraphVertex *) u
* e->pSource points to u, iff u->outEdges contain e
* e->pDestination points to u, iff u->inEdges contain e
*
* II) If the graph is undirected, an edge always comes from the GraphVertex of
* smallest address to the one with largest address.
*
* III) The ownership of the set cursors are given to specific functions:
* 	- Graph::vertexSet -> graphGet*Vertex
* 	- GraphVertex::outEdges -> graphGet*Neighbour / graphGet*OutNeighbour
* 	- GraphVertex::inEdge -> graphGet*Neighbour / graphGet*InNeighbour
*   Where * stands for either 'Next' or 'Previous'
*
* IV) GraphVertex::inEdgesToIterate must always be in between 0 and the
* number of edges from which the vertex is DESTINATION
*
* V) GraphVertex::outEdgesToIterate must always be in between 0 and the
* number of edges from which the vertex is SOURCE
*
* VI) GraphVertex::outEdgesToIterate must never be lower than
* GraphVertex::inEdgesToIterate.
*
*******************************************************************************/

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
	size_t outEdgesToIterate; /* counter for graphGet*Neighbour */
	size_t inEdgesToIterate; /* counter for graphGet*Neighbour */
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
static void _resetAdjListCounters(struct GraphVertex *pVertex, int orientation);
static void _cycleSetCursor(Set *pSet, void **pValue, int orientation);

GraphReturnID _graphGetNeighbour(Graph *pGraph, void *u, void **pV,
	void **uv, int direction, int orientation);

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

GraphReturnID graphGetNumberOfVertices(Graph *pGraph, size_t *pSize)
{
	size_t temp;
	if (pGraph == NULL || pSize == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	_assert(!setGetSize(pGraph->vertexSet, &temp));
	*pSize = temp;
	return GRAPH_RETURN_OK;
}

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

GraphReturnID graphGetPreviousVertex(Graph *pGraph, void **pV)
{
	SetReturnID setId;
	struct GraphVertex *pVertex = NULL;
	if (pGraph == NULL || pV == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (setId = setGetCurrentItem(pGraph->vertexSet, &pVertex)) {
		_assert(setId == SET_RETURN_EMPTY);
		return GRAPH_RETURN_EMPTY;
	}
	if (setId = setPreviousItem(pGraph->vertexSet)) {
		_assert(setId == SET_RETURN_OUT_OF_BOUNDS);
		_assert(!setLastItem(pGraph->vertexSet));
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
	pVertex->inEdgesToIterate = 0;
	pVertex->outEdgesToIterate = 0;
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
	_resetAdjListCounters(pEdgeUV->pSource, 1);
	_assert(pEdgeUV->pDestination != NULL);
	if (setId = setAddItem(pEdgeUV->pDestination->inEdges, pEdgeUV)) {
		_assert(!setRemoveItem(pEdgeUV->pSource->outEdges, pEdgeUV));
		free(pEdgeUV);
		_assert(setId == SET_RETURN_MEMORY);
		return GRAPH_RETURN_MEMORY;
	}
	_resetAdjListCounters(pEdgeUV->pDestination, 1);
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
	_resetAdjListCounters(pSource, 1);
	_assert(!setRemoveItem(pDestination->inEdges, pEdgeUV));
	_resetAdjListCounters(pDestination, 1);
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
	size_t *pOut)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	size_t temp;
	if (pOut == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, v, &pVertex))
		return graphId;
	_assert(!setGetSize(pVertex->outEdges, &temp));
	*pOut = temp;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetVertexInDegree(Graph *pGraph, void *v,
	size_t *pIn)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	size_t temp;
	if (pIn == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, v, &pVertex))
		return graphId;
	_assert(!setGetSize(pVertex->inEdges, &temp));
	*pIn = temp;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetVertexDegree(Graph *pGraph, void *v,
	size_t *pDegree)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	size_t in, out;
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
	size_t inSize, outSize;
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
		_cycleSetCursor(pVertex->outEdges, &temp, 1);
	} else if (outSize == 0) {
		// thus, inSize > 0
		_cycleSetCursor(pVertex->inEdges, &temp, 1);
	} else {
		// thus, both are > 0
		if (pVertex->inEdgesToIterate == 0 &&
			pVertex->outEdgesToIterate == 0) {
			_resetAdjListCounters(pVertex, 1);
			goto flag;
		} else if (pVertex->inEdgesToIterate == 0) {
			// thus, outEdgesIterated > 0
			_cycleSetCursor(pVertex->outEdges, &temp, 1);
			--pVertex->outEdgesToIterate;
		} else {
		flag:
			// thus, inEdgesToIterate > 0
			_cycleSetCursor(pVertex->inEdges, &temp, 1);
			--pVertex->inEdgesToIterate;
		}
	}
	*pV = temp->pDestination == pVertex ?
		temp->pSource->item : temp->pDestination->item;
	*uv = temp->item;
	return GRAPH_RETURN_OK;
}

GraphReturnID graphGetPreviousNeighbour(Graph *pGraph, void *u, void **pV,
	void **uv)
{
	struct GraphVertex *pVertex;
	GraphReturnID graphId;
	size_t inSize, outSize;
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
		_cycleSetCursor(pVertex->outEdges, &temp, -1);
	} else if (outSize == 0) {
		// thus, inSize > 0
		_cycleSetCursor(pVertex->inEdges, &temp, -1);
	} else {
		// thus, both are > 0
		if (pVertex->inEdgesToIterate == inSize) {
			_resetAdjListCounters(pVertex, -1);
			goto flag;
		} else if (pVertex->outEdgesToIterate == outSize) {
			// thus, inEdgesToIterate < inSize
			_cycleSetCursor(pVertex->inEdges, &temp, -1);
			++pVertex->inEdgesToIterate;
		} else {
		flag:
			// thus, outEdgesToIterate < outSize
			_cycleSetCursor(pVertex->outEdges, &temp, -1);
			++pVertex->outEdgesToIterate;
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
	return _graphGetNeighbour(pGraph, u, pV, uv, 1, 1);
}

GraphReturnID graphGetPreviousInNeighbour(Graph *pGraph, void *u, void **pV,
	void **uv)
{
	return _graphGetNeighbour(pGraph, u, pV, uv, 1, -1);
}

GraphReturnID graphGetNextOutNeighbour(Graph *pGraph, void *u, void **pV,
	void **uv)
{
	return _graphGetNeighbour(pGraph, u, pV, uv, -1, 1);
}

GraphReturnID graphGetPreviousOutNeighbour(Graph *pGraph, void *u, void **pV,
	void **uv)
{
	return _graphGetNeighbour(pGraph, u, pV, uv, -1, -1);
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
// orientation = 1 (next) or -1 (previous)
// If an error occurs, returns 1, else 0.
// [!] Assumes set is not empty!
static void _cycleSetCursor(Set *pSet, void **pValue, int orientation)
{
	SetReturnID setId;
	SetReturnID (*cycleFunc)(Set *);
	SetReturnID (*resetFunc)(Set *);
	void *temp;
	_assert(orientation == 1 || orientation == -1);
	cycleFunc = orientation == 1 ? setNextItem : setPreviousItem;
	resetFunc = orientation == 1 ? setFirstItem : setLastItem;
	_assert(!setGetCurrentItem(pSet, &temp));
	if (setId = cycleFunc(pSet)) {
		_assert(setId == SET_RETURN_OUT_OF_BOUNDS);
		_assert(!resetFunc(pSet));
	}
	*pValue = temp;
}

// Reset set cursors (due to getNeighbour contract)
static void _resetAdjListCounters(struct GraphVertex *pVertex, int orientation)
{
	_assert(orientation == 1 || orientation == -1);
	if (orientation == 1) {
		setFirstItem(pVertex->inEdges);
		setFirstItem(pVertex->outEdges);
		setGetSize(pVertex->inEdges, &pVertex->inEdgesToIterate);
		setGetSize(pVertex->outEdges, &pVertex->outEdgesToIterate);
	} else {
		setLastItem(pVertex->inEdges);
		setLastItem(pVertex->outEdges);
		pVertex->inEdgesToIterate = 0;
		pVertex->outEdgesToIterate = 0;
	}
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

// Generic function for obtaining neighbour of a defined edge set
// direction = 1 for inEdges, -1 for outEdges
// orientation = 1 for Next, -1 for Previous
// The other parameters are the same of graphGet(1)(2)Neighbour,
// where (1) is in {'Next', 'Previous'} and (2) is in {'In', 'Out'}.
GraphReturnID _graphGetNeighbour(Graph *pGraph, void *u, void **pV,
	void **uv, int direction, int orientation)
{
	struct GraphVertex *pVertex;
	Set *pSet;
	GraphReturnID graphId;
	size_t setSize;
	struct GraphEdge *temp;
	if (pV == NULL || uv == NULL)
		return GRAPH_RETURN_INVALID_PARAMETER;
	if (graphId = _parseVertices(pGraph, u, &pVertex))
		return graphId;
	_assert(direction == 1 || direction == -1);
	pSet = direction == 1 ? pVertex->inEdges : pVertex->outEdges;
	_assert(!setGetSize(pSet, &setSize));
	if (setSize == 0)
		return GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE;
	_cycleSetCursor(pSet, &temp, orientation);
	*pV = direction == 1 ? temp->pSource->item : temp->pDestination->item;
	*uv = temp->item;
	return GRAPH_RETURN_OK;
}