#include "device_discovery.h"
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
    struct pw_main_loop *loop;
    struct pw_core *core;
    struct proxy_node *proxies; // Linked list of proxies to clean up
    int sync_seq_1;             // First sync (get globals)
    int sync_seq_2;             // Second sync (get details)
};

static void on_device_info(void *data, const struct pw_device_info *info) {
    PyObject *d = (PyObject *)data; // We passed the Dict as user_data

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

    // We only care about Devices
    if (strcmp(type, PW_TYPE_INTERFACE_Device) == 0) {
        PyObject *d = PyDict_New();

        // Basic info from Registry
        PyDict_SetItemString(d, "id", PyLong_FromUnsignedLong(id));
        const char *name = spa_dict_lookup(props, PW_KEY_DEVICE_NAME);
        PyDict_SetItemString(d, "name", PyUnicode_FromString(name ? name : "unknown"));

        // Bind to the device to get detailed info
        struct pw_proxy *proxy = pw_registry_bind(
            pw_core_get_registry(rd->core, PW_VERSION_REGISTRY, 0),
            id, type, PW_VERSION_DEVICE, 0
        );

        if (proxy) {
            // Create a node to track this proxy
            struct proxy_node *node = malloc(sizeof(struct proxy_node));
            node->proxy = proxy;
            node->next = rd->proxies;
            rd->proxies = node;

            // Listen to the device events, passing the Python Dict as user_data
            pw_device_add_listener((struct pw_device*)proxy, &node->hook, &device_events, d);
        }

        PyList_Append(rd->list, d);
        Py_DECREF(d);
    }
}

static void on_done(void *data, uint32_t id, int seq) {
    struct registry_data *rd = data;

    // Sync 1 Complete: We have received all Global events.
    // Now we must trigger Sync 2 to allow the "bind" requests we just made
    // to go to the server and come back with "info".
    if (seq == rd->sync_seq_1) {
        rd->sync_seq_2 = pw_core_sync(rd->core, PW_ID_CORE, 0);
    }
    // Sync 2 Complete: We have received all Info events.
    else if (seq == rd->sync_seq_2) {
        pw_main_loop_quit(rd->loop);
    }
}

PyObject *PWConnection_get_devices(PWConnection *self, PyObject *Py_UNUSED(ignored)) {
    struct pw_registry *registry = pw_core_get_registry(self->core, PW_VERSION_REGISTRY, 0);

    struct registry_data rd = {
        .list = PyList_New(0),
        .loop = self->loop,
        .core = self->core,
        .proxies = NULL,
        .sync_seq_1 = 0,
        .sync_seq_2 = -1
    };

    struct spa_hook reg_listener, core_listener;
    static const struct pw_registry_events reg_events = { .version = PW_VERSION_REGISTRY_EVENTS, .global = on_global };
    static const struct pw_core_events core_events = { .version = PW_VERSION_CORE_EVENTS, .done = on_done };

    pw_registry_add_listener(registry, &reg_listener, &reg_events, &rd);
    pw_core_add_listener(self->core, &core_listener, &core_events, &rd);

    // Trigger First Sync
    rd.sync_seq_1 = pw_core_sync(self->core, PW_ID_CORE, 0);

    // Run Loop (Will process Globals -> Sync 1 -> Info events -> Sync 2 -> Quit)
    pw_main_loop_run(self->loop);

    // Cleanup Hooks
    spa_hook_remove(&reg_listener);
    spa_hook_remove(&core_listener);

    // Cleanup Registry Proxy
    pw_proxy_destroy((struct pw_proxy*)registry);

    // Cleanup Temporary Device Proxies
    struct proxy_node *current = rd.proxies;
    while (current) {
        struct proxy_node *next = current->next;
        spa_hook_remove(&current->hook);
        pw_proxy_destroy(current->proxy);
        free(current);
        current = next;
    }

    return rd.list;
}
