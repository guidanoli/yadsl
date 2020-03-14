#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "pydefines.h"

#include "stack.h"

PyModuleDef pystack_module = {
	PyModuleDef_HEAD_INIT,
	"pystack",
};

PyMODINIT_FUNC
PyInit_pystack(void)
{
	Py_Initialize();
	return PyModule_Create(&pystack_module);
}
