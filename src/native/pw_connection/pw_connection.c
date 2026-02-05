#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <pipewire/pipewire.h>
#include "pw_connection.h"
#include "../pw_module.h"

static void PWConnection_dealloc(PWConnection *self) {
    Py_XDECREF(self->name);

    if (self->registry) {
        pw_thread_loop_lock(self->thread_loop);
        pw_proxy_destroy((struct pw_proxy *) self->registry);
        pw_thread_loop_unlock(self->thread_loop);
        self->registry = NULL;
    }

    if (self->thread_loop) {
        pw_thread_loop_stop(self->thread_loop);
    }

    // 2. Now it is safe to destroy objects without locking
    if (self->core) {
        pw_core_disconnect(self->core);
    }
    if (self->context) {
        pw_context_destroy(self->context);
    }

    // 3. Finally, destroy the loop itself
    if (self->thread_loop) {
        pw_thread_loop_destroy(self->thread_loop);
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *PWConnection_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PWConnection *self = (PWConnection *)type->tp_alloc(type, 0);
    if (self) {
        self->name = PyUnicode_FromString("default");
        if (!self->name) {
            Py_DECREF(self);
            return NULL;
        }
        self->thread_loop = NULL;
        self->context = NULL;
        self->core = NULL;
    }
    return (PyObject *)self;
}

static int PWConnection_init(PWConnection *self, PyObject *args, PyObject *kwds)
{
    static char *kwlist[] = {"name", NULL};
    PyObject *name = NULL;

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|U", kwlist, &name)) {
        return -1;  // TypeError already set by CPython
    }

    if (name) {
        Py_INCREF(name);
        Py_XDECREF(self->name);
        self->name = name;
    }

    const char *loop_name = PyUnicode_AsUTF8(self->name);
    if (!loop_name) {
        return -1;  // UnicodeError already set
    }

    /* If __init__ can be called more than once (rare, but possible),
       avoid leaking previous native resources. Easiest policy: forbid. */
    if (self->thread_loop || self->context || self->core) {
        PyErr_SetString(PyExc_RuntimeError, "PWConnection is already initialized");
        return -1;
    }

    self->thread_loop = pw_thread_loop_new(loop_name, NULL);
    if (!self->thread_loop) {
        PyErr_SetString(PyExc_RuntimeError, "pw_thread_loop_new failed");
        goto fail;
    }

    struct pw_loop *loop = pw_thread_loop_get_loop(self->thread_loop);
    if (!loop) {
        PyErr_SetString(PyExc_RuntimeError, "pw_thread_loop_get_loop failed");
        goto fail;
    }

    self->context = pw_context_new(loop, NULL, 0);
    if (!self->context) {
        PyErr_SetString(PyExc_RuntimeError, "pw_context_new failed");
        goto fail;
    }

    /* Optional: name the PipeWire client (recommended) */
    struct pw_properties *props =
        pw_properties_new(PW_KEY_APP_NAME, loop_name, NULL);
    if (!props) {
        PyErr_NoMemory();
        goto fail;
    }

    self->core = pw_context_connect(self->context, props, 0);
    /* pw_context_connect takes ownership of props on success in many APIs,
       but to be safe across versions: free only if connect failed. */
    if (!self->core) {
        //pw_properties_free(props);
        PyErr_SetString(PyExc_RuntimeError, "pw_context_connect failed");
        goto fail;
    }

    int res = pw_thread_loop_start(self->thread_loop);
    if (res < 0) {
        PyErr_SetString(PyExc_RuntimeError, "pw_thread_loop_start failed");
        goto fail;
    }

    self->registry = pw_core_get_registry(self->core, PW_VERSION_REGISTRY, 0);
    if (self->registry < 0) {
        PyErr_SetString(PyExc_RuntimeError, "pw_core_get_registry failed");
        goto fail;
    }

    return 0;

fail:
    /* Clean up partially constructed native state */
    if (self->registry) {
        pw_proxy_destroy((struct pw_proxy *) self->registry);
        self->registry = NULL;
    }
    if (self->core) {
        pw_core_disconnect(self->core);
        self->core = NULL;
    }
    if (self->context) {
        pw_context_destroy(self->context);
        self->context = NULL;
    }
    if (self->thread_loop) {
        pw_thread_loop_destroy(self->thread_loop);
        self->thread_loop = NULL;
    }
    return -1;
}

// --- Repr: So it looks nice in the Python console ---
PyObject *PWConnection_repr(PWConnection *self) {
    return PyUnicode_FromFormat("<PWConnection name=\"%U\">", self->name);
}

static PyMemberDef Custom_members[] = {
    {"name", Py_T_OBJECT_EX, offsetof(PWConnection, name), 0, "Connection name"},
    {NULL}  /* Sentinel */
};

static PyMethodDef PWConnection_methods[] = {
    {"create_object", (PyCFunction)PWConnection_create_object, METH_VARARGS | METH_KEYWORDS, "Create an object"},
    {"get_modules", (PyCFunction)PWConnection_get_modules, METH_NOARGS, "List modules"},
    {"get_devices", (PyCFunction)PWConnection_get_devices, METH_NOARGS, "List devices"},
    {"get_factories", (PyCFunction)PWConnection_get_factories, METH_NOARGS, "List factories"},
    {"get_nodes", (PyCFunction)PWConnection_get_nodes, METH_NOARGS, "List nodes"},
    {NULL}
};

PyTypeObject PWConnectionType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "_core.PWConnection",
    .tp_doc = "PipeWire Connection Object",
    .tp_basicsize = sizeof(PWConnection),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_repr = (reprfunc)PWConnection_repr,
    .tp_new = PWConnection_new,
    .tp_init = (initproc) PWConnection_init,
    .tp_dealloc = (destructor) PWConnection_dealloc,
    .tp_methods = PWConnection_methods,
    .tp_members = Custom_members
};
