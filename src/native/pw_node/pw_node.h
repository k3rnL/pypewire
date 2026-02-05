//
// Created by edaniel on 2/5/26.
//

#ifndef PYPEWIRE_PW_NODE_H
#define PYPEWIRE_PW_NODE_H

#include <Python.h>
#include <pipewire/pipewire.h>

#include "../pw_connection/pw_connection.h"

typedef struct {
    PyObject_HEAD
    struct pw_node *proxy;
    struct spa_hook listener;
    PWConnection *connection;
    uint32_t id;
    uint32_t max_input_ports;
    uint32_t max_output_ports;
    uint32_t n_input_ports;
    uint32_t n_output_ports;
    int8_t state;
    PyObject *error;
    // TODO: Add params
} PWNode;

extern PyTypeObject PWNodeType;

#endif //PYPEWIRE_PW_NODE_H