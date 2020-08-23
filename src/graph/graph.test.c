#include <yadsl/posixstring.h>
#include <graph/graph.h>
#include <graphio/graphio.h>
#include <graphsearch/graphsearch.h>

#include <stdlib.h>

#include <tester/tester.h>
#include <testerutils/testerutils.h>

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

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
	"/prevvertex <expected vertex>          get previous vertex in graph",
	"/outdegree <vertex> <expected>         get vertex out degree",
	"/indegree <vertex> <expected>          get vertex in degree",
	"/degree <vertex> <expected>            get vertex (total) degree",
	"/nextinneighbour <vertex> <nb> <edge>  get next vertex in-neighbour",
	"/nextoutneighbour <vertex> <nb> <edge> get next vertex out-neighbour",
	"/nextneighbour <vertex> <nb> <edge>    get next vertex neighbour",
	"/previnneighbour <vertex> <nb> <edge>  get previous vertex in-neighbour",
	"/prevoutneighbour <vertex> <nb> <edge> get previous vertex out-neighbour",
	"/prevneighbour <vertex> <nb> <edge>    get previous vertex neighbour",
	"/containsvertex <vertex> [YES/NO]      check if graph contains vertex",
	"/addvertex <vertex>                    add vertex to graph",
	"/removevertex <vertex>                 remove vertex from graph",
	"/containsedge <u> <v> [YES/NO]         check if graph contains edge",
	"/addedge <u> <v> <edge>                add edge to graph",
	"/getedge <u> <v> <edge>                get edge from graph",
	"/removeedege <u> <v>                   remove edge uv",
	"/setvertexflag <u> <flag>              set vertex flag",
	"/setallflags <flag>                    set flag of all vertices",
	"/getvertexflag <u> <expected>          get vertex flag",
	"",
	"Graph IO commands:",
	"/write <filename>                      write graph to file",
	"/read <filename>                       read from file to graph",
	"",
	"Graph Search commands:",
	"/dfs <v> <flag>                        run dfs on graph starting from v"
	"                                       v and marking visited with flag",
	"/bfs <v> <flag>                        run bfs on graph starting from v"
	"                                       v and marking visited with flag",
	NULL,
};

/* Graph */

static Graph *pGraph = NULL;
static char buffer[BUFSIZ], buffer2[BUFSIZ], buffer3[BUFSIZ];

static int _cmpStrings(void *a, void *b);
static int _readString(FILE *fp, void **ppVertex);
static int _writeString(FILE *fp, void *v);

static TesterReturnValue convertRet(GraphRet graphId);
static TesterReturnValue convertIoRet(GraphIoRet graphIoId);
static TesterReturnValue convertSearchRet(GraphSearchRet graphSearchId);

TesterReturnValue TesterInitCallback()
{
	GraphRet graphId;
	if (graphId = graphCreate(&pGraph, 1,
		_cmpStrings, free,
		_cmpStrings, free))
		return convertRet(graphId);
	return TESTER_OK;
}

static TesterReturnValue parseGraphCommands(const char *command)
{
	GraphRet graphId = GRAPH_OK;
	if matches(command, "create") {
		Graph *temp;
		int isDirected;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		isDirected = matches(buffer, "DIRECTED");
		graphId = graphCreate(&temp, isDirected, _cmpStrings, free,
			_cmpStrings, free);
		if (graphId == GRAPH_OK) {
			graphDestroy(pGraph);
			pGraph = temp;
		}
	} else if matches(command, "isdirected") {
		int actual, expected;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		expected = matches(buffer, "YES");
		graphId = graphIsDirected(pGraph, &actual);
		if (graphId == GRAPH_OK && expected != actual)
			return TESTER_RETURN;
	} else if matches(command, "vertexcount") {
		size_t actual, expected;
		if (TesterParseArguments("z", &expected) != 1)
			return TESTER_ARGUMENT;
		graphId = graphGetNumberOfVertices(pGraph, &actual);
		if (graphId == GRAPH_OK && expected != actual)
			return TESTER_RETURN;
	} else if matches(command, "nextvertex") {
		char *vertex;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		graphId = graphGetNextVertex(pGraph, &vertex);
		if (graphId == GRAPH_OK) {
			if (nmatches(buffer, vertex))
				return TESTER_RETURN;
		}
	} else if matches(command, "prevvertex") {
		char *vertex;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		graphId = graphGetPreviousVertex(pGraph, &vertex);
		if (graphId == GRAPH_OK) {
			if (nmatches(buffer, vertex))
				return TESTER_RETURN;
		}
	} else if matches(command, "outdegree") {
		size_t actual, expected;
		if (TesterParseArguments("sz", buffer, &expected) != 2)
			return TESTER_ARGUMENT;
		graphId = graphGetVertexOutDegree(pGraph, buffer, &actual);
		if (graphId == GRAPH_OK && expected != actual)
			return TESTER_RETURN;
	} else if matches(command, "indegree") {
		size_t actual, expected;
		if (TesterParseArguments("sz", buffer, &expected) != 2)
			return TESTER_ARGUMENT;
		graphId = graphGetVertexInDegree(pGraph, buffer, &actual);
		if (graphId == GRAPH_OK && expected != actual)
			return TESTER_RETURN;
	} else if matches(command, "degree") {
		size_t actual, expected;
		if (TesterParseArguments("sz", buffer, &expected) != 2)
			return TESTER_ARGUMENT;
		graphId = graphGetVertexDegree(pGraph, buffer, &actual);
		if (graphId == GRAPH_OK && expected != actual)
			return TESTER_RETURN;
	} else if matches(command, "nextneighbour") {
		char *v, *uv;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_ARGUMENT;
		graphId = graphGetNextNeighbour(pGraph, buffer, &v, &uv);
		if (graphId == GRAPH_OK) {
			if (nmatches(v, buffer2) || nmatches(uv, buffer3))
				return TESTER_RETURN;
		}
	} else if matches(command, "nextinneighbour") {
		char *v, *uv;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_ARGUMENT;
		graphId = graphGetNextInNeighbour(pGraph, buffer, &v, &uv);
		if (graphId == GRAPH_OK) {
			if (nmatches(v, buffer2) || nmatches(uv, buffer3))
				return TESTER_RETURN;
		}
	} else if matches(command, "nextoutneighbour") {
		char *v, *uv;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_ARGUMENT;
		graphId = graphGetNextOutNeighbour(pGraph, buffer, &v, &uv);
		if (graphId == GRAPH_OK) {
			if (nmatches(v, buffer2) || nmatches(uv, buffer3))
				return TESTER_RETURN;
		}
	} else if matches(command, "prevneighbour") {
		char *v, *uv;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_ARGUMENT;
		graphId = graphGetPreviousNeighbour(pGraph, buffer, &v, &uv);
		if (graphId == GRAPH_OK) {
			if (nmatches(v, buffer2) || nmatches(uv, buffer3))
				return TESTER_RETURN;
		}
	} else if matches(command, "previnneighbour") {
		char *v, *uv;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_ARGUMENT;
		graphId = graphGetPreviousInNeighbour(pGraph, buffer, &v, &uv);
		if (graphId == GRAPH_OK) {
			if (nmatches(v, buffer2) || nmatches(uv, buffer3))
				return TESTER_RETURN;
		}
	} else if matches(command, "prevoutneighbour") {
		char *v, *uv;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_ARGUMENT;
		graphId = graphGetPreviousOutNeighbour(pGraph, buffer, &v, &uv);
		if (graphId == GRAPH_OK) {
			if (nmatches(v, buffer2) || nmatches(uv, buffer3))
				return TESTER_RETURN;
		}
	} else if matches(command, "containsvertex") {
		int actual, expected;
		if (TesterParseArguments("ss", buffer, buffer2) != 2)
			return TESTER_ARGUMENT;
		expected = TesterGetYesOrNoFromString(buffer2);
		graphId = graphContainsVertex(pGraph, buffer, &actual);
		if (graphId == GRAPH_OK && actual != expected)
			return TESTER_RETURN;
	} else if matches(command, "addvertex") {
		char *vertex;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		if ((vertex = strdup(buffer)) == NULL)
			return TESTER_MALLOC;
		if (graphId = graphAddVertex(pGraph, vertex))
			free(vertex);
	} else if matches(command, "removevertex") {
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		graphId = graphRemoveVertex(pGraph, buffer);
	} else if matches(command, "containsedge") {
		int actual, expected;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_ARGUMENT;
		expected = TesterGetYesOrNoFromString(buffer3);
		graphId = graphContainsEdge(pGraph, buffer, buffer2, &actual);
		if (graphId == GRAPH_OK && actual != expected)
			return TESTER_RETURN;
	} else if matches(command, "addedge") {
		char *edge;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_ARGUMENT;
		if ((edge = strdup(buffer3)) == NULL)
			return TESTER_MALLOC;
		if (graphId = graphAddEdge(pGraph, buffer, buffer2, edge))
			free(edge);
	} else if matches(command, "getedge") {
		char *actual;
		if (TesterParseArguments("sss", buffer, buffer2, buffer3) != 3)
			return TESTER_ARGUMENT;
		graphId = graphGetEdge(pGraph, buffer, buffer2, &actual);
		if (graphId == GRAPH_OK) {
			if (nmatches(actual, buffer3))
				return TESTER_RETURN;
		}
	} else if matches(command, "removeedge") {
		if (TesterParseArguments("ss", buffer, buffer2) != 2)
			return TESTER_ARGUMENT;
		graphId = graphRemoveEdge(pGraph, buffer, buffer2);
	} else if matches(command, "setvertexflag") {
		int flag;
		if (TesterParseArguments("si", buffer, &flag) != 2)
			return TESTER_ARGUMENT;
		graphId = graphSetVertexFlag(pGraph, buffer, flag);
	} else if matches(command, "setallflags") {
		int flag;
		if (TesterParseArguments("i", &flag) != 1)
			return TESTER_ARGUMENT;
		graphId = graphSetAllVerticesFlags(pGraph, flag);
	} else if matches(command, "getvertexflag") {
		int actual, expected;
		if (TesterParseArguments("si", buffer, &expected) != 2)
			return TESTER_ARGUMENT;
		graphId = graphGetVertexFlag(pGraph, buffer, &actual);
		if (graphId == GRAPH_OK && actual != expected)
			return TESTER_RETURN;
	} else {
		return TESTER_COUNT;
	}
	return convertRet(graphId);
}

static TesterReturnValue parseGraphIoCommands(const char *command)
{
	GraphIoRet graphIoId = GRAPH_IO_OK;
	FILE *fp;
	if matches(command, "write") {
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		fp = fopen(buffer, "w");
		if (fp == NULL)
			return TESTER_FILE;
		graphIoId = graphWrite(pGraph, fp, _writeString, _writeString);
		fclose(fp);
	} else if matches(command, "read") {
		Graph *temp;
		if (TesterParseArguments("s", buffer) != 1)
			return TESTER_ARGUMENT;
		fp = fopen(buffer, "r");
		if (fp == NULL)
			return TESTER_FILE;
		graphIoId = graphRead(&temp, fp, _readString, _readString,
			_cmpStrings, free, _cmpStrings, free);
		if (graphIoId == GRAPH_IO_OK) {
			graphDestroy(pGraph);
			pGraph = temp;
		}
		fclose(fp);
	} else {
		return TESTER_COUNT;
	}
	return convertIoRet(graphIoId);
}

static void visitVertexCallback(void *vertex)
{
	printf("%s was visited\n", (char *) vertex);
}

static void visitEdgeCallback(void *source, void *edge, void *dest)
{
	printf("%s was visited (%s -> %s)\n",
		(char *) edge, (char *) source, (char *) dest);
}

static TesterReturnValue parseGraphSearchCommands(const char *command)
{
	GraphSearchRet graphSearchId = GRAPH_SEARCH_OK;
	int flag;
	if matches(command, "dfs") {
		if (TesterParseArguments("si", buffer, &flag) != 2)
			return TESTER_ARGUMENT;
		graphSearchId = graphDFS(pGraph,
			buffer,
			flag,
			visitVertexCallback,
			visitEdgeCallback);
	} else if matches(command, "bfs") {
		if (TesterParseArguments("si", buffer, &flag) != 2)
			return TESTER_ARGUMENT;
		graphSearchId = graphBFS(pGraph,
			buffer,
			flag,
			visitVertexCallback,
			visitEdgeCallback);
	} else {
		return TESTER_COUNT;
	}
	return convertSearchRet(graphSearchId);
}

// Here the TESTER_COUNT is being used as a flag to indicate that
// the command is not of a certain type, since it is never used truly for
// testing purposes.
TesterReturnValue TesterParseCallback(const char *command)
{
	TesterReturnValue ret;
	if ((ret = parseGraphCommands(command))
		!= TESTER_COUNT)
		return ret;
	if ((ret = parseGraphIoCommands(command))
		!= TESTER_COUNT)
		return ret;
	if ((ret = parseGraphSearchCommands(command))
		!= TESTER_COUNT)
		return ret;
	return TESTER_COMMAND;
}

TesterReturnValue TesterExitCallback()
{
	if (pGraph) graphDestroy(pGraph);
#ifdef _DEBUG
	if (getGraphSearchNodeRefCount())
		return TESTER_MEMLEAK;
#endif
	return TESTER_OK;
}

static int _cmpStrings(void *a, void *b)
{
	return matches((char *) a, (char *) b);
}

static int _readString(FILE *fp, void **ppVertex)
{
	return ((*ppVertex = TesterUtilsDeserializeString(fp)) == NULL);
}

static int _writeString(FILE *fp, void *v)
{
	return TesterUtilsSerializeString(fp, (char *) v);
}

static TesterReturnValue convertRet(GraphRet graphId)
{
	switch (graphId) {
	case GRAPH_OK:
		return TESTER_OK;
	case GRAPH_EMPTY:
		return TesterExternalReturnValue("empty");
	case GRAPH_CONTAINS_VERTEX:
		return TesterExternalReturnValue("contains vertex");
	case GRAPH_DOES_NOT_CONTAIN_VERTEX:
		return TesterExternalReturnValue("does not contain vertex");
	case GRAPH_CONTAINS_EDGE:
		return TesterExternalReturnValue("contains edge");
	case GRAPH_DOES_NOT_CONTAIN_EDGE:
		return TesterExternalReturnValue("does not contain edge");
	case GRAPH_MEMORY:
		return TesterExternalReturnValue("memory");
	default:
		return TesterExternalReturnValue("unknown");
	}
}

static TesterReturnValue convertIoRet(GraphIoRet graphIoId)
{
	switch (graphIoId) {
	case GRAPH_IO_OK:
		return TESTER_OK;
	case GRAPH_IO_MEMORY:
		return TesterExternalReturnValue("memory");
	case GRAPH_IO_WRITING_FAILURE:
		return TesterExternalReturnValue("writing failure");
	case GRAPH_IO_CREATION_FAILURE:
		return TesterExternalReturnValue("creation failure");
	case GRAPH_IO_SAME_CREATION:
		return TesterExternalReturnValue("same creation");
	case GRAPH_IO_FILE_ERROR:
		return TesterExternalReturnValue("file error");
	case GRAPH_IO_DEPRECATED_FILE_FORMAT:
		return TesterExternalReturnValue("deprecated file format");
	default:
		return TesterExternalReturnValue("unknown");
	}
}

static TesterReturnValue convertSearchRet(GraphSearchRet graphSearchId)
{
	switch (graphSearchId) {
	case GRAPH_SEARCH_OK:
		return TESTER_OK;
	case GRAPH_SEARCH_DOES_NOT_CONTAIN_VERTEX:
		return TesterExternalReturnValue("does not contain vertex");
	case GRAPH_SEARCH_VERTEX_ALREADY_VISITED:
		return TesterExternalReturnValue("vertex already visited");
	case GRAPH_SEARCH_MEMORY:
		return TesterExternalReturnValue("memory");
	default:
		return TesterExternalReturnValue("unknown");
	}
}
