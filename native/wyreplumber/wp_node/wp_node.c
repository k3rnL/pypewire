//
// Created by edaniel on 2/5/26.
//

#include "wp_node.h"
#include <pipewire/keys.h>

// Forward declarations
static void WPNode_dealloc(WPNode *self);
static PyObject *WPNode_repr(WPNode *self);
static PyObject *WPNode_get_id(WPNode *self, void *closure);
static PyObject *WPNode_get_properties(WPNode *self, void *closure);
static PyObject *WPNode_get_state(WPNode *self, void *closure);
static PyObject *WPNode_get_n_input_ports(WPNode *self, void *closure);
static PyObject *WPNode_get_max_input_ports(WPNode *self, void *closure);
static PyObject *WPNode_get_n_output_ports(WPNode *self, void *closure);
static PyObject *WPNode_get_max_output_ports(WPNode *self, void *closure);
static PyObject *WPNode_get_error_message(WPNode *self, void *closure);
static PyObject *WPNode_delete(WPNode *self, PyObject *Py_UNUSED(ignored));

static void WPNode_dealloc(WPNode *self) {
    if (self->node) {
        g_object_unref(self->node);
        self->node = NULL;
    }
    if (self->core) {
        g_object_unref(self->core);
        self->core = NULL;
    }
    Py_XDECREF(self->properties);
    if (self->error_message) {
        free(self->error_message);
        self->error_message = NULL;
    }
    Py_TYPE(self)->tp_free((PyObject *)self);
}

static PyObject *WPNode_repr(WPNode *self) {
    guint32 id = wp_proxy_get_bound_id(WP_PROXY(self->node));
    return PyUnicode_FromFormat("<WPNode id=%u>", id);
}

static PyObject *WPNode_get_id(WPNode *self, void *closure) {
    guint32 id = wp_proxy_get_bound_id(WP_PROXY(self->node));
    return PyLong_FromUnsignedLong(id);
}

static PyObject *WPNode_get_properties(WPNode *self, void *closure) {
    Py_INCREF(self->properties);
    return self->properties;
}

static PyObject *WPNode_get_state(WPNode *self, void *closure) {
    return PyLong_FromLong(self->state);
}

static PyObject *WPNode_get_n_input_ports(WPNode *self, void *closure) {
    return PyLong_FromUnsignedLong(self->n_input_ports);
}

static PyObject *WPNode_get_max_input_ports(WPNode *self, void *closure) {
    return PyLong_FromUnsignedLong(self->max_input_ports);
}

static PyObject *WPNode_get_n_output_ports(WPNode *self, void *closure) {
    return PyLong_FromUnsignedLong(self->n_output_ports);
}

static PyObject *WPNode_get_max_output_ports(WPNode *self, void *closure) {
    return PyLong_FromUnsignedLong(self->max_output_ports);
}

static PyObject *WPNode_get_error_message(WPNode *self, void *closure) {
    if (self->error_message) {
        return PyUnicode_FromString(self->error_message);
    }
    Py_RETURN_NONE;
}

static PyObject *WPNode_delete(WPNode *self, PyObject *Py_UNUSED(ignored)) {
    if (!self->node) {
        PyErr_SetString(PyExc_RuntimeError, "Node already deleted or invalid");
        return NULL;
    }

    // Request destruction of the node
    wp_global_proxy_request_destroy(WP_GLOBAL_PROXY(self->node));

    Py_RETURN_NONE;
}

static PyGetSetDef WPNode_getsetters[] = {
    {"id", (getter)WPNode_get_id, NULL, "Node ID", NULL},
    {"properties", (getter)WPNode_get_properties, NULL, "All node properties", NULL},
    {"state", (getter)WPNode_get_state, NULL, "Node state (WP_NODE_STATE_*)", NULL},
    {"n_input_ports", (getter)WPNode_get_n_input_ports, NULL, "Number of input ports", NULL},
    {"max_input_ports", (getter)WPNode_get_max_input_ports, NULL, "Maximum input ports", NULL},
    {"n_output_ports", (getter)WPNode_get_n_output_ports, NULL, "Number of output ports", NULL},
    {"max_output_ports", (getter)WPNode_get_max_output_ports, NULL, "Maximum output ports", NULL},
    {"error_message", (getter)WPNode_get_error_message, NULL, "Error message if state is ERROR", NULL},
    {NULL}
};

static PyMethodDef WPNode_methods[] = {
    {"delete", (PyCFunction)WPNode_delete, METH_NOARGS, "Delete this node from the server"},
    {NULL}
};

PyTypeObject WPNodeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "_core.WPNode",
    .tp_doc = "WirePlumber Node Object",
    .tp_basicsize = sizeof(WPNode),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_repr = (reprfunc)WPNode_repr,
    .tp_dealloc = (destructor)WPNode_dealloc,
    .tp_methods = WPNode_methods,
    .tp_getset = WPNode_getsetters,
};

// Helper to iterate over all properties
static PyObject *iterate_wp_properties(WpProperties *props) {
    PyObject *dict = PyDict_New();
    if (!dict) return NULL;

    if (!props) return dict;

    WpIterator *it = wp_properties_new_iterator(props);
    g_auto(GValue) item = G_VALUE_INIT;

    while (wp_iterator_next(it, &item)) {
        WpPropertiesItem *prop_item = g_value_get_boxed(&item);
        const char *key = wp_properties_item_get_key(prop_item);
        const char *value = wp_properties_item_get_value(prop_item);

        if (key && value) {
            PyObject *py_value = PyUnicode_FromString(value);
            PyDict_SetItemString(dict, key, py_value);
            Py_DECREF(py_value);
        }

        g_value_unset(&item);
    }

    wp_iterator_unref(it);
    return dict;
}

PyObject *WPNode_from_wp_node(WpNode *wp_node, WpCore *core) {
    WPNode *self = (WPNode *)WPNodeType.tp_alloc(&WPNodeType, 0);
    if (!self) return NULL;

    // Store references
    self->node = g_object_ref(wp_node);
    self->core = g_object_ref(core);

    // Get all properties
    WpProperties *props = wp_pipewire_object_get_properties(WP_PIPEWIRE_OBJECT(wp_node));
    self->properties = iterate_wp_properties(props);
    if (!self->properties) {
        Py_DECREF(self);
        return NULL;
    }

    // Get state
    const char *error_msg = NULL;
    self->state = wp_node_get_state(wp_node, &error_msg);
    if (error_msg) {
        self->error_message = strdup(error_msg);
    } else {
        self->error_message = NULL;
    }

    // Get port information
    self->n_input_ports = wp_node_get_n_input_ports(wp_node, &self->max_input_ports);
    self->n_output_ports = wp_node_get_n_output_ports(wp_node, &self->max_output_ports);

    return (PyObject *)self;
}
