#include "pw_connection.h"

typedef struct {
    struct pw_thread_loop *loop;
    bool done;
    int seq;
    int err_res;
    char err_msg[256];
    uint32_t bound_id;
} create_state;

static void on_proxy_bound(void *data, uint32_t global_id)
{
    printf("proxy bound id=%u\n", global_id);
    create_state *st = (create_state *)data;
    st->bound_id = global_id;
}

static void on_proxy_done(void *data, int seq)
{
    printf("proxy done\n");
    create_state *st = (create_state *)data;
    if (seq == st->seq) {
        st->done = true;
        // 2. SIGNAL the loop to wake up the main thread!
        pw_thread_loop_signal(st->loop, false);
    }
}

static void on_proxy_error(void *data, int seq, int res, const char *message)
{
    printf("proxy error: %d (%s)\n", res, message);
    create_state *st = (create_state *)data;
    (void)seq;
    st->err_res = res;
    st->done = true;
    if (message) {
        snprintf(st->err_msg, sizeof(st->err_msg), "%s", message);
    } else {
        snprintf(st->err_msg, sizeof(st->err_msg), "unknown error");
    }
    // 2. SIGNAL the loop here too
    pw_thread_loop_signal(st->loop, false);
}

static const struct pw_proxy_events create_proxy_events = {
    .version = PW_VERSION_PROXY_EVENTS,
    .bound = on_proxy_bound,
    .done = on_proxy_done,
    .error = on_proxy_error,
};

/**
 * Python signature:
 *   create_object(factory_name: str, type_name: str, version: int, props: dict[str, Any] | None = None) -> PWModule
 */
PyObject *PWConnection_create_object(PWConnection *self, PyObject *args, PyObject *kw)
{
    const char *factory_name = NULL;
    const char *type_name = NULL;
    unsigned int version = 0;
    PyObject *props_obj = Py_None;

    static char *kwlist[] = {"factory_name", "type_name", "version", "props", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kw, "ssI|O", kwlist,
                                     &factory_name, &type_name, &version, &props_obj)) {
        return NULL;
    }

    // Build pw_properties from Python dict (or empty)
    struct pw_properties *p = pw_properties_new(NULL, NULL);
    if (!p) {
        return PyErr_NoMemory();
    }

    if (props_obj != Py_None) {
        if (!PyDict_Check(props_obj)) {
            pw_properties_free(p);
            PyErr_SetString(PyExc_TypeError, "props must be a dict or None");
            return NULL;
        }

        PyObject *key, *val;
        Py_ssize_t pos = 0;
        while (PyDict_Next(props_obj, &pos, &key, &val)) {
            PyObject *k_str = PyObject_Str(key);
            if (!k_str) { pw_properties_free(p); return NULL; }
            const char *k = PyUnicode_AsUTF8(k_str);
            if (!k) { Py_DECREF(k_str); pw_properties_free(p); return NULL; }

            // allow non-string values; convert with str()
            PyObject *v_str = PyObject_Str(val);
            if (!v_str) { Py_DECREF(k_str); pw_properties_free(p); return NULL; }
            const char *v = PyUnicode_AsUTF8(v_str);
            if (!v) { Py_DECREF(v_str); Py_DECREF(k_str); pw_properties_free(p); return NULL; }

            pw_properties_set(p, k, v);

            Py_DECREF(v_str);
            Py_DECREF(k_str);
        }
    }

    // Create the proxy under the PipeWire thread-loop lock.
    struct pw_proxy *proxy = NULL;
    create_state st = {
        .loop = self->thread_loop, // Initialize the loop pointer
        .done = false,
        .seq = -1,
        .err_res = 0,
        .err_msg = {0},
        .bound_id = SPA_ID_INVALID
    };
    struct spa_hook hook;

    Py_BEGIN_ALLOW_THREADS
    pw_thread_loop_lock(self->thread_loop);

    proxy = (struct pw_proxy *)pw_core_create_object(
        self->core,
        factory_name,
        type_name,
        version,
        &p->dict,
        0);

    pw_properties_free(p);

    if (proxy) {
        pw_proxy_add_listener(proxy, &hook, &create_proxy_events, &st);

        // Force a round-trip so we reliably see bound/done/error.
        st.seq = pw_proxy_sync(proxy, 0);

        while (!st.done) {
            pw_thread_loop_wait(self->thread_loop);
        }

        spa_hook_remove(&hook);
    }

    pw_thread_loop_unlock(self->thread_loop);
    Py_END_ALLOW_THREADS

    if (!proxy) {
        PyErr_SetString(PyExc_RuntimeError, "pw_core_create_object returned NULL (permission denied? unknown factory/type?)");
        return NULL;
    }

    if (st.err_res != 0) {
        // We must destroy the proxy if creation failed remotely but proxy exists locally
        pw_thread_loop_lock(self->thread_loop);
        pw_proxy_destroy(proxy);
        pw_thread_loop_unlock(self->thread_loop);

        PyErr_Format(PyExc_RuntimeError, "create_object error (%d): %s", st.err_res, st.err_msg);
        return NULL;
    }

//    // For now: only wrap Modules. Extend this later for Node/Link/etc.
//    if (strcmp(type_name, PW_TYPE_INTERFACE_Module) == 0) {
//        // Ensure PWModuleType is ready once; ideally do this only in PyInit__core.
//        if (PyType_Ready(&PWModuleType) < 0) {
//            pw_proxy_destroy(proxy);
//            return NULL;
//        }
//
//        PWModule *mod_obj = (PWModule *)PyObject_CallNoArgs((PyObject *)&PWModuleType);
//        if (!mod_obj) {
//            pw_proxy_destroy(proxy);
//            return NULL;
//        }
//
//        mod_obj->proxy = (struct pw_module *)proxy;
//        mod_obj->id = (st.bound_id != SPA_ID_INVALID) ? st.bound_id : pw_proxy_get_bound_id(proxy);
//        mod_obj->parent = (PyObject *)self;
//        Py_INCREF(mod_obj->parent);
//
//        // If you want name/args to be known immediately, you can set them from props here
//        // (or implement a refresh pattern).
//        // Leave as None for now:
//        Py_XDECREF(mod_obj->name);
//        mod_obj->name = Py_None; Py_INCREF(Py_None);
//        Py_XDECREF(mod_obj->args);
//        mod_obj->args = Py_None; Py_INCREF(Py_None);
//
//        return (PyObject *)mod_obj;
//    }

    // Not implemented yet: destroy to avoid leaking
    pw_thread_loop_lock(self->thread_loop);
    // pw_proxy_destroy(proxy);
    pw_thread_loop_unlock(self->thread_loop);

    Py_RETURN_NONE;
}