#ifndef PW_CONNECTION_H
#define PW_CONNECTION_H

#include <Python.h>
#include <pipewire/pipewire.h>

// This struct will be managed as a Python Object
typedef struct {
    PyObject_HEAD
    struct pw_main_loop *loop;
    struct pw_context *context;
    struct pw_core *core;
} PWConnection;

#endif
