#include "pydialog.h"

#include <QString>
#include <QMessageBox>
#include <QInputDialog>

namespace PyDialog {

typedef struct {
    PyObject_HEAD
} Dialog;

static PyObject* Screen_getOK(Dialog* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    return Py_BuildValue("i", QMessageBox::Ok);
}

static PyObject* Screen_getCancel(Dialog* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    return Py_BuildValue("i", QMessageBox::Cancel);
}

static PyObject* Screen_getAbort(Dialog* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    return Py_BuildValue("i", QMessageBox::Abort);
}

static PyObject* Screen_getRetry(Dialog* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    return Py_BuildValue("i", QMessageBox::Retry);
}

static PyObject* Screen_getIgnore(Dialog* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    return Py_BuildValue("i", QMessageBox::Ignore);
}

static PyObject* Screen_getYes(Dialog* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    return Py_BuildValue("i", QMessageBox::Yes);
}

static PyObject* Screen_getNo(Dialog* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    return Py_BuildValue("i", QMessageBox::No);
}

static PyGetSetDef Dialog_getseters[] = {
    {"OK", (getter)Screen_getOK, NULL, "OK", NULL},
    {"Cancel", (getter)Screen_getCancel, NULL, "Cancel", NULL},
    {"Abort", (getter)Screen_getAbort, NULL, "Abort", NULL},
    {"Retry", (getter)Screen_getRetry, NULL, "Retry", NULL},
    {"Ignore", (getter)Screen_getIgnore, NULL, "Ignore", NULL},
    {"Yes", (getter)Screen_getYes, NULL, "Yes", NULL},
    {"No", (getter)Screen_getNo, NULL, "No", NULL},
    {NULL,NULL,NULL,NULL,NULL}  /* Sentinel */
};

// Dialog.MessageBox(message: str, title: str, buttons: int) -> int
static PyObject* Dialog_MessageBox(Dialog* self, PyObject* args, PyObject* kwargs) {
    const char* message;
    const char* title = "Message";
    int buttons = 0;
    static const char* kwlist[] = {"message", "title", "buttons", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|si", (char**)kwlist, &message, &title, &buttons)) {
        return NULL;
    }
    QString messageStr = QString::fromUtf8(message);
    QString titleStr = QString::fromUtf8(title);
    PyCore::getCore()->messageBox(messageStr,titleStr,buttons);
    Q_UNUSED(self);
    return Py_BuildValue("i", 0);
}

// Dialog.Prompt(prompt: str, name: str, input: str, password: bool) -> str
static PyObject* Dialog_Prompt(Dialog* self, PyObject* args) {
    const char* prompt;
    const char* name;
    const char* input;
    int password;
    if (!PyArg_ParseTuple(args, "sssi", &prompt, &name, &input, &password)) {
        return NULL;
    }
    QString promptStr = QString::fromUtf8(prompt);
    QString nameStr = QString::fromUtf8(name);
    QString inputStr = QString::fromUtf8(input);
    QString result = PyCore::getCore()->prompt(promptStr, nameStr, inputStr, password);
    Q_UNUSED(self);
    return Py_BuildValue("s", result.toUtf8().constData());
}

// Dialog.FileOpenDialog(title: str, buttonLabel: str, directory: str, filter: str) -> str
static PyObject* Dialog_FileOpenDialog(Dialog* self, PyObject* args, PyObject* kwargs) {
    const char* title;
    const char* buttonLabel = "Open";
    const char* directory = "";
    const char* filter = "All Files (*.*)";
    static const char* kwlist[] = {"title", "buttonLabel", "directory", "filter", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|sss", (char**)kwlist, &title, &buttonLabel, &directory, &filter)) {
        return NULL;
    }
    QString titleStr = QString::fromUtf8(title);
    QString buttonLabelStr = QString::fromUtf8(buttonLabel);
    QString directoryStr = QString::fromUtf8(directory);
    QString filterStr = QString::fromUtf8(filter);
    QString result = PyCore::getCore()->fileOpenDialog(titleStr, buttonLabelStr, directoryStr, filterStr);
    Q_UNUSED(self);
    return Py_BuildValue("s", result.toUtf8().constData());
}

// Dialog.FileSaveDialog(title: str, buttonLabel: str, directory: str, filter: str) -> str
static PyObject* Dialog_FileSaveDialog(Dialog* self, PyObject* args, PyObject* kwargs) {
    const char* title;
    const char* buttonLabel = "Save";
    const char* directory = "";
    const char* filter = "All Files (*.*)";
    static const char* kwlist[] = {"title", "buttonLabel", "directory", "filter", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|sss", (char**)kwlist, &title, &buttonLabel, &directory, &filter)) {
        return NULL;
    }
    QString titleStr = QString::fromUtf8(title);
    QString buttonLabelStr = QString::fromUtf8(buttonLabel);
    QString directoryStr = QString::fromUtf8(directory);
    QString filterStr = QString::fromUtf8(filter);
    QString result = PyCore::getCore()->fileSaveDialog(titleStr, buttonLabelStr, directoryStr, filterStr);
    Q_UNUSED(self);
    return Py_BuildValue("s", result.toUtf8().constData());
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
static PyMethodDef Dialog_methods[] = {
    {"MessageBox", (PyCFunction)Dialog_MessageBox, METH_VARARGS|METH_KEYWORDS, "MessageBox"},
    {"Prompt", (PyCFunction)Dialog_Prompt, METH_VARARGS, "Prompt"},
    {"FileOpenDialog", (PyCFunction)Dialog_FileOpenDialog, METH_VARARGS|METH_KEYWORDS, "FileOpenDialog"},
    {"FileSaveDialog", (PyCFunction)Dialog_FileSaveDialog, METH_VARARGS|METH_KEYWORDS, "FileSaveDialog"},
    {NULL,NULL,0,NULL}  // Sentinel
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

static PyObject* Dialog_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    Dialog* self;
    Q_UNUSED(args);Q_UNUSED(kwds);
    self = (Dialog*)type->tp_alloc(type, 0);

    return (PyObject*)self;
}

static void Dialog_dealloc(PyObject* ptr) {
    Dialog* self = (Dialog*)ptr;
    Py_TYPE(self)->tp_free((PyObject*)self);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
PyTypeObject DialogType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "DialogClass",          /* tp_name */
    sizeof(Dialog),         /* tp_basicsize */
    0,                      /* tp_itemsize */
    Dialog_dealloc,         /* tp_dealloc */
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
    "Dialog Class",         /* tp_doc */
    0,                      /* tp_traverse */
    0,                      /* tp_clear */
    0,                      /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    Dialog_methods,         /* tp_methods */
    0,                      /* tp_members */
    Dialog_getseters,       /* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    0,                      /* tp_descr_get */
    0,                      /* tp_descr_set */
    0,                      /* tp_dictoffset */
    0,                      /* tp_init */
    0,                      /* tp_alloc */
    Dialog_new,             /* tp_new */
    0,                      /* tp_free */
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

}
