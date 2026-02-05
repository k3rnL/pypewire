#ifndef PW_FACTORY_H
#define PW_FACTORY_H

#include <Python.h>
#include <pipewire/pipewire.h>

#include "../pw_connection/pw_connection.h"

typedef struct {
    PyObject_HEAD
    struct pw_factory *proxy;
    struct spa_hook listener;
    PWConnection *connection;
    uint32_t id;
    PyObject *name;
    PyObject *type;
    uint32_t version;
} PWFactory;

extern PyTypeObject PWFactoryType;

#endif