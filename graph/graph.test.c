#include <string.h>

#include "graph.h"
#include "graphio.h"
#include "graphsearch.h"
#include "tester.h"
#include "var.h"

/* Help */

const char *TesterHelpStrings[] = {
	"This is an interactive module of the graph library",
	"You will interact with the same graph object at all times",
	"A directed graph is already created from the start",
	"The registered actions are the following:",
	"",
	"Graph commands:",
	"/create [DIRECTED/UNDIRECTED]          create new graph",
	"/isdirected [YES/NO]                   check if graph is directed",
	"/vertexcount <expected>                get graph vertex count",
	"/nextvertex <expected vertex>          get next vertex in graph",
	"/outdegree <vertex> <expected>         get vertex out degree",
	"/indegree <vertex> <expected>          get vertex in degree",
	"/degree <vertex> <expected>            get vertex (total) degree",
	"/nextinneighbour <vertex> <nb> <edge>  get next vertex in-neighbour",
	"/nextoutneighbour <vertex> <nb> <edge> get next vertex out-neighbour",
	"/nextneighbour <vertex> <nb> <edge>    get next vertex neighbour",
	"/containsvertex <vertex> [YES/NO]      check if graph contains vertex",
	"/addvertex <vertex>                    add vertex to graph",
	"/removevertex <vertex>                 remove vertex from graph",
	"/containsedge <u> <v> [YES/NO]         check if graph contains edge",
	"/addedge <u> <v> <edge>                add edge to graph",
	"/getedge <u> <v> <edge>                get edge from graph",
	"/removeedege <u> <v>",
	"",
	"Graph IO commands:",
	"/write <filename>                      write graph to file",
	"/read <filename>                       read from file to graph",
	NULL,
};

/* Graph */

static Graph *pGraph = NULL;
static char buffer[BUFSIZ], buffer2[BUFSIZ], buffer3[BUFSIZ];

static void _freeVariables(void *v);
static int _cmpVariables(void *a, void *b);
static int _readVariables(FILE *fp, void **ppVertex);
static int _writeVariables(FILE *fp, void *v);
static TesterReturnValue convertRet(GraphReturnID graphId);
static TesterReturnValue convertIoRet(GraphIoReturnID graphIoId);
static TesterReturnValue convertSearchRet(GraphSearchReturnID graphSearchId);

TesterReturnValue TesterInitCallback()
{
	GraphReturnID graphId;
	if (graphId = graphCreate(&pGraph, 1, _cmpVariables, _freeVariables,
		_cmpVariables, _freeVariables))
		return convertRet(graphId);
	return TESTER_RETURN_OK;
}

#define matches(a, b) (strcmp(a, b) == 0)

static TesterReturnValue parseGraphCommands(const char *command)
{
	GraphReturnID graphId = GRAPH_RETURN_OK;
	if matches(command, "create") {
		Graph *temp;
		int isDirected;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_RETURN_ARGUMENT;
		isDirected = matches(buffer, "DIRECTED");
		graphId = graphCreate(&temp, isDirected, _cmpVariables, _freeVariables,
			_cmpVariables, _freeVariables);
		if (graphId == GRAPH_RETURN_OK) {
			graphDestroy(pGraph);
			pGraph = temp;
		}
	} else if matches(command, "isdirected") {
		int actual, expected;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_RETURN_ARGUMENT;
		expected = matches(buffer, "YES");
		graphId = graphIsDirected(pGraph, &actual);
		if (graphId == GRAPH_RETURN_OK && expected != actual)
			return TESTER_RETURN_RETURN;
	} else if matches(command, "vertexcount") {
		unsigned long actual, expected;
		if (TesterParseArguments("l", &expected) != 1)
			return TESTER_RETURN_ARGUMENT;
		graphId = graphGetNumberOfVertices(pGraph, &actual);
		if (graphId == GRAPH_RETURN_OK && expected != actual)
			return TESTER_RETURN_RETURN;
	} else if matches(command, "nextvertex") {
		Variable *pVar, *temp;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_RETURN_ARGUMENT;
		if (varCreate(buffer, &pVar))
			return TESTER_RETURN_MALLOC;
		graphId = graphGetNextVertex(pGraph, &temp);
		if (graphId == GRAPH_RETURN_OK) {
			int areEqual;
			varCompare(pVar, temp, &areEqual);
			varDestroy(pVar);
			if (!areEqual)
				return TESTER_RETURN_RETURN;
		} else {
			varDestroy(pVar);
		}
	} else if matches(command, "outdegree") {
		Variable *pVar;
		unsigned long actual, expected;
		if (TesterParseArguments("sl", buffer, &expected) != 2)
			return TESTER_RETURN_ARGUMENT;
		if (varCreate(buffer, &pVar))
			return TESTER_RETURN_MALLOC;
		graphId = graphGetVertexOutDegree(pGraph, pVar, &actual);
		varDestroy(pVar);
		if (graphId == GRAPH_RETURN_OK && expected != actual)
			return TESTER_RETURN_RETURN;
	} else if matches(command, "indegree") {
		Variable *pVar;
		unsigned long actual, expected;
		if (TesterParseArguments("sl", buffer, &expected) != 2)
			return TESTER_RETURN_ARGUMENT;
		if (varCreate(buffer, &pVar))
			return TESTER_RETURN_MALLOC;
		graphId = graphGetVertexInDegree(pGraph, pVar, &actual);
		varDestroy(pVar);
		if (graphId == GRAPH_RETURN_OK && expected != actual)
			return TESTER_RETURN_RETURN;
	} else if matches(command, "degree") {
		Variable *pVar;
		unsigned long actual, expected;
		if (TesterParseArguments("sl", buffer, &expected) != 2)
			return TESTER_RETURN_ARGUMENT;
		if (varCreate(buffer, &pVar))
			return TESTER_RETURN_MALLOC;
		graphId = graphGetVertexDegree(pGraph, pVar, &actual);
		varDestroy(pVar);
		if (graphId == GRAPH_RETURN_OK && expected != actual)
			return TESTER_RETURN_RETURN;
	} else if matches(command, "nextneighbour") {
		Variable *pVertexVar, *pActualNeighbourVar, *pExpectedNeighbourVar, 
			*pActualEdgeVar, *pExpectedEdgeVar;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_RETURN_ARGUMENT;
		if (varCreateMultiple(
			buffer, &pVertexVar,
			buffer2, &pExpectedNeighbourVar,
			buffer3, &pExpectedEdgeVar, NULL))
			return TESTER_RETURN_MALLOC;
		graphId = graphGetNextNeighbour(pGraph, pVertexVar,
			&pActualNeighbourVar, &pActualEdgeVar);
		varDestroy(pVertexVar);
		if (graphId == GRAPH_RETURN_OK) {
			int neighbourEqual, edgeEqual;
			varCompare(pExpectedNeighbourVar, pActualNeighbourVar,
				&neighbourEqual);
			varCompare(pExpectedEdgeVar, pActualEdgeVar, &edgeEqual);
			varDestroy(pExpectedEdgeVar);
			varDestroy(pExpectedNeighbourVar);
			if (!neighbourEqual || !edgeEqual)
				return TESTER_RETURN_RETURN;
		} else {
			varDestroy(pExpectedEdgeVar);
			varDestroy(pExpectedNeighbourVar);
		}
	} else if matches(command, "nextinneighbour") {
		Variable *pVertexVar, *pActualNeighbourVar, *pExpectedNeighbourVar, 
			*pActualEdgeVar, *pExpectedEdgeVar;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_RETURN_ARGUMENT;
		if (varCreateMultiple(
			buffer, &pVertexVar,
			buffer2, &pExpectedNeighbourVar,
			buffer3, &pExpectedEdgeVar, NULL))
			return TESTER_RETURN_MALLOC;
		graphId = graphGetNextInNeighbour(pGraph, pVertexVar,
			&pActualNeighbourVar, &pActualEdgeVar);
		varDestroy(pVertexVar);
		if (graphId == GRAPH_RETURN_OK) {
			int neighbourEqual, edgeEqual;
			varCompare(pExpectedNeighbourVar, pActualNeighbourVar,
				&neighbourEqual);
			varCompare(pExpectedEdgeVar, pActualEdgeVar, &edgeEqual);
			varDestroy(pExpectedEdgeVar);
			varDestroy(pExpectedNeighbourVar);
			if (!neighbourEqual || !edgeEqual)
				return TESTER_RETURN_RETURN;
		} else {
			varDestroy(pExpectedEdgeVar);
			varDestroy(pExpectedNeighbourVar);
		}
	} else if matches(command, "nextoutneighbour") {
		Variable *pVertexVar, *pActualNeighbourVar, *pExpectedNeighbourVar, 
			*pActualEdgeVar, *pExpectedEdgeVar;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_RETURN_ARGUMENT;
		if (varCreateMultiple(
			buffer, &pVertexVar,
			buffer2, &pExpectedNeighbourVar,
			buffer3, &pExpectedEdgeVar, NULL))
			return TESTER_RETURN_MALLOC;
		graphId = graphGetNextOutNeighbour(pGraph, pVertexVar,
			&pActualNeighbourVar, &pActualEdgeVar);
		varDestroy(pVertexVar);
		if (graphId == GRAPH_RETURN_OK) {
			int neighbourEqual, edgeEqual;
			varCompare(pExpectedNeighbourVar, pActualNeighbourVar,
				&neighbourEqual);
			varCompare(pExpectedEdgeVar, pActualEdgeVar, &edgeEqual);
			varDestroy(pExpectedEdgeVar);
			varDestroy(pExpectedNeighbourVar);
			if (!neighbourEqual || !edgeEqual)
				return TESTER_RETURN_RETURN;
		} else {
			varDestroy(pExpectedEdgeVar);
			varDestroy(pExpectedNeighbourVar);
		}
	} else if matches(command, "containsvertex") {
		Variable *pVar;
		int actual, expected;
		if (TesterParseArguments("ss", buffer, buffer2) != 2)
			return TESTER_RETURN_ARGUMENT;
		if (varCreate(buffer, &pVar))
			return TESTER_RETURN_MALLOC;
		expected = matches(buffer2, "YES");
		graphId = graphContainsVertex(pGraph, pVar);
		varDestroy(pVar);
		actual = graphId == GRAPH_RETURN_CONTAINS_VERTEX;
		return (actual == expected) ? TESTER_RETURN_OK : TESTER_RETURN_RETURN;
	} else if matches(command, "addvertex") {
		Variable *pVar;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_RETURN_ARGUMENT;
		if (varCreate(buffer, &pVar))
			return TESTER_RETURN_MALLOC;
		if (graphId = graphAddVertex(pGraph, pVar))
			varDestroy(pVar);
	} else if matches(command, "removevertex") {
		Variable *pVar;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_RETURN_ARGUMENT;
		if (varCreate(buffer, &pVar))
			return TESTER_RETURN_MALLOC;
		graphId = graphRemoveVertex(pGraph, pVar);
		varDestroy(pVar);
	} else if matches(command, "containsedge") {
		Variable *pU, *pV;
		int actual, expected;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_RETURN_ARGUMENT;
		if (varCreateMultiple(buffer, &pU, buffer2, &pV, NULL))
			return TESTER_RETURN_MALLOC;
		expected = matches(buffer3, "YES");
		graphId = graphContainsEdge(pGraph, pU, pV);
		varDestroy(pU);
		varDestroy(pV);
		if (graphId == GRAPH_RETURN_CONTAINS_EDGE ||
			graphId == GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE) {
			actual = (graphId == GRAPH_RETURN_CONTAINS_EDGE);
			if (expected != actual)
				return TESTER_RETURN_RETURN;
			return TESTER_RETURN_OK;
		}
	} else if matches(command, "addedge") {
		Variable *pU, *pV, *pEdge;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_RETURN_ARGUMENT;
		if (varCreateMultiple(buffer, &pU, buffer2, &pV, buffer3, &pEdge,
			NULL))
			return TESTER_RETURN_MALLOC;
		if (graphId = graphAddEdge(pGraph, pU, pV, pEdge))
			varDestroy(pEdge);
		varDestroy(pU);
		varDestroy(pV);
	} else if matches(command, "getedge") {
		Variable *pU, *pV, *pExpected, *pActual;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_RETURN_ARGUMENT;
		if (varCreateMultiple(buffer, &pU, buffer2, &pV, buffer3, &pExpected,
			NULL))
			return TESTER_RETURN_MALLOC;
		graphId = graphGetEdge(pGraph, pU, pV, &pActual);
		varDestroy(pU);
		varDestroy(pV);
		if (graphId == GRAPH_RETURN_OK) {
			int areEqual;
			varCompare(pExpected, pActual, &areEqual);
			varDestroy(pExpected);
			if (!areEqual)
				return TESTER_RETURN_RETURN;
		} else {
			varDestroy(pExpected);
		}
	} else if matches(command, "removeedge") {
		Variable *pU, *pV;
		if (TesterParseArguments("ss", buffer, buffer2) != 2)
			return TESTER_RETURN_ARGUMENT;
		if (varCreateMultiple(buffer, &pU, buffer2, &pV, NULL))
			return TESTER_RETURN_MALLOC;
		graphId = graphRemoveEdge(pGraph, pU, pV);
		varDestroy(pU);
		varDestroy(pV);
	} else {
		return TESTER_RETURN_COUNT;
	}
	return convertRet(graphId);
}

static TesterReturnValue parseGraphIoCommands(const char *command)
{
	GraphIoReturnID graphIoId = GRAPH_IO_RETURN_OK;
	FILE *fp;
	if matches(command, "write") {
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_RETURN_ARGUMENT;
		fp = fopen(buffer, "w");
		if (fp == NULL)
			return TESTER_RETURN_FILE;
		graphIoId = graphWrite(pGraph, fp, _writeVariables, _writeVariables);
		fclose(fp);
	} else if matches(command, "read") {
		Graph *temp;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_RETURN_ARGUMENT;
		fp = fopen(buffer, "r");
		if (fp == NULL)
			return TESTER_RETURN_FILE;
		graphIoId = graphRead(&temp, fp, _readVariables, _readVariables,
			_cmpVariables, _freeVariables, _cmpVariables, _freeVariables);
		if (graphIoId == GRAPH_IO_RETURN_OK) {
			graphDestroy(pGraph);
			pGraph = temp;
		}
		fclose(fp);
	} else {
		return TESTER_RETURN_COUNT;
	}
	return convertIoRet(graphIoId); // TEMP
}

static TesterReturnValue parseGraphSearchCommands(const char *command)
{
	GraphSearchReturnID graphSearchId = GRAPH_SEARCH_RETURN_OK;
	if matches(command, " ") { // todo

	} else {
		return TESTER_RETURN_COUNT;
	}
	return convertSearchRet(graphSearchId);
}

// Here the TESTER_RETURN_COUNT is being used as a flag to indicate that
// the command is not of a certain type, since it is never used truly for
// testing purposes.
TesterReturnValue TesterParseCallback(const char *command)
{
	TesterReturnValue ret;
	if ((ret = parseGraphCommands(command))
		!= TESTER_RETURN_COUNT)
		return ret;
	if ((ret = parseGraphIoCommands(command))
		!= TESTER_RETURN_COUNT)
		return ret;
	if ((ret = parseGraphSearchCommands(command))
		!= TESTER_RETURN_COUNT)
		return ret;
	return TESTER_RETURN_COMMAND;
}

TesterReturnValue TesterExitCallback()
{
	if (pGraph) graphDestroy(pGraph);
#ifdef _DEBUG
	if (varGetRefCount())
		return TESTER_RETURN_MEMLEAK;
#endif
	return TESTER_RETURN_OK;
}

static void _freeVariables(void *v)
{
	varDestroy((Variable *) v);
}

static int _cmpVariables(void *a, void *b)
{
	int cmpResult;
	if (varCompare((Variable *) a, (Variable *) b, &cmpResult)) return 0;
	return cmpResult;
}

static int _readVariables(FILE *fp, void **ppVertex)
{
	return varDeserialize((Variable **) ppVertex, fp);
}

static int _writeVariables(FILE *fp, void *v)
{
	return varSerialize((Variable *) v, fp);
}

static TesterReturnValue convertRet(GraphReturnID graphId)
{
	switch (graphId) {
	case GRAPH_RETURN_OK:
		return TESTER_RETURN_OK;
	case GRAPH_RETURN_EMPTY:
		return TesterExternalReturnValue("empty");
	case GRAPH_RETURN_CONTAINS_VERTEX:
		return TesterExternalReturnValue("contains vertex");
	case GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX:
		return TesterExternalReturnValue("does not contain vertex");
	case GRAPH_RETURN_CONTAINS_EDGE:
		return TesterExternalReturnValue("contains edge");
	case GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE:
		return TesterExternalReturnValue("does not contain edge");
	case GRAPH_RETURN_INVALID_PARAMETER:
		return TesterExternalReturnValue("invalid parameter");
	case GRAPH_RETURN_MEMORY:
		return TesterExternalReturnValue("memory");
	default:
		return TesterExternalReturnValue("unknown");
	}
}

static TesterReturnValue convertIoRet(GraphIoReturnID graphIoId)
{
	switch (graphIoId) {
	case GRAPH_IO_RETURN_OK:
		return TESTER_RETURN_OK;
	case GRAPH_IO_RETURN_INVALID_PARAMETER:
		return TesterExternalReturnValue("invalid parameter");
	case GRAPH_IO_RETURN_MEMORY:
		return TesterExternalReturnValue("memory");
	case GRAPH_IO_RETURN_WRITING_FAILURE:
		return TesterExternalReturnValue("writing failure");
	case GRAPH_IO_RETURN_CREATION_FAILURE:
		return TesterExternalReturnValue("creation failure");
	case GRAPH_IO_RETURN_SAME_CREATION:
		return TesterExternalReturnValue("same creation");
	case GRAPH_IO_RETURN_FILE_ERROR:
		return TesterExternalReturnValue("file error");
	case GRAPH_IO_RETURN_DEPRECATED_FILE_FORMAT:
		return TesterExternalReturnValue("deprecated file format");
	case GRAPH_RETURN_FATAL_ERROR:
		return TesterExternalReturnValue("fatal error");
	default:
		return TesterExternalReturnValue("unknown");
	}
}

static TesterReturnValue convertSearchRet(GraphSearchReturnID graphSearchId)
{
	switch (graphSearchId) {
	case GRAPH_SEARCH_RETURN_OK:
		return TESTER_RETURN_OK;
	case GRAPH_SEARCH_RETURN_DOES_NOT_CONTAIN_VERTEX:
		return TesterExternalReturnValue("does not contain vertex");
	case GRAPH_SEARCH_RETURN_INVALID_PARAMETER:
		return TesterExternalReturnValue("invalid parameter");
	case GRAPH_SEARCH_RETURN_MEMORY:
		return TesterExternalReturnValue("memory");
	default:
		return TesterExternalReturnValue("unknown");
	}
}
