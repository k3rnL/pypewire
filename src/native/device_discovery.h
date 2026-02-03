#ifndef DEVICE_DISCOVERY_H
#define DEVICE_DISCOVERY_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <pipewire/pipewire.h>
#include "pw_connection.h"

// Function to get devices from PipeWire
PyObject *PWConnection_get_devices(PWConnection *self, PyObject *Py_UNUSED(ignored));

#endif // DEVICE_DISCOVERY_H
