#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <common/pydefines.h>
#include <common/assert.h>

#include "graph.h"

typedef struct {
    PyObject_HEAD
    Graph *ob_graph;
	size_t global_offset;
} GraphObject;

typedef struct {
	PyObject_HEAD
	GraphObject *go;
	size_t local_index;
} GraphVertexIteratorObject;

static PyTypeObject GraphType;
static PyTypeObject GraphIterType;

static void
decRefCallback(void *item)
{
	Py_DECREF((PyObject *) item);
}

static int
cmpCallback(void *a, void *b)
{
	int cmp;
	Py_INCREF(a);
	cmp = PyObject_RichCompareBool(
		(PyObject *) a,
		(PyObject *) b,
		Py_EQ);
	Py_DECREF(a);
	return cmp == -1 ? 0 : cmp;
}

static PyObject *
new(PyTypeObject *type, PyObject *args, PyObject *kw)
{
	GraphObject *self;
	self = (GraphObject *) type->tp_alloc(type, 0);
	if (self != NULL) {
		self->ob_graph = NULL;
		self->global_offset = 0;
	}
	return (PyObject *) self;
}

PyDoc_STRVAR(_init__doc__,
"Graph(/, directed=True)\n"
"--\n"
"\n"
"Python graph data structure.");

static int
__init__(GraphObject *self, PyObject *args, PyObject *kw)
{
	int isDirected = 1;
	static char *keywords[] = { "directed", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kw,
		"|p:pygraph.Graph." __FUNCTION__, keywords, &isDirected))
		return -1;
	if (graphCreate(&self->ob_graph, isDirected,
		cmpCallback, decRefCallback,
		cmpCallback, decRefCallback))
		return -1;
	return 0;
}

static void
dealloc(GraphObject *self)
{
	if (self->ob_graph)
		graphDestroy(self->ob_graph);
	Py_TYPE(self)->tp_free((PyObject *) self);
}

PyDoc_STRVAR(_is_directed__doc__,
"is_directed(self, /) -> bool\n"
"--\n"
"\n"
"Check whether graph is directed or not");

static PyObject *
is_directed(GraphObject *self, PyObject *args, PyObject *kw)
{
	int isDirected;
	if (graphIsDirected(self->ob_graph, &isDirected))
		return NULL;
	return PyBool_FromLong(isDirected);
}

PyDoc_STRVAR(_contains_vertex__doc__,
"contains_vertex(self, u : Object, /) -> None\n"
"--\n"
"\n"
"Check whether vertex is contained in graph");

static PyObject *
contains_vertex(GraphObject *self, PyObject *obj)
{
	int contains;
	if (graphContainsVertex(self->ob_graph, obj, &contains))
		PyErr_BadInternalCall();
	if (PyErr_Occurred())
		return NULL;
	return PyBool_FromLong(contains);
}

PyDoc_STRVAR(_add_vertex__doc__,
"add_vertex(self, u : Object, /) -> None\n"
"--\n"
"\n"
"Add vertex to graph");

static PyObject *
add_vertex(GraphObject *self, PyObject *obj)
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
		PyErr_SetString(PyExc_RuntimeError,
			"Vertex already existed in graph");
		break;
	case GRAPH_RETURN_MEMORY:
		return PyErr_NoMemory();
	default:
		Py_UNREACHABLE();
	}
exit:
	return NULL;
}

PyDoc_STRVAR(_remove_vertex__doc__,
"remove_vertex(self, u : Object, /) -> None\n"
"--\n"
"\n"
"Remove vertex from graph");

static PyObject *
remove_vertex(GraphObject *self, PyObject *obj)
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
		Py_UNREACHABLE();
	}
exit:
	return NULL;
}

PyDoc_STRVAR(_get_vertex_count__doc__,
"get_vertex_count(/) -> int\n"
"--\n"
"\n"
"Get number of vertices in graph");

static PyObject *
get_vertex_count(GraphObject *self, PyObject *Py_UNUSED(ignored))
{
	size_t size;
	if (graphGetNumberOfVertices(self->ob_graph, &size))
		PyErr_BadInternalCall();
	return PyLong_FromSize_t(size);
}

PyDoc_STRVAR(_contains_edge__doc__,
"contains_edge(self, u : Object, v : Object, /) -> bool\n"
"--\n"
"\n"
"Check whether there is an edge from u to v in graph");

static PyObject *
contains_edge(GraphObject *self, PyObject *args, PyObject *kw)
{
	GraphReturnID returnId;
	PyObject *u, *v;
	int contains;
	if (!PyArg_ParseTuple(args, "OO:pygraph.Graph." __FUNCTION__, &u, &v))
		goto exit;
	returnId = graphContainsEdge(self->ob_graph, u, v, &contains);
	if (PyErr_Occurred())
		goto exit;
	switch (returnId) {
	case GRAPH_RETURN_OK:
		return PyBool_FromLong(contains);
	case GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX:
		PyErr_SetString(PyExc_RuntimeError,
			"Vertex not found in graph");
		break;
	default:
		Py_UNREACHABLE();
	}
exit:
	return NULL;
}


PyDoc_STRVAR(_add_edge__doc__,
"add_edge(self, u : Object, v : Object, uv : Object, /) -> None\n"
"--\n"
"\n"
"Add edge uv (from u to v) to graph");

static PyObject *
add_edge(GraphObject *self, PyObject *args, PyObject *kw)
{
	GraphReturnID returnId;
	PyObject *u, *v, *uv;
	if (!PyArg_ParseTuple(args, "OOO:pygraph.Graph." __FUNCTION__, &u, &v, &uv))
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
		Py_UNREACHABLE();
	}
exit:
	return NULL;
}

PyDoc_STRVAR(_remove_edge__doc__,
"remove_edge(self, u : Object, v : Object, /) -> None\n"
"--\n"
"\n"
"Remove edge from u to v from graph");

static PyObject*
remove_edge(GraphObject* self, PyObject* args, PyObject* kw)
{
	GraphReturnID returnId;
	PyObject *u, *v;
	if (!PyArg_ParseTuple(args, "OO:pygraph.Graph." __FUNCTION__, &u, &v))
		goto exit;
	returnId = graphRemoveEdge(self->ob_graph, u, v);
	if (PyErr_Occurred())
		goto exit;
	switch (returnId) {
	case GRAPH_RETURN_OK:
		Py_RETURN_NONE;
	case GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX:
		PyErr_SetString(PyExc_RuntimeError,
			"Vertex not found in graph");
		break;
	case GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE:
		PyErr_SetString(PyExc_RuntimeError,
			"Edge not found in graph");
		break;
	}
exit:
	return NULL;
}

PyDoc_STRVAR(_get_edge__doc__,
"get_edge(self, u : Object, v : Object, /) -> Object\n"
"--\n"
"\n"
"Get edge that goes from u to v in graph");

static PyObject*
get_edge(GraphObject* self, PyObject* args, PyObject* kw)
{
	GraphReturnID returnId;
	PyObject *u, *v, *uv;
	if (!PyArg_ParseTuple(args, "OO:pygraph.Graph." __FUNCTION__, &u, &v))
		goto exit;
	returnId = graphGetEdge(self->ob_graph, u, v, &uv);
	if (PyErr_Occurred())
		goto exit;
	switch (returnId) {
	case GRAPH_RETURN_OK:
		Py_INCREF(uv);
		return uv;
	case GRAPH_RETURN_DOES_NOT_CONTAIN_VERTEX:
		PyErr_SetString(PyExc_RuntimeError,
			"Vertex not found in graph");
		break;
	case GRAPH_RETURN_DOES_NOT_CONTAIN_EDGE:
		PyErr_SetString(PyExc_RuntimeError,
			"Edge not found in graph");
		break;
	}
exit:
	return NULL;
}

static PyObject *
iter(GraphObject *self)
{
	GraphVertexIteratorObject *it;
	it = PyObject_GC_New(GraphVertexIteratorObject, &GraphIterType);
	if (it == NULL)
		return NULL;
	Py_INCREF(self);
	it->go = self;
	it->local_index = 0;
	PyObject_GC_Track(it);
	return (PyObject *) it;
}

PyMethodDef methods[] = {
	//
	// Metadata about graph
	//
	{
		"is_directed",
		(PyCFunction) is_directed,
		METH_NOARGS,
		_is_directed__doc__
	},
	//
	// Vertex data
	//
	{
		"contains_vertex",
		(PyCFunction) contains_vertex,
		METH_O,
		_contains_vertex__doc__
	},
	{
		"add_vertex",
		(PyCFunction) add_vertex,
		METH_O,
		_add_vertex__doc__
	},
	{
		"remove_vertex",
		(PyCFunction) remove_vertex,
		METH_O,
		_remove_vertex__doc__
	},
	{
		"get_vertex_count",
		(PyCFunction) get_vertex_count,
		METH_NOARGS,
		_get_vertex_count__doc__
	},
	//
	// Edge data
	//
	{
		"contains_edge",
		(PyCFunction) contains_edge,
		METH_VARARGS,
		_contains_edge__doc__
	},
	{
		"add_edge",
		(PyCFunction) add_edge,
		METH_VARARGS,
		_add_edge__doc__
	},
	{
		"remove_edge",
		(PyCFunction) remove_edge,
		METH_VARARGS,
		_remove_edge__doc__
	},
	{
		"get_edge",
		(PyCFunction) get_edge,
		METH_VARARGS,
		_get_edge__doc__
	},
	{
		NULL,
		NULL,
		0,
		NULL
	}
};

static PyObject *
GraphIterator_next(GraphVertexIteratorObject *it)
{
	GraphObject *go;
	Graph *pGraph;
	size_t size;
	_assert(it != NULL);
	go = it->go;
	if (go == NULL) {
		return NULL;
	}
	pGraph = go->ob_graph;
	if (graphGetNumberOfVertices(pGraph, &size))
		PyErr_BadInternalCall();
	if (it->local_index < size) {
		PyObject *obj;
		while (1) {
			if (go->global_offset < it->local_index) {
				if (graphGetNextVertex(pGraph, &obj))
					PyErr_BadInternalCall();
				++go->global_offset;
			} else if (go->global_offset > it->local_index) {
				if (graphGetPreviousVertex(pGraph, &obj))
					PyErr_BadInternalCall();
				--go->global_offset;
			} else {
				break;
			}
		}
		go->global_offset = ++it->local_index;
		if (graphGetNextVertex(pGraph, &obj))
			PyErr_BadInternalCall();
		Py_INCREF(obj);
		return obj;
	}
	go->global_offset = 0;
	it->go = NULL;
	Py_DECREF(go);
	return NULL;
}

static void
GraphIterator_dealloc(GraphVertexIteratorObject *it)
{
	PyObject_GC_UnTrack(it);
	Py_XDECREF(it->go);
	PyObject_GC_Del(it);
}

static PyTypeObject GraphType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "pygraph.Graph",                                  
	.tp_basicsize = sizeof(GraphObject),
	.tp_itemsize = 0,
	.tp_dealloc = (destructor) dealloc,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_doc = _init__doc__,
	.tp_iter = (getiterfunc) iter,
	.tp_methods = methods,
	.tp_init = (initproc) __init__,
	.tp_new = (newfunc) new,
};

static PyTypeObject GraphIterType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "pygraph.GraphIterator",
	.tp_basicsize = sizeof(GraphVertexIteratorObject),
	.tp_itemsize = 0,
	.tp_dealloc = (destructor) GraphIterator_dealloc,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_HAVE_GC,
	.tp_iter = PyObject_SelfIter,
	.tp_iternext = (iternextfunc) GraphIterator_next,
};

PyModuleDef pygraph_module = {
	PyModuleDef_HEAD_INIT,
	"pygraph",
};

PyMODINIT_FUNC
PyInit_pygraph(void)
{
	PyObject *m;
	Py_Initialize();
	if (PyType_Ready(&GraphType) < 0)
		return NULL;
	m = PyModule_Create(&pygraph_module);
	if (m == NULL)
		return NULL;
	Py_INCREF(&GraphType);
	if (PyModule_AddObject(m, "Graph", (PyObject *) &GraphType) < 0) {
		Py_DECREF(&GraphType);
		Py_DECREF(m);
		return NULL;
	}
	return m;
}
