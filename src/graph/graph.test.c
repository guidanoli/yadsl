#include <graph/graph.h>
#include <graphio/graphio.h>
#include <graphsearch/graphsearch.h>

#include <string.h>

#include <string/string.h>
#include <tester/tester.h>
#include <testerutils/testerutils.h>

#if defined(_MSC_VER)
# pragma warning(disable : 4996)
#endif

const char* yadsl_tester_help_strings[] = {
	"This is an interactive module of the graph library",
	"You will interact with the same graph object at all times",
	"A directed graph is already created from the start",
	"The registered actions are the following:",
	"",
	"Graph commands:",
	"/create [DIRECTED/UNDIRECTED]                           create new graph",
	"/isdirected [YES/NO]                                    check if graph is directed",
	"/vertexcount <expected>                                 get graph vertex count",
	"/ivertices <iter-dir> <expected>                        iterate through vertices",
	"/degree <vertex> <edge-dir> <expected>                  get vertex degree",
	"/ineighbours <vertex> <iter-dir> <edge-dir> <nb> <edge> iterate through neighbours",
	"/containsvertex <vertex> [YES/NO]                       check if graph contains vertex",
	"/addvertex <vertex>                                     add vertex to graph",
	"/removevertex <vertex>                                  remove vertex from graph",
	"/containsedge <u> <v> [YES/NO]                          check if graph contains edge",
	"/addedge <u> <v> <edge>                                 add edge to graph",
	"/getedge <u> <v> <edge>                                 get edge from graph",
	"/removeedege <u> <v>                                    remove edge uv",
	"/setvertexflag <u> <flag>                               set vertex flag",
	"/setallflags <flag>                                     set flag of all vertices",
	"/getvertexflag <u> <expected>                           get vertex flag",
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

static yadsl_GraphHandle* graph = NULL;
static char buffer[BUFSIZ], buffer2[BUFSIZ], buffer3[BUFSIZ], buffer4[BUFSIZ], buffer5[BUFSIZ];
static bool string_duplicate_failed = false;

static int compare_strings_func(void* a, void* b);
static int read_string_func(FILE* fp, void** vertex_ptr);
static int write_string_func(FILE* fp, void* v);

static bool parse_edge_direction(const char* buffer, yadsl_GraphEdgeDirection* edge_direction_ptr);
static bool parse_iteration_direction(const char* buffer, yadsl_GraphIterationDirection* iteration_direction_ptr);

static yadsl_TesterRet convert_graph_ret(yadsl_GraphRet graphId);
static yadsl_TesterRet convert_graph_io_ret(yadsl_GraphIoRet graphIoId);
static yadsl_TesterRet convert_graph_search_ret(yadsl_GraphSearchRet graphSearchId);

yadsl_TesterRet yadsl_tester_init()
{
	graph = yadsl_graph_create(1, compare_strings_func, free, compare_strings_func, free);
	return graph ? YADSL_TESTER_RET_OK : YADSL_TESTER_RET_MALLOC;
}

static yadsl_TesterRet parse_graph_command(const char* command)
{
	yadsl_GraphRet graph_ret = YADSL_GRAPH_RET_OK;
	if (yadsl_testerutils_match(command, "create")) {
		int is_directed;
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		is_directed = yadsl_testerutils_match(buffer, "DIRECTED");
		yadsl_GraphHandle* temp = yadsl_graph_create(is_directed, compare_strings_func, free, compare_strings_func, free);
		if (temp) {
			yadsl_graph_destroy(graph);
			graph = temp;
		} else {
			return YADSL_TESTER_RET_MALLOC;
		}
	} else if (yadsl_testerutils_match(command, "isdirected")) {
		bool actual, expected;
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		expected = yadsl_testerutils_str_to_bool(buffer);
		graph_ret = yadsl_graph_is_directed_check(graph, &actual);
		if (graph_ret == YADSL_GRAPH_RET_OK && expected != actual)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "vertexcount")) {
		size_t actual, expected;
		if (yadsl_tester_parse_arguments("z", &expected) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		graph_ret = yadsl_graph_vertex_count_get(graph, &actual);
		if (graph_ret == YADSL_GRAPH_RET_OK && expected != actual)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "ivertices")) {
		char* vertex;
		yadsl_GraphIterationDirection iteration_direction;
		if (yadsl_tester_parse_arguments("ss", buffer, buffer2) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		if (!parse_iteration_direction(buffer, &iteration_direction))
			return YADSL_TESTER_RET_ARGUMENT;
		graph_ret = yadsl_graph_vertex_iter(graph, iteration_direction, (yadsl_GraphVertexObject**) &vertex);
		if (graph_ret == YADSL_GRAPH_RET_OK)
			if (strcmp(buffer2, vertex))
				return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "degree")) {
		size_t actual, expected;
		yadsl_GraphEdgeDirection edge_direction;
		if (yadsl_tester_parse_arguments("ssz", buffer, buffer2, &expected) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		if (!parse_edge_direction(buffer2, &edge_direction))
			return YADSL_TESTER_RET_ARGUMENT;
		graph_ret = yadsl_graph_vertex_degree_get(graph, buffer, edge_direction, &actual);
		if (graph_ret == YADSL_GRAPH_RET_OK && expected != actual)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "ineighbours")) {
		char* v, * uv;
		yadsl_GraphIterationDirection iteration_direction;
		yadsl_GraphEdgeDirection edge_direction;
		if (yadsl_tester_parse_arguments("sssss", buffer, buffer2, buffer3, buffer4, buffer5) != 5)
			return YADSL_TESTER_RET_ARGUMENT;
		if (!parse_iteration_direction(buffer2, &iteration_direction))
			return YADSL_TESTER_RET_ARGUMENT;
		if (!parse_edge_direction(buffer3, &edge_direction))
			return YADSL_TESTER_RET_ARGUMENT;
		graph_ret = yadsl_graph_vertex_nb_iter(graph, buffer, edge_direction,
			iteration_direction, (yadsl_GraphVertexObject**) &v, (yadsl_GraphEdgeObject**) &uv);
		if (graph_ret == YADSL_GRAPH_RET_OK)
			if (strcmp(v, buffer4) || strcmp(uv, buffer5))
				return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "containsvertex")) {
		bool actual, expected;
		if (yadsl_tester_parse_arguments("ss", buffer, buffer2) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		expected = yadsl_testerutils_str_to_bool(buffer2);
		graph_ret = yadsl_graph_vertex_exists_check(graph, buffer, &actual);
		if (graph_ret == YADSL_GRAPH_RET_OK && actual != expected)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "addvertex")) {
		char* vertex;
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((vertex = yadsl_string_duplicate(buffer)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		if (graph_ret = yadsl_graph_vertex_add(graph, vertex))
			free(vertex);
	} else if (yadsl_testerutils_match(command, "removevertex")) {
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		graph_ret = yadsl_graph_vertex_remove(graph, buffer);
	} else if (yadsl_testerutils_match(command, "containsedge")) {
		bool actual, expected;
		if (yadsl_tester_parse_arguments("sss", buffer, buffer2, buffer3) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		expected = yadsl_testerutils_str_to_bool(buffer3);
		graph_ret = yadsl_graph_edge_exists_check(graph, buffer, buffer2, &actual);
		if (graph_ret == YADSL_GRAPH_RET_OK && actual != expected)
			return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "addedge")) {
		char* edge;
		if (yadsl_tester_parse_arguments("sss", buffer, buffer2, buffer3) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		if ((edge = yadsl_string_duplicate(buffer3)) == NULL)
			return YADSL_TESTER_RET_MALLOC;
		if (graph_ret = yadsl_graph_edge_add(graph, buffer, buffer2, edge))
			free(edge);
	} else if (yadsl_testerutils_match(command, "getedge")) {
		char* actual;
		if (yadsl_tester_parse_arguments("sss", buffer, buffer2, buffer3) != 3)
			return YADSL_TESTER_RET_ARGUMENT;
		graph_ret = yadsl_graph_edge_get(graph, buffer, buffer2, (yadsl_GraphEdgeObject**) &actual);
		if (graph_ret == YADSL_GRAPH_RET_OK)
			if (strcmp(actual, buffer3))
				return YADSL_TESTER_RET_RETURN;
	} else if (yadsl_testerutils_match(command, "removeedge")) {
		if (yadsl_tester_parse_arguments("ss", buffer, buffer2) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		graph_ret = yadsl_graph_edge_remove(graph, buffer, buffer2);
	} else if (yadsl_testerutils_match(command, "setvertexflag")) {
		int flag;
		if (yadsl_tester_parse_arguments("si", buffer, &flag) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		graph_ret = yadsl_graph_vertex_flag_set(graph, buffer, flag);
	} else if (yadsl_testerutils_match(command, "setallflags")) {
		int flag;
		if (yadsl_tester_parse_arguments("i", &flag) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		graph_ret = yadsl_graph_vertex_flag_set_all(graph, flag);
	} else if (yadsl_testerutils_match(command, "getvertexflag")) {
		int actual, expected;
		if (yadsl_tester_parse_arguments("si", buffer, &expected) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		graph_ret = yadsl_graph_vertex_flag_get(graph, buffer, &actual);
		if (graph_ret == YADSL_GRAPH_RET_OK && actual != expected)
			return YADSL_TESTER_RET_RETURN;
	} else {
		return YADSL_TESTER_RET_COUNT;
	}
	return convert_graph_ret(graph_ret);
}

static yadsl_TesterRet parse_graph_io_command(const char* command)
{
	yadsl_GraphIoRet graph_io_ret = YADSL_GRAPHIO_RET_OK;
	FILE* file_ptr;
	if (yadsl_testerutils_match(command, "write")) {
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		file_ptr = fopen(buffer, "w");
		if (file_ptr == NULL)
			return YADSL_TESTER_RET_FILE;
		graph_io_ret = yadsl_graphio_write(graph, file_ptr, write_string_func, write_string_func);
		fclose(file_ptr);
		if (!yadsl_testerutils_add_tempfile_to_list(buffer))
			return YADSL_TESTER_RET_MALLOC;
	} else if (yadsl_testerutils_match(command, "read")) {
		yadsl_GraphHandle* temp;
		if (yadsl_tester_parse_arguments("s", buffer) != 1)
			return YADSL_TESTER_RET_ARGUMENT;
		file_ptr = fopen(buffer, "r");
		if (file_ptr == NULL)
			return YADSL_TESTER_RET_FILE;
		graph_io_ret = yadsl_graphio_read(file_ptr, read_string_func, read_string_func, compare_strings_func, free, compare_strings_func, free, &temp);
		if (graph_io_ret == YADSL_GRAPHIO_RET_OK) {
			yadsl_graph_destroy(graph);
			graph = temp;
		} else if (graph_io_ret == YADSL_GRAPHIO_RET_CREATION_FAILURE && string_duplicate_failed) {
			graph_io_ret = YADSL_GRAPHIO_RET_MEMORY; /* So that it gets ignored when --fail-memory-allocation is set */
		}
		fclose(file_ptr);
	} else {
		return YADSL_TESTER_RET_COUNT;
	}
	return convert_graph_io_ret(graph_io_ret);
}

static void visit_vertex_func(void* vertex)
{
	printf("%s was visited\n", (char*) vertex);
}

static void visit_edge_func(void* source, void* edge, void* dest)
{
	printf("%s was visited (%s -> %s)\n", (char*) edge, (char*) source, (char*) dest);
}

static yadsl_TesterRet parse_graph_search_command(const char* command)
{
	yadsl_GraphSearchRet graph_search_ret = YADSL_GRAPHSEARCH_RET_OK;
	int flag;
	if (yadsl_testerutils_match(command, "dfs")) {
		if (yadsl_tester_parse_arguments("si", buffer, &flag) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		graph_search_ret = yadsl_graphsearch_dfs(graph, buffer, flag, visit_vertex_func, visit_edge_func);
	} else if (yadsl_testerutils_match(command, "bfs")) {
		if (yadsl_tester_parse_arguments("si", buffer, &flag) != 2)
			return YADSL_TESTER_RET_ARGUMENT;
		graph_search_ret = yadsl_graphsearch_bfs(graph, buffer, flag, visit_vertex_func, visit_edge_func);
	} else {
		return YADSL_TESTER_RET_COUNT;
	}
	return convert_graph_search_ret(graph_search_ret);
}

/**
 * Here the YADSL_TESTER_RET_COUNT is being used as a flag to indicate that
 * the command is not of a certain type, since it is never used truly for
 * testing purposes. 
*/
yadsl_TesterRet yadsl_tester_parse(const char* command)
{
	yadsl_TesterRet ret;
	if ((ret = parse_graph_command(command)) != YADSL_TESTER_RET_COUNT)
		return ret;
	if ((ret = parse_graph_io_command(command)) != YADSL_TESTER_RET_COUNT)
		return ret;
	if ((ret = parse_graph_search_command(command)) != YADSL_TESTER_RET_COUNT)
		return ret;
	return YADSL_TESTER_RET_COMMAND;
}

yadsl_TesterRet yadsl_tester_release()
{
	yadsl_testerutils_clear_tempfile_list();

	if (graph)
		yadsl_graph_destroy(graph);

#ifdef YADSL_DEBUG
	if (yadsl_graphsearch_get_node_ref_count())
		return YADSL_TESTER_RET_MEMLEAK;
#endif

	return YADSL_TESTER_RET_OK;
}

int compare_strings_func(void* a, void* b)
{
	return strcmp((char*) a, (char*) b) == 0;
}

int read_string_func(FILE* fp, void** vertex_ptr)
{
	char* str = yadsl_testerutils_str_deserialize(fp);
	
	if (str == NULL)
		string_duplicate_failed = true;
	else
		*vertex_ptr = str;

	return str == NULL;
}

int write_string_func(FILE* fp, void* v)
{
	return yadsl_testerutils_str_serialize(fp, (char*) v);
}

yadsl_TesterRet convert_graph_ret(yadsl_GraphRet graphId)
{
	switch (graphId) {
	case YADSL_GRAPH_RET_OK:
		return YADSL_TESTER_RET_OK;
	case YADSL_GRAPH_RET_EMPTY:
		return yadsl_tester_error("empty");
	case YADSL_GRAPH_RET_CONTAINS_VERTEX:
		return yadsl_tester_error("contains vertex");
	case YADSL_GRAPH_RET_DOES_NOT_CONTAIN_VERTEX:
		return yadsl_tester_error("does not contain vertex");
	case YADSL_GRAPH_RET_CONTAINS_EDGE:
		return yadsl_tester_error("contains edge");
	case YADSL_GRAPH_RET_DOES_NOT_CONTAIN_EDGE:
		return yadsl_tester_error("does not contain edge");
	case YADSL_GRAPH_RET_MEMORY:
		return YADSL_TESTER_RET_MALLOC;
	case YADSL_GRAPH_RET_PARAMETER:
		return yadsl_tester_error("parameter");
	default:
		return yadsl_tester_error("unknown");
	}
}

yadsl_TesterRet convert_graph_io_ret(yadsl_GraphIoRet graphIoId)
{
	switch (graphIoId) {
	case YADSL_GRAPHIO_RET_OK:
		return YADSL_TESTER_RET_OK;
	case YADSL_GRAPHIO_RET_MEMORY:
		return YADSL_TESTER_RET_MALLOC;
	case YADSL_GRAPHIO_RET_WRITING_FAILURE:
		return yadsl_tester_error("writing failure");
	case YADSL_GRAPHIO_RET_CREATION_FAILURE:
		return yadsl_tester_error("creation failure");
	case YADSL_GRAPHIO_RET_SAME_CREATION:
		return yadsl_tester_error("same creation");
	case YADSL_GRAPHIO_RET_FILE_ERROR:
		return yadsl_tester_error("file error");
	case YADSL_GRAPHIO_RET_DEPRECATED_FILE_FORMAT:
		return yadsl_tester_error("deprecated file format");
	default:
		return yadsl_tester_error("unknown");
	}
}

yadsl_TesterRet convert_graph_search_ret(yadsl_GraphSearchRet graphSearchId)
{
	switch (graphSearchId) {
	case YADSL_GRAPHSEARCH_RET_OK:
		return YADSL_TESTER_RET_OK;
	case YADSL_GRAPHSEARCH_RET_DOES_NOT_CONTAIN_VERTEX:
		return yadsl_tester_error("does not contain vertex");
	case YADSL_GRAPHSEARCH_RET_VERTEX_ALREADY_VISITED:
		return yadsl_tester_error("vertex already visited");
	case YADSL_GRAPHSEARCH_RET_MEMORY:
		return YADSL_TESTER_RET_MALLOC;
	default:
		return yadsl_tester_error("unknown");
	}
}

bool parse_edge_direction(const char* buffer, yadsl_GraphEdgeDirection* edge_direction_ptr)
{
	if (yadsl_testerutils_match(buffer, "in")) {
		*edge_direction_ptr = YADSL_GRAPH_EDGE_DIR_IN;
	} else if (yadsl_testerutils_match(buffer, "out")) {
		*edge_direction_ptr = YADSL_GRAPH_EDGE_DIR_OUT;
	} else if (yadsl_testerutils_match(buffer, "both")) {
		*edge_direction_ptr = YADSL_GRAPH_EDGE_DIR_BOTH;
	} else {
		yadsl_tester_log("Unknown edge direction \"%s\"", buffer);
		return false;
	}
	return true;
}

bool parse_iteration_direction(const char* buffer, yadsl_GraphIterationDirection* iteration_direction_ptr)
{
	if (yadsl_testerutils_match(buffer, "next")) {
		*iteration_direction_ptr = YADSL_GRAPH_ITER_DIR_NEXT;
	} else if (yadsl_testerutils_match(buffer, "previous")) {
		*iteration_direction_ptr = YADSL_GRAPH_ITER_DIR_PREVIOUS;
	} else {
		yadsl_tester_log("Unknown iteration direction \"%s\"", buffer);
		return false;
	}
	return true;
}