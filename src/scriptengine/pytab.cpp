#include "pytab.h"

namespace PyTab {

typedef struct {
    PyObject_HEAD
    int id;
} Tab;

static PyMethodDef Tab_methods[] = {
    // TODO:not implemented
    // "Activate"
    // "Clone"
    // "Close"
    // "ConnectSftp"
    // "ResetCaption"
    {NULL,NULL,0,NULL}  // Sentinel
};

static PyObject* Tab_getIndex(Tab* self, void* closure) {
    Q_UNUSED(closure);
    return Py_BuildValue("i", self->id);
}

static PyGetSetDef Tab_getseters[] = {
    {"Index", (getter)Tab_getIndex, NULL, "Index", NULL},
    // TODO:not implemented
    // "Caption"
    // "Screen"
    // "Session"
    {NULL,NULL,NULL,NULL,NULL}  /* Sentinel */
};

// tab = Tab(id: int)
static PyObject* Tab_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    Tab* self;
    Q_UNUSED(kwds);
    if (args == NULL) {
        PyCore::CrtScriptErrorException_SetMessage("id is NULL");
        PyErr_SetString(PyCore::getQuardCRTException(), "args is NULL");
        return NULL;
    }
    int id;
    if (!PyArg_ParseTuple(args, "i", &id)) {
        PyCore::CrtScriptErrorException_SetMessage("id is not an integer");
        PyErr_SetString(PyCore::getQuardCRTException(), "args is not an integer");
        return NULL;
    }
    if(id<0) {
        PyCore::CrtScriptErrorException_SetMessage("id is invalid");
        PyErr_SetString(PyCore::getQuardCRTException(), "id is invalid");
        return NULL;
    }

    self = (Tab*)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->id = id;
    }

    return (PyObject*)self;
}

static void Tab_dealloc(PyObject* ptr) {
    Tab* self = (Tab*)ptr;
    Py_TYPE(self)->tp_free(ptr);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
PyTypeObject TabType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "TabClass",             /* tp_name */
    sizeof(Tab),            /* tp_basicsize */
    0,                      /* tp_itemsize */
    Tab_dealloc,            /* tp_dealloc */
    0,                      /* tp_print */
    0,                      /* tp_getattr */
    0,                      /* tp_setattr */
    0,                      /* tp_reserved */
    0,                      /* tp_repr */
    0,                      /* tp_as_number */
    0,                      /* tp_as_sequence */
    0,                      /* tp_as_mapping */
    0,                      /* tp_hash  */
    0,                      /* tp_call */
    0,                      /* tp_str */
    0,                      /* tp_getattro */
    0,                      /* tp_setattro */
    0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,     /* tp_flags */
    "Tab Class",            /* tp_doc */
    0,                      /* tp_traverse */
    0,                      /* tp_clear */
    0,                      /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    Tab_methods,            /* tp_methods */
    0,                      /* tp_members */
    Tab_getseters,          /* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    0,                      /* tp_descr_get */
    0,                      /* tp_descr_set */
    0,                      /* tp_dictoffset */
    0,                      /* tp_init */
    0,                      /* tp_alloc */
    Tab_new,                /* tp_new */
    0,                      /* tp_free */
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

}
