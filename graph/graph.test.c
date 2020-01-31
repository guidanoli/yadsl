#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "graph.h"
#include "graphio.h"
#include "graphsearch.h"
#include "var.h"

/* Help */

static const char *helpStrings[] = {
    "This is an interactive module of the graph library",
    "You will interact with the same graph object at all times",
    "Actions and flags to the graph are parsed by the command line arguments",
    "",
    "The registered actions are the following:",
    "n\tprints the number of vertices",
    "i\tprints next vertex",
    "t\tprints the type of the graph",
    "w\twrites the graph to the output file",
    "Xi\tprints next in-neighbour of vertex X",
    "Xo\tprints next out-neighbour of vertex X",
    "Xn\tprints neighbour of X",
    "Xd\tprints the degree of X",
    "X+\tadds vertex X",
    "X-\tremoves vertex X",
    "X?\tchecks if vertex X exists",
    "X,Y-\tremoves edge XY",
    "X,Y?\tchecks if edge XY exists",
    "X,Y,E+\tadds edge E between XY",
    "\\bfs(X)\tdoes a bfs from X",
    "\\dfs(X)\tdoes a dfs from X",
    "",
    "The registered flags are the following:",
    "type\tsets the graph type (DIRECTED or UNDIRECTED)",
    "in\tsets the input graph file",
    "out\tsets the output graph file",
    NULL,
};

/* Graph */

static Graph *g = NULL;
Variable *v1 = NULL, *v2 = NULL, *v3 = NULL;

/* Private functions prototypes */

static void free_filenames(char *input, char *output);
static void print_help();
static int _cmpVariables(void *a, void *b);
static void _freeVariables(void *v);
static int _readVariables(FILE *fp, void **ppVertex);
static int _writeVariables(FILE *fp, void *v);
static void _printVariables(void *v);
static void _garbageCollectVariables();
#define _transferOwnership(...) __transferOwnership(__VA_ARGS__, NULL);
static void __transferOwnership(Variable **v, ...);
static char *parse(int *nid, int *isDirected, char **inputFilename,
    char **outputFilename, char **argv, int argc);
static char *test(GraphReturnID *pid, int *nid, int isDirected,
    char **cmds, int count, const char *inFile, const char *outFile);

/* Main function */

int main(int argc, char **argv)
{
    GraphReturnID id = GRAPH_RETURN_OK;
    int nid = 1;
    int isDirected = 0;
    char *str = NULL;
    char *inputFilename = NULL, *outputFilename = "out.graph";
    /* ignore program name */
    char **commands = argv + 1;
    int commandCount = argc - 1;
    /* parse arguments */
    if (commandCount > 0) {
        str = parse(&nid, &isDirected, &inputFilename, &outputFilename, commands, commandCount);
        if (str) {
            fprintf(stderr, "Error while parsing argument #%d (%s): %s\n", nid, argv[nid], str);
            free_filenames(inputFilename, outputFilename);
            return 1;
        }
    } else {
        print_help();
        free_filenames(inputFilename, outputFilename);
        return 0;
    }
    /* run tests */
    str = test(&id, &nid, isDirected, commands, commandCount, inputFilename, outputFilename);
    _garbageCollectVariables();
    if (g)
        graphDestroy(g);
    g = NULL;
#ifdef _DEBUG
    if (varGetRefCount())
        fprintf(stderr, "Memory leak detected\n");
#endif
    if (str) {
        if (id) {
            fprintf(stderr, "Error #%d on the action #%d (%s): %s\n", id, nid, argv[nid], str);
        } else {
            fprintf(stderr, "Error on the action #%d (%s): %s\n", nid, argv[nid], str);
        }
        free_filenames(inputFilename, outputFilename);
        return 1;
    } else {
        fprintf(stdout, "No errors.\n");
    }
    free_filenames(inputFilename, outputFilename);
    return 0;
}

/* Private functions implementations */
static char *test(GraphReturnID *pid, int *nid, int isDirected,
    char **cmds, int count, const char *inFile, const char *outFile)
{
    unsigned long pSize;
    char cmd, flag[128], buff1[1024], buff2[1024], buff3[1024], end = 0;
    int i, ret, dir;
    FILE *fp;
    if (inFile == NULL) {
        if (*pid = graphCreate(&g, isDirected, _cmpVariables, _freeVariables,
            _cmpVariables, _freeVariables)) {
            if (*pid == GRAPH_RETURN_MEMORY)
                g = NULL;
            return "Could not create graph";
        }
    } else {
        fp = fopen(inFile, "r");
        if (fp == NULL)
            return "Could not open input file";
        if (*pid = graphRead(&g, fp, _readVariables, _readVariables, _cmpVariables,
            _freeVariables, _cmpVariables, _freeVariables)) {
            if (*pid == GRAPH_RETURN_MEMORY)
                g = NULL;
            return "Could not create graph from file";
        }
    }
    for (i = 0; i < count; ++i) {
        *nid = i + 1;
        // Ignore flags (--.*)
        if (sscanf(cmds[i], "--%s", flag) == 1)
            continue;
        // Special functions (\.*(.*)$)
        if (sscanf(cmds[i], "\\%[^(](%[^)])%c", buff1, buff2, &end) == 2) {
            if (strcmp(buff1, "dfs") == 0) {
                if (varCreate(buff2, &v1))
                    return "Could not create variable";
                if (*pid = graphDFS(g, v1, _printVariables))
                    return "Could not do DFS";
            } else if (strcmp(buff1, "bfs") == 0) {
                if (varCreate(buff2, &v1))
                    return "Could not create variable";
                if (*pid = graphBFS(g, v1, _printVariables))
                    return "Could not do BFS";
            } else {
                return "Unknown command";
            }
        // Commands with three arguments (.*,.*,.*[+]$)
        } else if (sscanf(cmds[i], "%[^,],%[^,],%[^+]%c%c", buff1, buff2, buff3, &cmd, &end) == 4) {
            if (varCreate(buff1, &v1))
                return "Could not create first variable";
            if (varCreate(buff2, &v2))
                return "Could not create second variable";
            if (varCreate(buff3, &v3))
                return "Could not create third variable";
            switch (cmd) {
            case '+':
                if (*pid = graphAddEdge(g, v1, v2, v3))
                    return "Could not add edge";
                _transferOwnership(&v3);
                break;
            default:
                return "Unknown command";
            }
        // Commands with two arguments (.*,.*[?-]$)
        } else if (sscanf(cmds[i], "%[^,],%[^?-]%c%c", buff1, buff2, &cmd, &end) == 3) {
            if (varCreate(buff1, &v1))
                return "Could not create first variable";
            if (varCreate(buff2, &v2))
                return "Could not create second variable";
            switch (cmd) {
            case '-':
                if (*pid = graphRemoveEdge(g, v1, v2))
                    return "Could not remove edge";
                break;
            case '?':
                *pid = graphContainsEdge(g, v1, v2);
                switch (*pid) {
                case GRAPH_RETURN_CONTAINS_EDGE:
                    fprintf(stdout, "[%d] Contains: ", *nid);
                    if (*pid = graphGetEdge(g, v1, v2, &v3))
                        return "Could not get edge";
                    varWrite(v3, stdout);
                    fprintf(stdout, "\n");
                    _transferOwnership(&v3);
                    break;
                case GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE:
                    fprintf(stdout, "[%d] Does not contain edge\n", *nid);
                    break;
                default:
                    return "Could not assert if edge was added or not";
                }
                break;
            default:
                return "Unknown command";
            }
        // Unary commands (.*[nodi+?-]$)
        } else if (sscanf(cmds[i], "%[^nodi+?-]%c%c", buff1, &cmd, &end) == 2) {
            if (varCreate(buff1, &v1))
                return "Could not create variable";
            switch (cmd) {
            case 'i':
                switch (*pid = graphGetNextInNeighbour(g, v1, &v2, &v3)) {
                case GRAPH_RETURN_OK:
                    fprintf(stdout, "[%d] ", *nid);
                    ret = varWrite(v2, stdout);
                    if (!ret)
                        fprintf(stdout, " (edge = ");
                    ret = ret || varWrite(v3, stdout);
                    _transferOwnership(&v2, &v3);
                    if (ret)
                        return "Could not write variable";
                    fprintf(stdout, ")\n");
                    break;
                case GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX:
                    fprintf(stdout, "[%d] Vertex ", *nid);
                    if (varWrite(v1, stdout))
                        return "Could not write variable";
                    fprintf(stdout, " is not in the graph\n");
                    break;
                case GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE:
                    fprintf(stdout, "[%d] Edge ", *nid);
                    if (varWrite(v1, stdout))
                        return "Could not write variable";
                    fprintf(stdout, " has no in-neighbours\n");
                    break;
                default:
                    return "Could not get next in-neighbour";
                }
                break;
            case 'o':
                switch (*pid = graphGetNextOutNeighbour(g, v1, &v2, &v3)) {
                case GRAPH_RETURN_OK:
                    fprintf(stdout, "[%d] ", *nid);
                    ret = varWrite(v2, stdout);
                    if (!ret)
                        fprintf(stdout, " (edge = ");
                    ret = ret || varWrite(v3, stdout);
                    _transferOwnership(&v2, &v3);
                    if (ret)
                        return "Could not write variable";
                    fprintf(stdout, ")\n");
                    break;
                case GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX:
                    fprintf(stdout, "[%d] Vertex ", *nid);
                    if (varWrite(v1, stdout))
                        return "Could not write variable";
                    fprintf(stdout, " is not in the graph\n");
                    break;
                case GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE:
                    fprintf(stdout, "[%d] Edge ", *nid);
                    if (varWrite(v1, stdout))
                        return "Could not write variable";
                    fprintf(stdout, " has no out-neighbours\n");
                    break;
                default:
                    return "Could not get next out-neighbour";
                }
                break;
            case 'n':
                switch (*pid = graphGetNextNeighbour(g, v1, &v2, &v3)) {
                case GRAPH_RETURN_OK:
                    fprintf(stdout, "[%d] ", *nid);
                    ret = varWrite(v2, stdout);
                    if (!ret)
                        fprintf(stdout, " (edge = ");
                    ret = ret || varWrite(v3, stdout);
                    _transferOwnership(&v2, &v3);
                    if (ret)
                        return "Could not write variable";
                    fprintf(stdout, ")\n");
                    break;
                case GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX:
                    fprintf(stdout, "[%d] Vertex ", *nid);
                    if (varWrite(v1, stdout))
                        return "Could not write variable";
                    fprintf(stdout, " is not in the graph\n");
                    break;
                case GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE:
                    fprintf(stdout, "[%d] Vertex ", *nid);
                    if (varWrite(v1, stdout))
                        return "Could not write variable";
                    fprintf(stdout, " has no out-neighbours\n");
                    break;
                default:
                    return "Could not get next out-neighbour";
                }
                break;
            case 'd':
                if (*pid = graphGetVertexDegree(g, v1, &pSize))
                    return "Could not get vertex degree";
                fprintf(stdout, "[%d] %lu neighbour(s) in total\n", *nid, pSize);
                if (isDirected) {
                    if (*pid = graphGetVertexInDegree(g, v1, &pSize))
                        return "Could not get number of in-neighbours";
                    fprintf(stdout, "[%d] %lu in-neighbour(s)\n", *nid, pSize);
                    if (*pid = graphGetVertexOutDegree(g, v1, &pSize))
                        return "Could not get number of out-neighbours";
                    fprintf(stdout, "[%d] %lu out-neighbour(s)\n", *nid, pSize);
                }
                break;
            case '?':
                *pid = graphContainsVertex(g, v1);
                switch (*pid) {
                case GRAPH_RETURN_CONTAINS_VERTEX:
                    fprintf(stdout, "[%d] Contains ", *nid);
                    varWrite(v1, stdout);
                    fprintf(stdout, "\n");
                    break;
                case GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX:
                    fprintf(stdout, "[%d] Does not contain ", *nid);
                    varWrite(v1, stdout);
                    fprintf(stdout, "\n");
                    break;
                default:
                    return "Could not assert if vertex was added or not";
                }
                break;
            case '+':
                if (*pid = graphAddVertex(g, v1))
                    return "Could not add vertex";
                _transferOwnership(&v1);
                break;
            case '-':
                if (*pid = graphRemoveVertex(g, v1))
                    return "Could not remove vertex";
                break;
            default:
                return "Unknown command";
            }    
        // One character commands (.$)
        } else if (sscanf(cmds[i], "%c%c", &cmd, &end) == 1) {
            switch (cmd) {
            case 't':
                if (*pid = graphIsDirected(g, &dir))
                    return "Could not get type";
                fprintf(stdout, "[%d] %s\n", *nid, dir ?
                    "Directed" : "Undirected");
                break;
            case 'n':
                if (*pid = graphGetNumberOfVertices(g, &pSize))
                    return "Could not get number of vertices";
                fprintf(stdout, "[%d] %lu\n", *nid, pSize);
                break;
            case 'w':
                fp = fopen(outFile, "w");
                if (!fp)
                    return "Could not open file";
                *pid = graphWrite(g, fp, _writeVariables, _writeVariables);
                fclose(fp);
                if (*pid)
                    return "Could not write to file";
                fprintf(stdout, "[%d] Wrote to file\n", *nid);
                break;
            case 'i':
                if (*pid = graphGetNextVertex(g, &v1))
                    return "Could not get next vertex";
                fprintf(stdout, "[%d] ", *nid);
                varWrite(v1, stdout);
                fprintf(stdout, "\n");
                _transferOwnership(&v1);
                break;
            default:
                return "Unknown command";
            }
        } else {
            return "Parsing error";
        }
        /* Local variables will be deallocated */
        _garbageCollectVariables();
    }
    return NULL;
}

static void print_help() {
    const char **str = helpStrings;
    for (; *str; ++str) puts(*str);
}

static char *parse(int *nid, int *isDirected, char **inputFilename,
    char **outputFilename, char **argv, int argc)
{
    int i;
    char flag[64], value[64], c = '0';
    *nid = 1;
    for (i = 0; i < argc; ++i) {
        if (sscanf(argv[i], "--%[^=]=%s", flag, value) == 2) {
            if (!strcmp(flag, "type")) {
                if (!strcmp(value, "DIRECTED"))
                    *isDirected = 1;
                else if (!strcmp(value, "UNDIRECTED"))
                    *isDirected = 0;
                else
                    return "Could not set graph type";
            } else if (!strcmp(flag, "in")) {
                *inputFilename = strdup(value);
            } else if (!strcmp(flag, "out")) {
                *outputFilename = strdup(value);
            } else {
                return "Unknown flag";
            }
        } else if (sscanf(argv[i], "--%s", flag) == 1) {
            if (!strcmp(flag, "help")) {
                print_help();
                return NULL; /* abort parsing */
            } else {
                return "Unknown flag";
            }
        }
        *nid++;
    }
    return NULL;
}

static void _freeVariables(void *v)
{
    varDestroy(v);
}

static int _cmpVariables(void *a, void *b)
{
    int cmpResult;
    if (varCompare(a, b, &cmpResult)) return 0;
    return cmpResult;
}

static int _readVariables(FILE *fp, void **ppVertex)
{
    return varDeserialize(ppVertex, fp);
}

static int _writeVariables(FILE *fp, void *v)
{
    return varSerialize(v, fp);
}

static void _garbageCollectVariables()
{
    if (v1) varDestroy(v1);
    if (v2) varDestroy(v2);
    if (v3) varDestroy(v3);
    v1 = NULL;
    v2 = NULL;
    v3 = NULL;
}

static void __transferOwnership(Variable **v, ...)
{
    va_list va;
    va_start(va, v);
    do { *v = NULL; }
    while (v = va_arg(va, Variable **));
    va_end(va);
}

static void free_filenames(char *input, char *output)
{
    if (input)
        free(input);
    if (output && strcmp(output, "out.graph"))
        free(output);
}

static void _printVariables(void *v)
{
    fprintf(stdout, "Visited ");
    varWrite(v, stdout);
    fprintf(stdout, "\n");
}