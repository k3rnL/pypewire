#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <pipewire/pipewire.h>
#include <stdio.h>

// This struct will be managed as a Python Object
typedef struct {
    PyObject_HEAD
    struct pw_main_loop *loop;
    struct pw_context *context;
    struct pw_core *core;
} PWConnection;

static void PWConnection_dealloc(PWConnection *self) {
    if (self->core) pw_core_disconnect(self->core);
    if (self->context) pw_context_destroy(self->context);
    if (self->loop) pw_main_loop_destroy(self->loop);
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *PWConnection_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    PWConnection *self = (PWConnection *)type->tp_alloc(type, 0);
    if (self) {
        self->loop = pw_main_loop_new(NULL);
        self->context = pw_context_new(pw_main_loop_get_loop(self->loop), NULL, 0);
        self->core = pw_context_connect(self->context, NULL, 0);
    }
    return (PyObject *)self;
}

// Internal helper for the Registry
struct registry_data {
    PyObject *list;
    struct pw_main_loop *loop;
    int sync_seq;
};

static void on_global(void *data, uint32_t id, uint32_t permissions, const char *type, uint32_t version, const struct spa_dict *props) {
    struct registry_data *rd = data;
    if (strcmp(type, "PipeWire:Interface:Module") == 0) {
        PyObject *d = PyDict_New();
        PyDict_SetItemString(d, "id", PyLong_FromUnsignedLong(id));
        const char *name = spa_dict_lookup(props, "module.name");
        PyDict_SetItemString(d, "name", PyUnicode_FromString(name ? name : "unknown"));
        PyList_Append(rd->list, d);
        Py_DECREF(d);
    }
}

static void on_done(void *data, uint32_t id, int seq) {
    struct registry_data *rd = data;
    if (seq == rd->sync_seq) pw_main_loop_quit(rd->loop);
}

static PyObject *PWConnection_get_modules(PWConnection *self, PyObject *Py_UNUSED(ignored)) {
    struct registry_data rd = { .list = PyList_New(0), .loop = self->loop };
    struct pw_registry *registry = pw_core_get_registry(self->core, PW_VERSION_REGISTRY, 0);

    struct spa_hook reg_listener, core_listener;
    static const struct pw_registry_events reg_events = { .version = PW_VERSION_REGISTRY_EVENTS, .global = on_global };
    static const struct pw_core_events core_events = { .version = PW_VERSION_CORE_EVENTS, .done = on_done };

    pw_registry_add_listener(registry, &reg_listener, &reg_events, &rd);
    pw_core_add_listener(self->core, &core_listener, &core_events, &rd);
    rd.sync_seq = pw_core_sync(self->core, PW_ID_CORE, 0);

    pw_main_loop_run(self->loop);

    spa_hook_remove(&reg_listener);
    spa_hook_remove(&core_listener);
    pw_proxy_destroy((struct pw_proxy*)registry);

    return rd.list;
}

static PyMethodDef PWConnection_methods[] = {
    {"get_modules", (PyCFunction)PWConnection_get_modules, METH_NOARGS, "List modules"},
    {NULL}
};

static PyTypeObject PWConnectionType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "_core.PWConnection",
    .tp_doc = "PipeWire Connection Object",
    .tp_basicsize = sizeof(PWConnection),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PWConnection_new,
    .tp_dealloc = (destructor)PWConnection_dealloc,
    .tp_methods = PWConnection_methods,
};

static struct PyModuleDef module = { PyModuleDef_HEAD_INIT, "_core", NULL, -1, NULL };

PyMODINIT_FUNC PyInit__core(void) {
    pw_init(NULL, NULL);

    PyObject *m = PyModule_Create(&module);
    if (PyType_Ready(&PWConnectionType) < 0) return NULL;
    Py_INCREF(&PWConnectionType);
    PyModule_AddObject(m, "PWConnection", (PyObject *)&PWConnectionType);
    return m;
}