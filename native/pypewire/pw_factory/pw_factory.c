#include "pw_factory.h"

PyObject *PWFactory_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PWFactory *self = (PWFactory *)type->tp_alloc(type, 0);
    if (self) {
        self->name = Py_None;
        Py_INCREF(Py_None);

        self->type = Py_None;
        Py_INCREF(Py_None);
    }
    return (PyObject *)self;
}

// --- Lifecycle: Dealloc ---
void PWFactory_dealloc(PWFactory *self) {
    // 1. Lock the thread loop before destroying the proxy!
    if (self->connection && self->proxy) {
        const PWConnection *conn = (PWConnection *)self->connection;

        Py_BEGIN_ALLOW_THREADS
        pw_thread_loop_lock(conn->thread_loop);

        spa_hook_remove(&self->listener);
        pw_proxy_destroy((struct pw_proxy *)self->proxy);

        pw_thread_loop_unlock(conn->thread_loop);
        Py_END_ALLOW_THREADS
    }

    Py_XDECREF(self->name);
    Py_XDECREF(self->type);
    Py_XDECREF(self->connection);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// --- Repr: So it looks nice in the Python console ---
PyObject *PWFactory_repr(const PWFactory *self) {
    return PyUnicode_FromFormat("<PWFactory id=%u, name=\"%U\", type=\"%U\">",
                                self->id, self->name, self->type);
}

static PyMemberDef PWFactory_members[] = {
    {"id", Py_T_UINT, offsetof(PWFactory, id), Py_READONLY, "Factory ID"},
    {"name", Py_T_OBJECT_EX, offsetof(PWFactory, name), Py_READONLY, "Factory Name"},
    {"type", Py_T_OBJECT_EX, offsetof(PWFactory, type), Py_READONLY, "Factory type"},
    {"version", Py_T_UINT, offsetof(PWFactory, version), Py_READONLY, "Factory version"},
    {NULL}
};

PyTypeObject PWFactoryType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "_core.PWFactory",
    .tp_basicsize = sizeof(PWFactory),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PWFactory_new,
    .tp_dealloc = (destructor) PWFactory_dealloc,
    .tp_repr = (reprfunc) PWFactory_repr,
    .tp_members = PWFactory_members,
};
