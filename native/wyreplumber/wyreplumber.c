//
// Created by edaniel on 2/5/26.
//

#include <Python.h>
#include <wp/wp.h>

static struct PyModuleDef module = { PyModuleDef_HEAD_INIT, "_core", NULL, -1, NULL };

PyMODINIT_FUNC PyInit__core(void) {
    wp_init(WP_INIT_ALL);

    PyObject *m = PyModule_Create(&module);

    // if (PyType_Ready(&PWConnectionType) < 0) return NULL;
    // Py_INCREF(&PWConnectionType);
    // PyModule_AddObject(m, "PWConnection", (PyObject *)&PWConnectionType);

    return m;
}