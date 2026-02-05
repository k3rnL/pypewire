#ifndef PW_CONNECTION_H
#define PW_CONNECTION_H

#include <Python.h>
#include <pipewire/pipewire.h>

typedef struct {
    PyObject_HEAD
    PyObject *name;
    struct pw_thread_loop *thread_loop;
    struct pw_context *context;
    struct pw_core *core;
    struct pw_registry *registry;
} PWConnection;

extern PyTypeObject PWConnectionType;

PyObject *PWConnection_create_object(PWConnection *self, PyObject *args, PyObject *kw);
//PyObject *PWConnection_load_module(PWConnection *self, PyObject *args);
PyObject *PWConnection_get_modules(PWConnection *self, PyObject *Py_UNUSED(ignored));
PyObject *PWConnection_get_devices(PWConnection *self, PyObject *Py_UNUSED(ignored));
PyObject *PWConnection_get_factories(PWConnection *self, PyObject *Py_UNUSED(ignored));
PyObject *PWConnection_get_nodes(PWConnection *self, PyObject *Py_UNUSED(ignored));

#endif