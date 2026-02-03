#ifndef MODULE_DISCOVERY_H
#define MODULE_DISCOVERY_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <pipewire/pipewire.h>

// This struct will be managed as a Python Object
typedef struct {
    PyObject_HEAD
    struct pw_main_loop *loop;
    struct pw_context *context;
    struct pw_core *core;
} PWConnection;

// Helper struct to track temporary proxies
struct proxy_node {
    struct pw_proxy *proxy;
    struct spa_hook hook;
    struct proxy_node *next;
};

struct registry_data {
    PyObject *list;
    struct pw_main_loop *loop;
    struct pw_core *core;
    struct proxy_node *proxies; // Linked list of proxies to clean up
    int sync_seq_1;             // First sync (get globals)
    int sync_seq_2;             // Second sync (get details)
};

// Function to get modules from PipeWire
PyObject *PWConnection_get_modules(PWConnection *self, PyObject *Py_UNUSED(ignored));

#endif // MODULE_DISCOVERY_H
