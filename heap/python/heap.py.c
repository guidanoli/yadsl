#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <common/pydefines.h>

#include "heap.h"

#ifdef _DEBUG
#include "memdb.h"
#endif

typedef struct
{
	PyObject_HEAD
	Heap *ob_heap;
	PyObject *ob_func;
	int lock : 1;
} HeapObject;

static PyObject *
Heap_new(PyTypeObject *type, PyObject *args, PyObject *kw)
{
	HeapObject *self;
	self = (HeapObject *) type->tp_alloc(type, 0);
	if (self != NULL) {
		self->ob_heap = NULL;
		self->ob_func = NULL;
		self->lock = 0;
	}
	return (PyObject *) self;
}

static void
decRefCallback(void *item)
{
	Py_XDECREF((PyObject *) item);
}

static int
cmpCallback(void *obj1, void *obj2, void *arg)
{
	HeapObject *ho;
	PyObject *callable;
	ho = (HeapObject *) arg;
	if (callable = ho->ob_func) {
		int result;
		PyObject *args, *resultObj;
		if ((args = PyTuple_Pack(2, obj1, obj2)) == NULL) {
			PyErr_SetNone(PyExc_MemoryError);
			return 0;
		}
		ho->lock = 1;
		resultObj = PyObject_CallObject(callable, args);
		ho->lock = 0;
		Py_DECREF(args);
		if (resultObj == NULL) {
			PyErr_SetNone(PyExc_RuntimeError);
			return 0;
		}
		result = PyObject_IsTrue(resultObj);
		Py_DECREF(resultObj);
		return result;
	} else {
		return obj1 < obj2;
	}
}

PyDoc_STRVAR(_Heap_init__doc__,
"Heap(/, f=None, size=15)\n"
"--\n"
"\n"
"Python heap data structure.");

static int
Heap_init(HeapObject *self, PyObject *args, PyObject *kw)
{
	PyObject *initialSizeObj = NULL, *callbackObj = NULL;
	size_t initialSize = 15;
	static char *keywords[] = { "f", "size", NULL };
	if (!PyArg_ParseTupleAndKeywords(args, kw,
		"|OO!:pyheap.Heap.__init__", keywords,
		&callbackObj, &PyLong_Type, &initialSizeObj))
		return -1;
	if (callbackObj != NULL) {
		if (!PyCallable_Check(callbackObj))
			callbackObj = NULL;
	}
	if (initialSizeObj != NULL) {
		initialSize = PyLong_AsSize_t(initialSizeObj);
		if (initialSize == ((size_t) -1) && PyErr_Occurred())
			return -1;
		if (initialSize == 0) {
			PyErr_SetString(PyExc_ValueError, "size must not be zero");
			return -1;
		}
	}
	switch (heapCreate(&self->ob_heap, initialSize,
		cmpCallback, decRefCallback, self)) {
	case HEAP_RETURN_OK:
		break;
	case HEAP_RETURN_MEMORY:
		return -1;
	case HEAP_RETURN_INVALID_PARAMETER:
		PyErr_BadInternalCall();
		return -1;
	default:
		Py_UNREACHABLE();
	}
	self->ob_func = callbackObj;
	Py_XINCREF(callbackObj);
	return 0;
}

static void
Heap_dealloc(HeapObject *self)
{
	if (self->ob_heap)
		heapDestroy(self->ob_heap);
#ifdef _DEBUG
	printf("MEMDB: %zu items in list\n", _memdb_list_size());
#endif
	Py_XDECREF(self->ob_func);
	Py_TYPE(self)->tp_free((PyObject *) self);
}

PyMethodDef Heap_methods[] = {
	//
	// Sentinel
	//
	{
		NULL,
		NULL,
		0,
		NULL
	}
};

static PyTypeObject HeapType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "pyheap.Heap",
	.tp_basicsize = sizeof(HeapObject),
	.tp_itemsize = 0,
	.tp_dealloc = (destructor) Heap_dealloc,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_doc = _Heap_init__doc__,
	.tp_methods = Heap_methods,
	.tp_init = (initproc) Heap_init,
	.tp_new = (newfunc) Heap_new,
};

PyModuleDef pyheap_module = {
	PyModuleDef_HEAD_INIT,
	"pyheap",
};

PyMODINIT_FUNC
PyInit_pyheap(void)
{
	PyObject *m;
	Py_Initialize();
	if (PyType_Ready(&HeapType) < 0)
		return NULL;
	m = PyModule_Create(&pyheap_module);
	if (m == NULL)
		return NULL;
	Py_INCREF(&HeapType);
	if (PyModule_AddObject(m, "Heap", (PyObject *) &HeapType) < 0) {
		Py_DECREF(&HeapType);
		Py_DECREF(m);
		return NULL;
	}
	return m;
}