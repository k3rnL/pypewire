//
// Created by edaniel on 2/5/26.
//

#include <Python.h>
#include <wp/wp.h>
#include "wp_connection/wp_connection.h"

static struct PyModuleDef module = { PyModuleDef_HEAD_INIT, "_core", NULL, -1, NULL };

PyMODINIT_FUNC PyInit__core(void) {
    wp_init(WP_INIT_ALL);

    // // Initialize Python threading support for GIL operations
    // if (!PyEval_ThreadsInitialized()) {
    //     PyEval_InitThreads();
    // }

    PyObject *m = PyModule_Create(&module);
    if (!m) return NULL;

    if (PyType_Ready(&WPConnectionType) < 0) return NULL;
    Py_INCREF(&WPConnectionType);
    if (PyModule_AddObject(m, "WPConnection", (PyObject *)&WPConnectionType) < 0) {
        Py_DECREF(&WPConnectionType);
        Py_DECREF(m);
        return NULL;
    }

    return m;
}