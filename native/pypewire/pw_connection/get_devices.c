#include "pw_connection.h"
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
    struct pw_core *core;
    struct proxy_node *proxies; // Linked list to track bound devices
    int sync_seq;
    bool is_done;
};

static void on_device_info(void *data, const struct pw_device_info *info) {
    PyObject *d = (PyObject *)data;

    if (info->props) {
        PyObject *props_dict = PyDict_New();
        const struct spa_dict_item *item;
        spa_dict_for_each(item, info->props) {
            PyDict_SetItemString(props_dict, item->key, PyUnicode_FromString(item->value));
        }
        PyDict_SetItemString(d, "props", props_dict);
        Py_DECREF(props_dict);
    }
}

static const struct pw_device_events device_events = {
    .version = PW_VERSION_DEVICE_EVENTS,
    .info = on_device_info
};

static void on_global(void *data, uint32_t id, uint32_t permissions,
                      const char *type, uint32_t version, const struct spa_dict *props) {
    struct registry_data *rd = data;

    if (strcmp(type, PW_TYPE_INTERFACE_Device) == 0) {
        PyObject *d = PyDict_New();

        PyDict_SetItemString(d, "id", PyLong_FromUnsignedLong(id));
        const char *name = spa_dict_lookup(props, PW_KEY_DEVICE_NAME);
        PyDict_SetItemString(d, "name", PyUnicode_FromString(name ? name : "unknown"));

        // Bind to the device
        // Note: We use the core from our rd struct
        struct pw_proxy *proxy = pw_registry_bind(
            pw_core_get_registry(rd->core, PW_VERSION_REGISTRY, 0),
            id, type, PW_VERSION_DEVICE, 0
        );

        if (proxy) {
            struct proxy_node *node = malloc(sizeof(struct proxy_node));
            node->proxy = proxy;
            node->next = rd->proxies;
            rd->proxies = node;

            pw_device_add_listener((struct pw_device*)proxy, &node->hook, &device_events, d);
        }

        PyList_Append(rd->list, d);
        Py_DECREF(d);
    }
}

static void on_done(void *data, uint32_t id, int seq) {
    struct registry_data *rd = data;
    if (seq == rd->sync_seq) {
        rd->is_done = true;
        pw_thread_loop_signal(rd->thread_loop, false);
    }
}

PyObject *PWConnection_get_devices(PWConnection *self, PyObject *Py_UNUSED(ignored)) {
    struct registry_data rd = {
        .list = PyList_New(0),
        .thread_loop = self->thread_loop,
        .core = self->core,
        .proxies = NULL,
        .is_done = false
    };

    // 1. LOCK
    pw_thread_loop_lock(self->thread_loop);

    struct spa_hook reg_listener, core_listener;

    static const struct pw_registry_events reg_events = { .version = PW_VERSION_REGISTRY_EVENTS, .global = on_global };
    static const struct pw_core_events core_events = { .version = PW_VERSION_CORE_EVENTS, .done = on_done };

    pw_registry_add_listener(self->registry, &reg_listener, &reg_events, &rd);
    pw_core_add_listener(self->core, &core_listener, &core_events, &rd);

    rd.sync_seq = pw_core_sync(self->core, PW_ID_CORE, 0);

    // 2. WAIT (Releases lock internally)
    while (!rd.is_done) {
        pw_thread_loop_wait(self->thread_loop);
    }

    // 3. CLEANUP (While still locked)
    spa_hook_remove(&reg_listener);
    spa_hook_remove(&core_listener);

    // Clean up all the temporary device proxies we bound to
    struct proxy_node *current = rd.proxies;
    while (current) {
        struct proxy_node *next = current->next;
        spa_hook_remove(&current->hook);
        pw_proxy_destroy(current->proxy);
        free(current);
        current = next;
    }

    // 4. UNLOCK
    pw_thread_loop_unlock(self->thread_loop);

    return rd.list;
}