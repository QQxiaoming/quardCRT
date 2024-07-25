#include "pywindow.h"

namespace PyWindow {

typedef struct {
    PyObject_HEAD
} Window;

static PyObject* Window_getState(Window* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    int type = PyCore::getCore()->getWindowShowType();
    return Py_BuildValue("i", type);
}

static PyObject* Window_getActive(Window* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    bool active = PyCore::getCore()->getWindowActive();
    return Py_BuildValue("b", active);
}

static PyObject* Window_getHide(Window* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    return Py_BuildValue("i", 0);
}

static PyObject* Window_getShowNormal(Window* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    return Py_BuildValue("i", 1);
}

static PyObject* Window_getShowMinimized(Window* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    return Py_BuildValue("i", 2);
}

static PyObject* Window_getShowMaximized(Window* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    return Py_BuildValue("i", 3);
}

static PyGetSetDef Window_getseters[] = {
    {"State", (getter)Window_getState, NULL, "State", NULL},
    {"Active", (getter)Window_getActive, NULL, "Active", NULL},
    {"Hide", (getter)Window_getHide, NULL, "Hide", NULL},
    {"ShowNormal", (getter)Window_getShowNormal, NULL, "ShowNormal", NULL},
    {"ShowMinimized", (getter)Window_getShowMinimized, NULL, "ShowMinimized", NULL},
    {"ShowMaximized", (getter)Window_getShowMaximized, NULL, "ShowMaximized", NULL},
    {NULL,NULL,NULL,NULL,NULL}  /* Sentinel */
};

// Activate() -> None
static PyObject* Window_Activate(Window* self, PyObject* args) {
    Q_UNUSED(self);
    Q_UNUSED(args);
    PyCore::getCore()->windowActivate();
    Py_RETURN_NONE;
}

// Show(type: int) -> None
static PyObject* Window_Show(Window* self, PyObject* args) {
    int type;
    Q_UNUSED(self);
    if (!PyArg_ParseTuple(args, "i", &type)) {
        return NULL;
    }
    PyCore::getCore()->windowShow(type);
    Py_RETURN_NONE;
}

static PyMethodDef Window_methods[] = {
    {"Activate", (PyCFunction)Window_Activate, METH_NOARGS, "Activate"},
    {"Show", (PyCFunction)Window_Show, METH_VARARGS, "Show"},
    {NULL,NULL,0,NULL}  // Sentinel
};

static PyObject* Window_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    Window* self;
    Q_UNUSED(args);Q_UNUSED(kwds);
    self = (Window*)type->tp_alloc(type, 0);

    return (PyObject*)self;
}

static void Window_dealloc(PyObject* ptr) {
    Window* self = (Window*)ptr;
    Py_TYPE(self)->tp_free(ptr);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
PyTypeObject WindowType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "WindowClass",          /* tp_name */
    sizeof(Window),         /* tp_basicsize */
    0,                      /* tp_itemsize */
    Window_dealloc,         /* tp_dealloc */
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
    "Window Class",         /* tp_doc */
    0,                      /* tp_traverse */
    0,                      /* tp_clear */
    0,                      /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    Window_methods,         /* tp_methods */
    0,                      /* tp_members */
    Window_getseters,       /* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    0,                      /* tp_descr_get */
    0,                      /* tp_descr_set */
    0,                      /* tp_dictoffset */
    0,                      /* tp_init */
    0,                      /* tp_alloc */
    Window_new,             /* tp_new */
    0,                      /* tp_free */
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

}
