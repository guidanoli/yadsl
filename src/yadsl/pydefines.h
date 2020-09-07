#ifndef __YADSL_PYDEFINES_H__
#define __YADSL_PYDEFINES_H__

/**
 * Python definitions header file
 * Make sure to include <Python.h> before
*/

/* Add backwards compatibility with Python C API < v.3.7 */
#if defined(PY_MAJOR_VERSION) && ((PY_MAJOR_VERSION < 3) || \
    ((PY_MAJOR_VERSION == 3) && (PY_MINOR_VERSION < 7)))
#pragma message("Py_UNREACHABLE() was not found, maybe your Python is of " \
"version previous to v3.7. It will be then defined as a macro for abort().")
#define Py_UNREACHABLE() abort()
#endif

/* Define exception metadata struct */
#define YADSL_PYDEFINES_EXCEPTION_METADATA() \
struct _exception_metadata \
{ \
	PyObject **obj;        /* reference to exception object */ \
	const char *fullname;  /* exception name with module prefix */ \
	const char *name;      /* exception name */  \
	const char *doc;       /* exception documentation */ \
};

/**
 * Define exception handling functions
 * m is a prefix for your module
 * _exc_table is an array of struct _exception_metadata
*/
#define YADSL_PYDEFINES_EXCEPTION_FUNCTIONS(m, _exc_table) \
static const char * \
_ ## m ## _get_exception_string(PyObject *exc) \
{ \
	struct _exception_metadata *_exc; \
	for (_exc = _exc_table; _exc->obj; ++_exc) { \
		if (*_exc->obj == exc && *_exc->doc) \
			return _exc->doc; \
	} \
	return NULL; \
} \
static void \
_ ## m ## _throw_error(PyObject *exc) \
{ \
	const char *doc = _ ## m ## _get_exception_string(exc); \
	if (doc) { \
		PyErr_SetString(exc, doc); \
	} else { \
		PyErr_SetNone(exc); \
	} \
}

/**
 * Initialize exception objects in module
 * m is your module object
 * _exc_ptr is an already declated struct _exception_metadata pointer
*/
#define YADSL_PYDEFINES_INIT_EXCEPTION_OBJECTS(m, _exc_ptr) \
for (_exc_ptr = exceptions; _exc_ptr->obj; ++_exc_ptr) { \
	*_exc_ptr->obj = PyErr_NewExceptionWithDoc( \
		_exc_ptr->fullname, /* name */ \
		_exc_ptr->doc,      /* doc */ \
		NULL,               /* base */ \
		NULL);              /* dict */ \
	if (*_exc_ptr->obj == NULL) \
		return NULL; \
	Py_INCREF(*_exc_ptr->obj); \
	if (PyModule_AddObject(m, _exc_ptr->name, *_exc_ptr->obj) < 0) \
		return NULL; \
}

#endif
