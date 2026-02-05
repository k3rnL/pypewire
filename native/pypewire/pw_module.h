#ifndef PW_MODULE_H
#define PW_MODULE_H

#include <Python.h>
#include <pipewire/pipewire.h>

typedef struct {
    PyObject_HEAD
    struct pw_module *proxy;
    struct spa_hook listener;
    PyObject *parent;      // Reference to PWConnection
    uint32_t id;
    PyObject *name;        // Storing as Python strings for easy access
    PyObject *args;
} PWModule;

extern PyTypeObject PWModuleType;

#endif