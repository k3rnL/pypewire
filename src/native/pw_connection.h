#ifndef PW_CONNECTION_H
#define PW_CONNECTION_H

#include <Python.h>
#include <pipewire/pipewire.h>

// This struct will be managed as a Python Object
typedef struct {
    PyObject_HEAD
    PyObject *name;
    struct pw_thread_loop *thread_loop;
    struct pw_context *context;
    struct pw_core *core;
    struct pw_registry *registry;
} PWConnection;

#endif
