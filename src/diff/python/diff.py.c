#include <Python.h>
#include <diff/diff.h>

PyDoc_STRVAR(_diff__doc__,
"diff(s1 : str, s2 : str, /) -> float\n"
"--\n"
"\n"
"Calculates the difference between two strings");

static PyObject *pydiff_diff(PyObject *self, PyObject *args)
{
	const char *s1, *s2;
	double result;
	if (!PyArg_ParseTuple(args, "ss", &s1, &s2))
		return NULL;
	result = aa_utils_diff(s1, s2);
	if (result == -1.0)
		return NULL;
	return PyFloat_FromDouble(result);
}

PyMethodDef method_table[] = {
	{
		"diff",
		(PyCFunction) pydiff_diff,
		METH_VARARGS,
		_diff__doc__
	},
	{NULL, NULL, 0, NULL}
};

PyModuleDef pydiff_module = {
	PyModuleDef_HEAD_INIT,
	"pydiff",
	"Python binding of the diff module",
	-1,
	method_table
};

PyMODINIT_FUNC PyInit_pydiff(void)
{
	Py_Initialize();
	return PyModule_Create(&pydiff_module);
}
