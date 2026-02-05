#include "pw_connection.h"
#include "../pw_factory/pw_factory.h"
#include <stdlib.h>
#include <string.h>

// Helper struct to track temporary proxies
struct proxy_node {
    struct pw_proxy *proxy;
    struct spa_hook hook;
    struct proxy_node *next;
};

struct registry_data {
    PyObject *list;
    struct pw_thread_loop *thread_loop;
    struct pw_core *core; // Added: Needed for pw_registry_bind
    PWConnection *self; // Added: Needed to set mod_obj->parent
    int sync_seq;
    bool is_done;
};

static void on_factory_info(void *data, const struct pw_factory_info *info) {
    const PyGILState_STATE gstate = PyGILState_Ensure();
    PWFactory *self = data;

    // Update the Python object with the details from info
    Py_XDECREF(self->type);
    self->type = PyUnicode_FromString(info->type ? info->type : "");
    Py_XDECREF(self->name);
    self->name = PyUnicode_FromString(info->name ? info->name : "");
    self->version = info->version;
    PyGILState_Release(gstate);
}

static const struct pw_factory_events factory_events = {
    .version = PW_VERSION_FACTORY_EVENTS,
    .info = on_factory_info
};

static void on_global(void *data, const uint32_t id, uint32_t permissions,
                      const char *type, uint32_t version, const struct spa_dict *props) {
    const struct registry_data *rd = data;

    if (strcmp(type, PW_TYPE_INTERFACE_Factory) == 0) {
        PyGILState_STATE gstate = PyGILState_Ensure();
        PWFactory *mod_obj = (PWFactory *) PyObject_CallNoArgs((PyObject *) &PWFactoryType);
        if (mod_obj == NULL) {
            PyErr_Print(); // <-- show the real exception (if any)
            PyGILState_Release(gstate);
            return;
        }
        mod_obj->id = id;
        mod_obj->connection = rd->self;
        Py_INCREF(mod_obj->connection);

        // 2. Bind the proxy
        // Use rd->core which we passed from the main function
        mod_obj->proxy = (struct pw_factory *) pw_registry_bind(
            rd->self->registry,
            id, type, PW_VERSION_FACTORY, 0
        );

        if (mod_obj->proxy) {
            // 3. Listen for the 'info' event to get args
            pw_factory_add_listener(mod_obj->proxy, &mod_obj->listener, &factory_events, mod_obj);
        }

        PyList_Append(rd->list, (PyObject *) mod_obj);
        Py_DECREF(mod_obj);
        PyGILState_Release(gstate);
    }
}

static void on_done(void *data, uint32_t id, const int seq) {
    struct registry_data *rd = data;
    if (seq == rd->sync_seq) {
        rd->is_done = true;
        pw_thread_loop_signal(rd->thread_loop, false);
    }
}

PyObject *PWConnection_get_factories(PWConnection *self, PyObject *Py_UNUSED(ignored)) {
    struct registry_data rd = {
        .list = PyList_New(0),
        .thread_loop = self->thread_loop,
        .core = self->core,
        .self = self,
        .is_done = false
    };

    Py_BEGIN_ALLOW_THREADS
        pw_thread_loop_lock(rd.thread_loop);

        const struct pw_registry *registry = pw_core_get_registry(self->core, PW_VERSION_REGISTRY, 0);
        if (registry < 0) {
            PyErr_SetString(PyExc_RuntimeError, "pw_core_get_registry failed");
            return NULL;
        }

        struct spa_hook reg_listener, core_listener;
        static const struct pw_registry_events reg_events = {
            .version = PW_VERSION_REGISTRY_EVENTS, .global = on_global
        };
        static const struct pw_core_events core_events = {.version = PW_VERSION_CORE_EVENTS, .done = on_done};

        pw_registry_add_listener(registry, &reg_listener, &reg_events, &rd);
        pw_core_add_listener(self->core, &core_listener, &core_events, &rd);

        // Trigger First Sync
        rd.sync_seq = pw_core_sync(self->core, PW_ID_CORE, 0);

        while (!rd.is_done) {
            pw_thread_loop_wait(rd.thread_loop);
        }

        // Trigger Second Sync to get factories infos
        rd.is_done = false;
        rd.sync_seq = pw_core_sync(self->core, PW_ID_CORE, 0);

        while (!rd.is_done) {
            pw_thread_loop_wait(rd.thread_loop);
        }

        // Cleanup Hooks
        spa_hook_remove(&reg_listener);
        spa_hook_remove(&core_listener);

        pw_proxy_destroy((struct pw_proxy *) registry);

        pw_thread_loop_unlock(rd.thread_loop);

    Py_END_ALLOW_THREADS

    return rd.list;
}
