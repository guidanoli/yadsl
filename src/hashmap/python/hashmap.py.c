#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <aa/pydefines.h>

#include <hashmap/hashmap.h>

typedef struct {
    PyObject_HEAD
    HashMap* ob_hash_map;
} HashMapObject;

static void
HashMap_decRefCallback(const char* key, void* value)
{
	Py_DECREF((PyObject *) value);
}

static PyObject *
HashMap_new(PyTypeObject *type, PyObject *args, PyObject *kw)
{
	HashMapObject *self;
	self = (HashMapObject *) type->tp_alloc(type, 0);
	if (self != NULL) {
		if (hashMapCreate(&self->ob_hash_map, 4, HashMap_decRefCallback)) {
			Py_DECREF(self);
			return NULL;
		}
	}
	return (PyObject *) self;
}

static void
HashMap_dealloc(HashMapObject *self)
{
	hashMapDestroy(self->ob_hash_map);
	Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyObject *
HashMap_add(HashMapObject *self, PyObject *args, PyObject *kw)
{
	PyObject *obj;
	const char* key;
	if (!PyArg_ParseTuple(args, "sO:HashMap.add", &key, &obj))
		return NULL;
	if (obj) {
		switch (hashMapAddEntry(self->ob_hash_map, key, (void*) obj)) {
		case HASHMAP_OK:
			Py_INCREF(obj);
			break;
		case HASHMAP_EXISTS:
			PyErr_SetString(PyExc_RuntimeError, "Already existing entry");
			return NULL; // Throws exception
		case HASHMAP_MEMORY:
			return PyErr_NoMemory();
		default:
			Py_UNREACHABLE();
		}
	}
	Py_RETURN_NONE;
}

static PyObject *
HashMap_remove(HashMapObject *self, PyObject *args, PyObject *kw)
{
	const char* key;
	if (!PyArg_ParseTuple(args, "s:HashMap.remove", &key))
		return NULL;
	switch (hashMapRemoveEntry(self->ob_hash_map, key)) {
	case HASHMAP_OK:
		break;
	case HASHMAP_DOESNT_EXIST:
		PyErr_SetString(PyExc_RuntimeError, "Entry doesn't exist");
		return NULL; // Throws exception
	default:
		Py_UNREACHABLE();
	}
	Py_RETURN_NONE;
}

static PyObject*
HashMap_get(HashMapObject* self, PyObject* args, PyObject* kw)
{
	const char* key;
	if (!PyArg_ParseTuple(args, "s:HashMap.get", &key))
		return NULL;
	PyObject* obj;
	switch (hashMapGetEntry(self->ob_hash_map, key, &obj)) {
	case HASHMAP_OK:
		Py_INCREF(obj);
		return obj;
	case HASHMAP_DOESNT_EXIST:
		PyErr_SetString(PyExc_RuntimeError, "Entry doesn't exist");
		return NULL; // Throws exception
	default:
		Py_UNREACHABLE();
	}
}

static PyMethodDef HashMap_methods[] = {
	{"add", (PyCFunction) HashMap_add, METH_VARARGS, "Add object"},
	{"remove", (PyCFunction) HashMap_remove, METH_VARARGS, "Remove and return object"},
	{"get", (PyCFunction) HashMap_get, METH_VARARGS, "Retrieve object"},
	{NULL}
};

static PyTypeObject HashMapType = {
	PyVarObject_HEAD_INIT(NULL, 0)
	.tp_name = "pyhashmap.HashMap",
	.tp_doc = "HashMap object",
	.tp_basicsize = sizeof(HashMapObject),
	.tp_itemsize = 0,
	.tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
	.tp_new = HashMap_new,
	.tp_dealloc = (destructor) HashMap_dealloc,
	.tp_methods = HashMap_methods,
	.tp_iter = PyObject_SelfIter,
};

PyModuleDef pyhash_map_module = {
	PyModuleDef_HEAD_INIT,
	"pyhashmap",
};

PyMODINIT_FUNC
PyInit_pyhashmap(void)
{
	PyObject *m;
	Py_Initialize();
	if (PyType_Ready(&HashMapType) < 0)
		return NULL;
	m = PyModule_Create(&pyhash_map_module);
	if (m == NULL)
		return NULL;
	Py_INCREF(&HashMapType);
	if (PyModule_AddObject(m, "HashMap", (PyObject *) &HashMapType) < 0) {
		Py_DECREF(&HashMapType);
		Py_DECREF(m);
		return NULL;
	}
	return m;
}
