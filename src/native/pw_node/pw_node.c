//
// Created by edaniel on 2/5/26.
//

#include "pw_node.h"


PyObject *PWNode_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PWNode *self = (PWNode *) type->tp_alloc(type, 0);
    if (self) {
        self->error = Py_None;
        Py_INCREF(Py_None);
    }
    return (PyObject *) self;
}

// --- Lifecycle: Dealloc ---
void PWNode_dealloc(PWNode *self) {
    // 1. Lock the thread loop before destroying the proxy!
    if (self->connection && self->proxy) {
        const PWConnection *conn = (PWConnection *) self->connection;

        Py_BEGIN_ALLOW_THREADS
            pw_thread_loop_lock(conn->thread_loop);

            spa_hook_remove(&self->listener);
            pw_proxy_destroy((struct pw_proxy *) self->proxy);

            pw_thread_loop_unlock(conn->thread_loop);
        Py_END_ALLOW_THREADS
    }

    Py_XDECREF(self->error);
    Py_XDECREF(self->connection);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

// --- Repr: So it looks nice in the Python console ---
PyObject *PWNode_repr(const PWNode *self) {
    const char *state = pw_node_state_as_string(self->state);

    return PyUnicode_FromFormat("<PWNode id=%u, state=\"%s\"",
                                self->id, state);
}

PyObject *PWNode_state(const PWNode *self, PyObject *Py_UNUSED(ignored)) {
    const char *string = pw_node_state_as_string(self->state);

    return PyUnicode_FromString(string);
}

static PyMethodDef PWNode_methods[] = {
    {"state", (PyCFunction) PWNode_state, METH_NOARGS, "State as a string"},
    {NULL}
};

static PyMemberDef PWNode_members[] = {
    {"id", Py_T_UINT, offsetof(PWNode, id), Py_READONLY, "Node ID"},
    {"max_input_ports", Py_T_UINT, offsetof(PWNode, max_input_ports), Py_READONLY, "Maximum input ports of this node"},
    {
        "max_output_ports", Py_T_UINT, offsetof(PWNode, max_output_ports), Py_READONLY,
        "Maximum output ports of this node"
    },
    {"n_input_ports", Py_T_UINT, offsetof(PWNode, n_input_ports), Py_READONLY, "Number of input ports of this node"},
    {"n_output_ports", Py_T_UINT, offsetof(PWNode, n_output_ports), Py_READONLY, "Number of output ports of this node"},
    {"state_code", Py_T_BYTE, offsetof(PWNode, state), Py_READONLY, "The state of this node"},
    {"error", Py_T_OBJECT_EX, offsetof(PWNode, error), Py_READONLY, "The error string if applicable"},
    {NULL}
};

PyTypeObject PWNodeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "_core.PWNode",
    .tp_basicsize = sizeof(PWNode),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PWNode_new,
    .tp_dealloc = (destructor) PWNode_dealloc,
    .tp_repr = (reprfunc) PWNode_repr,
    .tp_members = PWNode_members,
    .tp_methods = PWNode_methods,
};
