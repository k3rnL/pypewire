//
// Created by edaniel on 2/5/26.
//

#include <pipewire/keys.h>

#include "wp_connection.h"

typedef struct {
    WPConnection *conn;
    PyObject *result;
} GetNodesData;

// This runs on the WP thread (in the GMainLoop context)
static gboolean do_get_nodes_on_wp_thread(gpointer user_data) {
    GetNodesData *data = user_data;
    WPConnection *conn = data->conn;

    // We're on the WP thread, safe to access conn->om
    g_autoptr(WpIterator) it = wp_object_manager_new_iterator(conn->om);
    g_auto(GValue) val = G_VALUE_INIT;

    // Create Python list (must acquire GIL since we're creating Python objects)
    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject *list = PyList_New(0);
    if (!list) {
        PyGILState_Release(gstate);
        data->result = NULL;
        goto done;
    }

    // Iterate over all nodes
    while (wp_iterator_next(it, &val)) {
        WpNode *node = g_value_get_object(&val);
        if (!WP_IS_NODE(node)) {
            g_value_unset(&val);
            continue;
        }

        // Check if the object has the required features
        WpObjectFeatures features = wp_object_get_active_features(WP_OBJECT(node));
        if (!(features & WP_PROXY_FEATURE_BOUND) || !(features & WP_PIPEWIRE_OBJECT_FEATURE_INFO)) {
            // Skip nodes that don't have features activated yet
            g_value_unset(&val);
            continue;
        }

        // Get node properties - now safe to access
        guint32 id = wp_proxy_get_bound_id(WP_PROXY(node));
        WpProperties *props = wp_pipewire_object_get_properties(WP_PIPEWIRE_OBJECT(node));

        const char *name = props ? wp_properties_get(props, PW_KEY_NODE_NAME) : NULL;
        const char *nick = props ? wp_properties_get(props, PW_KEY_NODE_NICK) : NULL;
        const char *description = props ? wp_properties_get(props, PW_KEY_NODE_DESCRIPTION) : NULL;
        const char *media_class = props ? wp_properties_get(props, PW_KEY_MEDIA_CLASS) : NULL;

        // Create dict for this node
        PyObject *node_dict = PyDict_New();
        if (!node_dict) {
            g_value_unset(&val);
            Py_DECREF(list);
            list = NULL;
            break;
        }

        PyDict_SetItemString(node_dict, "id", PyLong_FromLong(id));
        if (name)
            PyDict_SetItemString(node_dict, "name", PyUnicode_FromString(name));
        if (nick)
            PyDict_SetItemString(node_dict, "nick", PyUnicode_FromString(nick));
        if (description)
            PyDict_SetItemString(node_dict, "description", PyUnicode_FromString(description));
        if (media_class)
            PyDict_SetItemString(node_dict, "media_class", PyUnicode_FromString(media_class));

        PyList_Append(list, node_dict);
        Py_DECREF(node_dict);

        g_value_unset(&val);
    }

    data->result = list;
    PyGILState_Release(gstate);

done:
    // Signal that the call is complete
    g_mutex_lock(&conn->call_lock);
    conn->call_completed = TRUE;
    g_cond_signal(&conn->call_cond);
    g_mutex_unlock(&conn->call_lock);

    return G_SOURCE_REMOVE;
}

PyObject *WPConnection_get_nodes(WPConnection *self, PyObject *Py_UNUSED(ignored)) {
    GetNodesData data = {
        .conn = self,
        .result = NULL
    };

    // Reset call state
    g_mutex_lock(&self->call_lock);
    self->call_completed = FALSE;
    g_mutex_unlock(&self->call_lock);

    // Schedule the work on the WP thread's main context
    GSource *source = g_idle_source_new();
    g_source_set_callback(source, do_get_nodes_on_wp_thread, &data, NULL);
    g_source_attach(source, self->ctx);
    g_source_unref(source);

    // Release GIL and wait for the WP thread to complete the work
    Py_BEGIN_ALLOW_THREADS
    g_mutex_lock(&self->call_lock);
    while (!self->call_completed) {
        g_cond_wait(&self->call_cond, &self->call_lock);
    }
    g_mutex_unlock(&self->call_lock);
    Py_END_ALLOW_THREADS

    // Return the result (or NULL if error occurred)
    if (!data.result) {
        if (!PyErr_Occurred()) {
            PyErr_SetString(PyExc_RuntimeError, "Failed to retrieve nodes");
        }
        return NULL;
    }

    return data.result;
}
