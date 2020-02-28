#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <common/pydefines.h>

#include "avl.h"

PyModuleDef pyavl_module = {
	PyModuleDef_HEAD_INIT,
	"pyavl",
};

PyMODINIT_FUNC
PyInit_pyavl(void)
{
	Py_Initialize();
	return PyModule_Create(&pyavl_module);
}
