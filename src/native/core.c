//
// Created by edaniel on 2/5/26.
//

#include "pw_factory/pw_factory.h"
#include "pw_node/pw_node.h"
#include "pw_module.h"
#include "pw_connection/pw_connection.h"

static struct PyModuleDef module = { PyModuleDef_HEAD_INIT, "_core", NULL, -1, NULL };

PyMODINIT_FUNC PyInit__core(void) {
    pw_init(NULL, NULL);

    PyObject *m = PyModule_Create(&module);

    if (PyType_Ready(&PWConnectionType) < 0) return NULL;
    Py_INCREF(&PWConnectionType);
    PyModule_AddObject(m, "PWConnection", (PyObject *)&PWConnectionType);

    if (PyType_Ready(&PWModuleType) < 0) return NULL;
    Py_INCREF(&PWModuleType);
    PyModule_AddObject(m, "PWModule", (PyObject *)&PWModuleType);

    if (PyType_Ready(&PWFactoryType) < 0) return NULL;
    Py_INCREF(&PWFactoryType);
    PyModule_AddObject(m, "PWFactory", (PyObject *)&PWFactoryType);

    if (PyType_Ready(&PWNodeType) < 0) return NULL;
    Py_INCREF(&PWNodeType);
    PyModule_AddObject(m, "PWNode", (PyObject *)&PWNodeType);

    // --- Interface Types ---
    PyModule_AddStringConstant(m, "PW_TYPE_INTERFACE_Core", PW_TYPE_INTERFACE_Core);
    PyModule_AddStringConstant(m, "PW_TYPE_INTERFACE_Client", PW_TYPE_INTERFACE_Client);
    PyModule_AddStringConstant(m, "PW_TYPE_INTERFACE_Device", PW_TYPE_INTERFACE_Device);
    PyModule_AddStringConstant(m, "PW_TYPE_INTERFACE_Factory", PW_TYPE_INTERFACE_Factory);
    PyModule_AddStringConstant(m, "PW_TYPE_INTERFACE_Link", PW_TYPE_INTERFACE_Link);
    PyModule_AddStringConstant(m, "PW_TYPE_INTERFACE_Module", PW_TYPE_INTERFACE_Module);
    PyModule_AddStringConstant(m, "PW_TYPE_INTERFACE_Node", PW_TYPE_INTERFACE_Node);
    PyModule_AddStringConstant(m, "PW_TYPE_INTERFACE_Port", PW_TYPE_INTERFACE_Port);
    PyModule_AddStringConstant(m, "PW_TYPE_INTERFACE_Registry", PW_TYPE_INTERFACE_Registry);

    // --- Versions ---
    PyModule_AddIntConstant(m, "PW_VERSION_CORE", PW_VERSION_CORE);
    PyModule_AddIntConstant(m, "PW_VERSION_CLIENT", PW_VERSION_CLIENT);
    PyModule_AddIntConstant(m, "PW_VERSION_DEVICE", PW_VERSION_DEVICE);
    PyModule_AddIntConstant(m, "PW_VERSION_FACTORY", PW_VERSION_FACTORY);
    PyModule_AddIntConstant(m, "PW_VERSION_LINK", PW_VERSION_LINK);
    PyModule_AddIntConstant(m, "PW_VERSION_MODULE", PW_VERSION_MODULE);
    PyModule_AddIntConstant(m, "PW_VERSION_NODE", PW_VERSION_NODE);
    PyModule_AddIntConstant(m, "PW_VERSION_PORT", PW_VERSION_PORT);
    PyModule_AddIntConstant(m, "PW_VERSION_REGISTRY", PW_VERSION_REGISTRY);

    // --- Common Properties (Keys) ---
    // These are actually macros to string literals in PipeWire/SPA
    PyModule_AddStringConstant(m, "PW_KEY_APP_NAME", PW_KEY_APP_NAME);
    PyModule_AddStringConstant(m, "PW_KEY_APP_ID", PW_KEY_APP_ID);
    PyModule_AddStringConstant(m, "PW_KEY_OBJECT_ID", PW_KEY_OBJECT_ID);
    PyModule_AddStringConstant(m, "PW_KEY_OBJECT_SERIAL", PW_KEY_OBJECT_SERIAL);
    PyModule_AddStringConstant(m, "PW_KEY_MODULE_NAME", PW_KEY_MODULE_NAME);
    PyModule_AddStringConstant(m, "PW_KEY_FACTORY_NAME", PW_KEY_FACTORY_NAME);
    PyModule_AddStringConstant(m, "PW_KEY_DEVICE_NAME", PW_KEY_DEVICE_NAME);
    PyModule_AddStringConstant(m, "PW_KEY_NODE_NAME", PW_KEY_NODE_NAME);
    PyModule_AddStringConstant(m, "PW_KEY_MEDIA_CLASS", PW_KEY_MEDIA_CLASS);
    PyModule_AddStringConstant(m, "PW_KEY_MEDIA_TYPE", PW_KEY_MEDIA_TYPE);
    PyModule_AddStringConstant(m, "PW_KEY_MEDIA_CATEGORY", PW_KEY_MEDIA_CATEGORY);
    PyModule_AddStringConstant(m, "PW_KEY_MEDIA_ROLE", PW_KEY_MEDIA_ROLE);
    PyModule_AddStringConstant(m, "PW_KEY_AUDIO_CHANNEL", PW_KEY_AUDIO_CHANNEL);


    return m;
}