#include "pysessionconfiguration.h"

namespace PySessionConfiguration {

typedef struct {
    PyObject_HEAD
} SessionConfiguration;

static PyMethodDef SessionConfiguration_methods[] = {
    // TODO:not implemented
    // "ConnectInTab"
    // "GetOption"
    // "Save"
    // "SetOption"
    {NULL,NULL,0,NULL}  // Sentinel
};

static PyObject* SessionConfiguration_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    SessionConfiguration* self;
    Q_UNUSED(args);Q_UNUSED(kwds);
    self = (SessionConfiguration*)type->tp_alloc(type, 0);

    return (PyObject*)self;
}

static void SessionConfiguration_dealloc(PyObject* ptr) {
    SessionConfiguration* self = (SessionConfiguration*)ptr;
    Py_TYPE(self)->tp_free(ptr);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
PyTypeObject SessionConfigurationType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "SessionConfigurationClass",    /* tp_name */
    sizeof(SessionConfiguration),   /* tp_basicsize */
    0,                              /* tp_itemsize */
    SessionConfiguration_dealloc,   /* tp_dealloc */
    0,                              /* tp_print */
    0,                              /* tp_getattr */
    0,                              /* tp_setattr */
    0,                              /* tp_reserved */
    0,                              /* tp_repr */
    0,                              /* tp_as_number */
    0,                              /* tp_as_sequence */
    0,                              /* tp_as_mapping */
    0,                              /* tp_hash  */
    0,                              /* tp_call */
    0,                              /* tp_str */
    0,                              /* tp_getattro */
    0,                              /* tp_setattro */
    0,                              /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,             /* tp_flags */
    "SessionConfiguration Class",   /* tp_doc */
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    SessionConfiguration_methods,   /* tp_methods */
    0,                              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    0,                              /* tp_init */
    0,                              /* tp_alloc */
    SessionConfiguration_new,       /* tp_new */
    0,                              /* tp_free */
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

}
