#ifndef MODULE_DISCOVERY_H
#define MODULE_DISCOVERY_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <pipewire/pipewire.h>
#include "pw_connection.h"

// Function to get modules from PipeWire
PyObject *PWConnection_get_modules(PWConnection *self, PyObject *Py_UNUSED(ignored));

#endif // MODULE_DISCOVERY_H
