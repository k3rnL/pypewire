#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <pipewire/pipewire.h>
#include "module_discovery.h"
#include "device_discovery.h"

static void PWConnection_dealloc(PWConnection *self) {
    if (self->core) pw_core_disconnect(self->core);
    if (self->context) pw_context_destroy(self->context);
    if (self->loop) pw_main_loop_destroy(self->loop);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *PWConnection_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PWConnection *self = (PWConnection *)type->tp_alloc(type, 0);
    if (self) {
        self->loop = pw_main_loop_new(NULL);
        self->context = pw_context_new(pw_main_loop_get_loop(self->loop), NULL, 0);
        self->core = pw_context_connect(self->context, NULL, 0);
    }
    return (PyObject *)self;
}


static PyMethodDef PWConnection_methods[] = {
    {"get_modules", (PyCFunction)PWConnection_get_modules, METH_NOARGS, "List modules"},
    {"get_devices", (PyCFunction)PWConnection_get_devices, METH_NOARGS, "List devices"},
    {NULL}
};

static PyTypeObject PWConnectionType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "_core.PWConnection",
    .tp_doc = "PipeWire Connection Object",
    .tp_basicsize = sizeof(PWConnection),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PWConnection_new,
    .tp_dealloc = (destructor)PWConnection_dealloc,
    .tp_methods = PWConnection_methods,
};

static struct PyModuleDef module = { PyModuleDef_HEAD_INIT, "_core", NULL, -1, NULL };

PyMODINIT_FUNC PyInit__core(void) {
    pw_init(NULL, NULL);

    PyObject *m = PyModule_Create(&module);
    if (PyType_Ready(&PWConnectionType) < 0) return NULL;
    Py_INCREF(&PWConnectionType);
    PyModule_AddObject(m, "PWConnection", (PyObject *)&PWConnectionType);
    return m;
}