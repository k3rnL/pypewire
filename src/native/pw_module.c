#include "pw_module.h"
#include "pw_connection/pw_connection.h"

PyObject *PWModule_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PWModule *self = (PWModule *)type->tp_alloc(type, 0);
    if (self) {
        self->name = Py_None;
        Py_INCREF(Py_None);

        self->args = Py_None;
        Py_INCREF(Py_None);

        self->parent = Py_None;
        Py_INCREF(Py_None);
    }
    return (PyObject *)self;
}

// --- Lifecycle: Dealloc ---
void PWModule_dealloc(PWModule *self) {
    // 1. Lock the thread loop before destroying the proxy!
    if (self->parent && self->proxy) {
        PWConnection *conn = (PWConnection *)self->parent;

        Py_BEGIN_ALLOW_THREADS
        pw_thread_loop_lock(conn->thread_loop);

        spa_hook_remove(&self->listener);
        pw_proxy_destroy((struct pw_proxy *)self->proxy);

        pw_thread_loop_unlock(conn->thread_loop);
        Py_END_ALLOW_THREADS
    }

    Py_XDECREF(self->name);
    Py_XDECREF(self->args);
    Py_XDECREF(self->parent);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

// --- Repr: So it looks nice in the Python console ---
PyObject *PWModule_repr(PWModule *self) {
    return PyUnicode_FromFormat("<PWModule id=%u, name=\"%U\">",
                                self->id, self->name);
}

// --- Methods: Example 'unload' ---
PyObject *PWModule_unload(PWModule *self, PyObject *Py_UNUSED(ignored)) {
    PWConnection *conn = (PWConnection *)self->parent;

    pw_thread_loop_lock(conn->thread_loop);
    // In PipeWire, you unload a module by destroying its proxy
    pw_proxy_destroy((struct pw_proxy *)self->proxy);
    self->proxy = NULL; // Mark as dead
    pw_thread_loop_unlock(conn->thread_loop);

    Py_RETURN_NONE;
}

static PyMethodDef PWModule_methods[] = {
    {"unload", (PyCFunction)PWModule_unload, METH_NOARGS, "Unload this module"},
    {NULL}
};

// --- Members: For read-only access in Python ---
static PyMemberDef PWModule_members[] = {
    {"id", Py_T_UINT, offsetof(PWModule, id), Py_READONLY, "Module ID"},
    {"name", Py_T_OBJECT_EX, offsetof(PWModule, name), Py_READONLY, "Module Name"},
    {"args", Py_T_OBJECT_EX, offsetof(PWModule, args), Py_READONLY, "Module Arguments"},
    {NULL}
};

PyTypeObject PWModuleType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "_core.PWModule",
    .tp_basicsize = sizeof(PWModule),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PWModule_new,
    .tp_dealloc = (destructor)PWModule_dealloc,
    .tp_repr = (reprfunc)PWModule_repr,
    .tp_methods = PWModule_methods,
    .tp_members = PWModule_members,
};