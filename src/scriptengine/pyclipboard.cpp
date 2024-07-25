#include "pyclipboard.h"

#include <QApplication>
#include <QClipboard>

namespace PyClipboard {

typedef struct {
    PyObject_HEAD
} Clipboard;

static PyObject* Clipboard_getText(Clipboard* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    QString text = QApplication::clipboard()->text();
    return Py_BuildValue("s", text.toUtf8().constData());
}

static int Clipboard_setText(Clipboard* self, PyObject* value, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    const char* text;
    if(!PyArg_ParseTuple(value, "s", &text)) {
        return -1;
    }
    QApplication::clipboard()->setText(text);
    return 0;
}

static PyGetSetDef Clipboard_getseters[] = {
    {"Text", (getter)Clipboard_getText, (setter)Clipboard_setText, "Text", NULL},
    {NULL,NULL,NULL,NULL,NULL}  /* Sentinel */
};

static PyMethodDef Clipboard_methods[] = {
    {NULL,NULL,0,NULL}  // Sentinel
};

static PyObject* Clipboard_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    Clipboard* self;
    Q_UNUSED(args);Q_UNUSED(kwds);
    self = (Clipboard*)type->tp_alloc(type, 0);

    return (PyObject*)self;
}

static void Clipboard_dealloc(PyObject* ptr) {
    Clipboard* self = (Clipboard*)ptr;
    Py_TYPE(self)->tp_free(ptr);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
PyTypeObject ClipboardType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "ClipboardClass",       /* tp_name */
    sizeof(Clipboard),      /* tp_basicsize */
    0,                      /* tp_itemsize */
    Clipboard_dealloc,      /* tp_dealloc */
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
    "Clipboard Class",      /* tp_doc */
    0,                      /* tp_traverse */
    0,                      /* tp_clear */
    0,                      /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    Clipboard_methods,      /* tp_methods */
    0,                      /* tp_members */
    Clipboard_getseters,    /* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    0,                      /* tp_descr_get */
    0,                      /* tp_descr_set */
    0,                      /* tp_dictoffset */
    0,                      /* tp_init */
    0,                      /* tp_alloc */
    Clipboard_new,          /* tp_new */
    0,                      /* tp_free */
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

}
