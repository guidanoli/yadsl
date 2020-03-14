#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "pydefines.h"

#include "heap.h"
#include "memdb.h"

//
// Objects
//

typedef struct
{
	PyObject_HEAD
	Heap *ob_heap;
	PyObject *ob_func;
	unsigned char lock : 1;
} HeapObject;

//
// Exceptions
//

static PyObject *PyExc_Empty = NULL;
static PyObject *PyExc_Full = NULL;
static PyObject *PyExc_Lock = NULL;
static PyObject *PyExc_Shrink = NULL;

_DEFINE_EXCEPTION_METADATA()

static struct _exception_metadata exceptions[] = {
	//
	// Exception definitions
	//
	{
		&PyExc_Empty,
		"pyheap.Empty",
		"Empty",
		"Heap is empty."
	},
	{
		&PyExc_Full,
		"pyheap.Full",
		"Full",
		"Heap is full. Try resizing it."
	},
	{
		&PyExc_Lock,
		"pyheap.Lock",
		"Lock",
		"Heap is locked. Illegal operation detected."
	},
	{
		&PyExc_Shrink,
		"pyheap.Shrink",
		"Shrink",
		"Cannot resize heap to smaller than "
		"the current number of objects in it."
	},
	//
	// Sentinel
	//
	{
		NULL,
		NULL,
		NULL,
	},
};

_DEFINE_EXCEPTION_FUNCTIONS(Heap, exceptions)

//
// Callbacks
//

static void
decRefCallback(void *item)
{
	Py_XDECREF((PyObject *) item);
}

static int
cmpCallback(void *obj1, void *obj2, void *arg)
{
	int result;
	HeapObject *ho;
	PyObject *callable;
	ho = (HeapObject *) arg;
	if (callable = ho->ob_func) {
		PyObject *args, *resultObj;
		if ((args = PyTuple_Pack(2, obj1, obj2)) == NULL) {
			PyErr_SetString(PyExc_MemoryError,
				"Could not create internal tuple.");
			return 0;
		}
		ho->lock = 1;
		resultObj = PyObject_CallObject(callable, args);
		ho->lock = 0;
		Py_DECREF(args);
		if (resultObj == NULL) {
			if (!PyErr_Occurred())
				PyErr_SetString(PyExc_RuntimeError,
					"An unspecified error occurred during callback.");
			return 0;
		}
		result = PyObject_IsTrue(resultObj);
		Py_DECREF(resultObj);
	} else {
		Py_XINCREF(obj1);
		ho->lock = 1;
		result = PyObject_RichCompareBool(obj1, obj2, Py_LT);
		ho->lock = 0;
		Py_XDECREF(obj1);
	}
	return result;
}

//
// Method definitions
//

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
		if (!PyCallable_Check(callbackObj)) {
			PyErr_SetString(PyExc_TypeError,
				"f should be a callable object");
			return -1;
		}
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
		PyErr_SetString(PyExc_MemoryError, "Could not create heap");
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
	_memdb_dump();
	Py_XDECREF(self->ob_func);
	Py_TYPE(self)->tp_free((PyObject *) self);
}

PyDoc_STRVAR(_Heap_insert__doc__,
"insert(obj : Object) -> None\n"
"--\n"
"\n"
"Insert object in heap.");

static PyObject *
Heap_insert(HeapObject *self, PyObject *obj)
{
	if (self->lock) {
		_Heap_throw_error(PyExc_Lock);
		goto exit;
	}
	switch (heapInsert(self->ob_heap, obj)) {
	case HEAP_RETURN_OK:
		Py_INCREF(obj);
		if (PyErr_Occurred())
			goto exit;
		Py_RETURN_NONE;
	case HEAP_RETURN_INVALID_PARAMETER:
		PyErr_BadInternalCall();
		break;
	case HEAP_RETURN_FULL:
		_Heap_throw_error(PyExc_Full);
		break;
	default:
		Py_UNREACHABLE();
	}
exit:
	return NULL;
}

PyDoc_STRVAR(_Heap_extract__doc__,
"extract() -> Object\n"
"--\n"
"\n"
"Extract object from heap.");

static PyObject *
Heap_extract(HeapObject *self, PyObject *Py_UNUSED(ignored))
{
	HeapReturnID returnId;
	PyObject *obj = NULL;
	if (self->lock) {
		_Heap_throw_error(PyExc_Lock);
		goto exit;
	}
	returnId = heapExtract(self->ob_heap, &obj);
	if (PyErr_Occurred()) {
		Py_XDECREF(obj);
		goto exit;
	}
	switch (returnId) {
	case HEAP_RETURN_OK:
		// Borrow reference
		return obj;
	case HEAP_RETURN_INVALID_PARAMETER:
		PyErr_BadInternalCall();
		break;
	case HEAP_RETURN_EMPTY:
		_Heap_throw_error(PyExc_Empty);
		break;
	default:
		Py_UNREACHABLE();
	}
exit:
	return NULL;
}

PyDoc_STRVAR(_Heap_size__doc__,
"size() -> int\n"
"--\n"
"\n"
"Get heap size.");

static PyObject *
Heap_size(HeapObject *self, PyObject *Py_UNUSED(ignored))
{
	size_t size;
	switch (heapGetSize(self->ob_heap, &size)) {
	case HEAP_RETURN_OK:
		return PyLong_FromSize_t(size);
	case HEAP_RETURN_INVALID_PARAMETER:
		PyErr_BadInternalCall();
		break;
	default:
		Py_UNREACHABLE();
	}
	return NULL;
}

PyDoc_STRVAR(_Heap_resize__doc__,
"resize(size : int) -> None\n"
"--\n"
"\n"
"Resize heap.");

static PyObject *
Heap_resize(HeapObject *self, PyObject *obj)
{
	size_t size;
	if (self->lock) {
		_Heap_throw_error(PyExc_Lock);
		goto exit;
	}
	if (!PyLong_Check(obj)) {
		PyErr_SetString(PyExc_TypeError,
			"parameter size should be an integer");
		goto exit;
	}
	size = PyLong_AsSize_t(obj);
	if (size == ((size_t) -1) && PyErr_Occurred())
		goto exit;
	if (size == 0) {
		PyErr_SetString(PyExc_ValueError, "size must not be zero");
		goto exit;
	}
	switch (heapResize(self->ob_heap, size)) {
	case HEAP_RETURN_OK:
		Py_RETURN_NONE;
	case HEAP_RETURN_INVALID_PARAMETER:
		PyErr_BadInternalCall();
		break;
	case HEAP_RETURN_SHRINK:
		_Heap_throw_error(PyExc_Shrink);
		break;
	case HEAP_RETURN_MEMORY:
		PyErr_SetString(PyExc_MemoryError,
			"Could not resize heap due to lack of memory");
		break;
	default:
		Py_UNREACHABLE();
	}
exit:
	return NULL;
}

//
// Method table
//

PyMethodDef Heap_methods[] = {
	//
	// Objects
	//
	{
		"insert",
		(PyCFunction) Heap_insert,
		METH_O,
		_Heap_insert__doc__
	},
	{
		"extract",
		(PyCFunction) Heap_extract,
		METH_NOARGS,
		_Heap_extract__doc__
	},
	//
	// Size management
	//
	{
		"size",
		(PyCFunction) Heap_size,
		METH_NOARGS,
		_Heap_size__doc__
	},
	{
		"resize",
		(PyCFunction) Heap_resize,
		METH_O,
		_Heap_resize__doc__
	},
	//
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

//
// Types
//

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

//
// Module
//

PyModuleDef pyheap_module = {
	PyModuleDef_HEAD_INIT,
	"pyheap",
};

PyMODINIT_FUNC
PyInit_pyheap(void)
{
	PyObject *m;
	struct _exception_metadata *_exc;
	Py_Initialize();
	if (PyType_Ready(&HeapType) < 0)
		return NULL;
	m = PyModule_Create(&pyheap_module);
	if (m == NULL)
		return NULL;
	_INIT_EXCEPTION_OBJECTS(m, _exc)
	Py_INCREF(&HeapType);
	if (PyModule_AddObject(m, "Heap", (PyObject *) &HeapType) < 0) {
		Py_DECREF(&HeapType);
		Py_DECREF(m);
		return NULL;
	}
	return m;
}