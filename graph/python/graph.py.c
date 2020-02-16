#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include "graph.h"

typedef struct {
    PyObject_HEAD
    Graph* ob_graph;
} GraphObject;

static void
Graph_decRefCallback(void* item)
{
    Py_DECREF((PyObject*)item);
}

static int
Graph_cmpCallback(void* a, void* b)
{
    int cmp;
    Py_INCREF(a);
    cmp = PyObject_RichCompareBool(
        (PyObject*) a,
        (PyObject*) b,
        Py_EQ);
    Py_DECREF(a);
    /*
    Ignore errors, for now. Later, it should
    be checked if a Python Exception was raised.
    */
    return cmp == -1 ? 0 : cmp;
}

static PyObject*
Graph_new(PyTypeObject* type, PyObject* args, PyObject* kw)
{
    GraphObject* self;
    self = (GraphObject*) type->tp_alloc(type, 0);
    if (self != NULL)
        self->ob_graph = NULL;
    return (PyObject*) self;
}

PyDoc_STRVAR(_Graph_init__doc__,
"Graph(/, directed=True)\n"
"--\n"
"\n"
"Python graph data structure.");

static int
Graph_init(GraphObject* self, PyObject* args, PyObject* kw)
{
    int isDirected = 1;
    static char *keywords[] = {"directed", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kw,
        "|p:pygraph.Graph.__init__", keywords, &isDirected))
        return -1;
    if (graphCreate(&self->ob_graph, isDirected,
        Graph_cmpCallback, Graph_decRefCallback,
        Graph_cmpCallback, Graph_decRefCallback))
        return -1;
    return 0;
}

static void
Graph_dealloc(GraphObject* self)
{
    if (self->ob_graph)
        graphDestroy(self->ob_graph);
    Py_TYPE(self)->tp_free((PyObject*) self);
}

PyDoc_STRVAR(_Graph_is_directed__doc__,
"is_directed(self, /) -> bool\n"
"--\n"
"\n"
"Check whether graph is directed or not");

static PyObject*
Graph_is_directed(GraphObject* self, PyObject* args, PyObject* kw)
{
    int isDirected;
    if (graphIsDirected(self->ob_graph, &isDirected))
        return NULL;
    return PyBool_FromLong(isDirected);
}

PyDoc_STRVAR(_Graph_add_vertex__doc__,
"add_vertex(self, u : Object, /) -> None\n"
"--\n"
"\n"
"Add vertex to graph");

static PyObject*
Graph_add_vertex(GraphObject* self, PyObject* obj)
{
    GraphReturnID returnId;
    returnId = graphAddVertex(self->ob_graph, obj);
    if (!returnId)
        Py_XINCREF(obj);
    if (PyErr_Occurred())
        goto exit;
    switch (returnId) {
    case GRAPH_RETURN_OK:
        Py_RETURN_NONE;
    case GRAPH_RETURN_CONTAINS_VERTEX:
        /*
        SEMANTICS: Even though two vertices might be
        semantically equal (__eq__), the PyObject will
        not be replaced, so the user should be notified.
        DESIGN CHOICE: An exception is thrown to notify
        the user that the vertex was already in the graph.
        */
        PyErr_SetString(PyExc_RuntimeError,
            "Vertex already existed in graph");
        break;
    case GRAPH_RETURN_MEMORY:
        return PyErr_NoMemory();
    default:
        PyErr_BadInternalCall();
    }
exit:
    return NULL;
}

PyDoc_STRVAR(_Graph_remove_vertex__doc__,
"remove_vertex(self, u : Object, /) -> None\n"
"--\n"
"\n"
"Remove vertex from graph");

static PyObject*
Graph_remove_vertex(GraphObject* self, PyObject* obj)
{
    GraphReturnID returnId;
    returnId = graphRemoveVertex(self->ob_graph, obj);
    if (PyErr_Occurred())
        goto exit;
    switch (returnId) {
    case GRAPH_RETURN_OK:
        Py_RETURN_NONE;
    case GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX:
        PyErr_SetString(PyExc_RuntimeError,
            "Vertex not found in graph");
        break;
    default:
        PyErr_BadInternalCall();
    }
exit:
    return NULL;
}

PyDoc_STRVAR(_Graph_add_edge__doc__,
"add_edge(self, u : Object, v : Object, uv : Object, /) -> None\n"
"--\n"
"\n"
"Add edge uv (from u to v) to graph");

static PyObject*
Graph_add_edge(GraphObject* self, PyObject* args, PyObject* kw)
{
    GraphReturnID returnId;
    PyObject *u, *v, *uv;
    if (!PyArg_ParseTuple(args,
        "OOO:pygraph.Graph.add_edge",
        &u, &v, &uv))
        goto exit;
    returnId = graphAddEdge(self->ob_graph, u, v, uv);
    if (!returnId)
        Py_XINCREF(uv);
    if (PyErr_Occurred())
        goto exit;
    switch (returnId) {
    case GRAPH_RETURN_OK:
        Py_RETURN_NONE;
    case GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX:
        PyErr_SetString(PyExc_RuntimeError,
            "Vertex not found in graph");
        break;
    case GRAPH_RETURN_CONTAINS_EDGE:
        PyErr_SetString(PyExc_RuntimeError,
            "Edge already found in graph");
        break;
    case GRAPH_RETURN_MEMORY:
        return PyErr_NoMemory();
    default:
        PyErr_BadInternalCall();
    }
exit:
    return NULL;
}

//static PyObject*
//Graph_remove_edge(GraphObject* self, PyObject* args, PyObject* kw)
//{
//
//}
//
//static PyObject*
//Graph_get_edge(GraphObject* self, PyObject* args, PyObject* kw)
//{
//
//}

PyMethodDef Graph_methods[] = {
    {
        "is_directed",
        (PyCFunction) Graph_is_directed,
        METH_NOARGS,
        _Graph_is_directed__doc__
    },
    {
        "add_vertex",
        (PyCFunction) Graph_add_vertex,
        METH_O,
        _Graph_add_vertex__doc__
    },
    {
        "remove_vertex",
        (PyCFunction) Graph_remove_vertex,
        METH_O,
        _Graph_remove_vertex__doc__
    },
    {
        "add_edge",
        (PyCFunction) Graph_add_edge,
        METH_VARARGS,
        _Graph_add_edge__doc__
    },
    {NULL, NULL, 0, NULL}
};

static int
Graph_as_sequence_contains(GraphObject* self, PyObject* value)
{
    GraphReturnID returnId;
    int contains = -1;
    /*
    SEMANTICS: In "(u,v) in g", (u,v) is treated as
    an edge between u and v, in that order. In other
    ocasions, the object is treated as a vertex.
    DESIGN CHOICE: Altough it restricts the usage of
    this notation to vertices that aren't tuples with
    two items, this seems very specific not to simplify
    the __contains__ notation.
    */
    if (PyTuple_Check(value) && PyTuple_Size(value) == 2) {
        PyObject *first, *second;
        first = PyTuple_GetItem(value, 0);
        second = PyTuple_GetItem(value, 1);
        returnId = graphContainsEdge(self->ob_graph,
            first,
            second,
            &contains);
        if (PyErr_Occurred())
            return -1;
        if (returnId) {
            if (returnId == GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX) {
                /*
                SEMANTICS: If u or v are not in the vertex set,
                then (u,v) is not in edge set.
                DESIGN CHOICE: Here an exception could be thrown,
                but I opted not to for simplicity reasons.
                */
                contains = 0;
            } else {
                PyErr_BadInternalCall();
            }
        }
    } else {
        returnId = graphContainsVertex(self->ob_graph, value, &contains);
        if (PyErr_Occurred())
            return -1;
        if (returnId)
            PyErr_BadInternalCall();
    }
    return contains;
}

static PySequenceMethods Graph_as_sequence = {
    .sq_contains = (objobjproc) Graph_as_sequence_contains
};

static PyTypeObject GraphType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "pygraph.Graph",
    .tp_doc = "Graph object",
    .tp_basicsize = sizeof(GraphObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = _Graph_init__doc__,
    .tp_new = (newfunc) Graph_new,
    .tp_init = (initproc) Graph_init,
    .tp_dealloc = (destructor) Graph_dealloc,
    .tp_methods = Graph_methods,
    .tp_as_sequence = &Graph_as_sequence,
};

PyModuleDef pygraph_module = {
    PyModuleDef_HEAD_INIT,
    "pygraph",
};

PyMODINIT_FUNC PyInit_pygraph(void) {
    PyObject* m;
    Py_Initialize();
    if (PyType_Ready(&GraphType) < 0)
        return NULL;
    m = PyModule_Create(&pygraph_module);
    if (m == NULL)
        return NULL;
    Py_INCREF(&GraphType);
    if (PyModule_AddObject(m, "Graph", (PyObject*) &GraphType) < 0) {
        Py_DECREF(&GraphType);
        Py_DECREF(m);
        return NULL;
    }
    return m;
}