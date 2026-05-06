/*
/^-----^\   data: 2026-04-30
V  o o  V  plik: libs/funchook/_deps/distorm-src/python/python_module_init.c
 |  Y  |   autor: pupnoodle
  \ Q /
  / - \
  |    \
  |     \     )
  || (___\====
*/

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if PY_MAJOR_VERSION == 2
PyMODINIT_FUNC init_distorm3(void)
{
    (void)Py_InitModule("_distorm3", NULL);
}
#else
static struct PyModuleDef _distorm3_module = {
    PyModuleDef_HEAD_INIT,
    "_distorm3",
    NULL,
    -1,
    NULL,
};

PyMODINIT_FUNC PyInit__distorm3(void)
{
    PyObject *m;

    m = PyModule_Create(&_distorm3_module);
    if (m == NULL)
        return NULL;

    return m;
}
#endif
