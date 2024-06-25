#include "pyscreen.h"

#include <QString>
#include <QMessageBox>

namespace PyScreen {

typedef struct {
    PyObject_HEAD
    int id;
    bool synchronous;
    bool ignoreEscape;
    bool ignoreCase;
    int matchIndex;
} Screen;

// Screen.WaitForString(str: str, timeout: int, bcaseInsensitive: bool) -> str
static PyObject* Screen_WaitForString(Screen* self, PyObject* args, PyObject* kwargs) {
    static const char* kwlist[] = {"strlist", "timeout", "bcaseInsensitive", NULL};
    const char* str;
    int timeout = 0;
    bool bcaseInsensitive = self->ignoreCase;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s|ip", (char**)kwlist, &str, &timeout, &bcaseInsensitive)) {
        return NULL;
    }
    QString qstr = QString::fromUtf8(str);
    if(self->ignoreEscape) {
        qstr.remove(QChar(27));
        qstr.remove(QChar(7));
    }
    QStringList qstrlist;
    qstrlist.append(qstr);
    QString ret = PyCore::getCore()->screenWaitForString(qstrlist,timeout,bcaseInsensitive,self->matchIndex,self->id);
    return PyUnicode_FromString(ret.toUtf8().constData());
}

// Screen.WaitForStrings(strlist: list[str], timeout: int, bcaseInsensitive: bool) -> str
static PyObject* Screen_WaitForStrings(Screen* self, PyObject* args, PyObject* kwargs) {
    static const char* kwlist[] = {"strlist", "timeout", "bcaseInsensitive", NULL};
    PyObject* strlist;
    int timeout = 0;
    bool bcaseInsensitive = self->ignoreCase;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|ip", (char**)kwlist, &strlist, &timeout, &bcaseInsensitive)) {
        return NULL;
    }
    QStringList qstrlist;
    if (!PyList_Check(strlist)) {
        PyCore::CrtScriptErrorException_SetMessage("The strlist parameter must be a list of strings");
        PyErr_SetString(PyCore::getQuardCRTException(), "The strlist parameter must be a list of strings");
        return NULL;
    } else {
        int size = PyList_Size(strlist);
        for (int i = 0; i < size; i++) {
            PyObject* item = PyList_GetItem(strlist, i);
            if (!PyUnicode_Check(item)) {
                PyCore::CrtScriptErrorException_SetMessage("The strlist parameter must be a list of strings");
                PyErr_SetString(PyCore::getQuardCRTException(), "The strlist parameter must be a list of strings");
                return NULL;
            }
            const char* str = PyUnicode_AsUTF8(item);
            qstrlist.append(QString::fromUtf8(str));
        }
    }
    if(self->ignoreEscape) {
        foreach (QString str, qstrlist) {
            str.remove(QChar(27));
            str.remove(QChar(7));
        }
    }
    QString ret = PyCore::getCore()->screenWaitForString(qstrlist,timeout,bcaseInsensitive,self->matchIndex,self->id);
    return PyUnicode_FromString(ret.toUtf8().constData());
}

// Screen.Send(str: str) -> int
static PyObject* Screen_Send(Screen* self, PyObject* args) {
    const char* str;
    if (!PyArg_ParseTuple(args, "s", &str)) {
        return NULL;
    }
    QString qstr = QString::fromUtf8(str);
    int ret = PyCore::getCore()->screenSend(qstr,self->synchronous,self->id);
    return Py_BuildValue("i", ret);
}

// Screen.Clear() -> None
static PyObject* Screen_Clear(Screen* self) {
    PyCore::getCore()->screenClear(self->id);
    Py_RETURN_NONE;
}

// Screen.Get(row1: int, col1: int, row2: int, col2: int) -> str
static PyObject* Screen_Get(Screen* self, PyObject* args) {
    int row1, col1, row2, col2;
    if (!PyArg_ParseTuple(args, "iiii", &row1, &col1, &row2, &col2)) {
        return NULL;
    }
    QString ret = PyCore::getCore()->screenGet(row1, col1, row2, col2, self->id);
    return PyUnicode_FromString(ret.toUtf8().constData());
}

// Screen.Get2(row1: int, col1: int, row2: int, col2: int) -> str
static PyObject* Screen_Get2(Screen* self, PyObject* args) {
    int row1, col1, row2, col2;
    if (!PyArg_ParseTuple(args, "iiii", &row1, &col1, &row2, &col2)) {
        return NULL;
    }
    QString ret = PyCore::getCore()->screenGet2(row1, col1, row2, col2, self->id);
    return PyUnicode_FromString(ret.toUtf8().constData());
}

// Screen.IgnoreCase(enable: bool) -> None
static PyObject* Screen_IgnoreCase(Screen* self, PyObject* args, PyObject* kwargs) {
    static const char* kwlist[] = {"enable", NULL};
    int enable = true;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|i", (char**)kwlist, &enable)) {
        return NULL;
    }
    self->ignoreCase = enable;
    Py_RETURN_NONE;
}

// Screen.Print() -> None
static PyObject* Screen_Print(Screen* self) {
    PyCore::getCore()->screenPrint(self->id);
    Py_RETURN_NONE;
}

// Screen.Shortcut(path: str) -> None
static PyObject* Screen_Shortcut(Screen* self, PyObject* args) {
    const char* path;
    if (!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }
    QString qpath = QString::fromUtf8(path);
    PyCore::getCore()->screenShortcut(qpath,self->id);
    Py_RETURN_NONE;
}

// Screen.SendKeys(keylist: list[str]) -> None
static PyObject* Screen_SendKeys(Screen* self, PyObject* args) {
    PyObject* keylist;
    if (!PyArg_ParseTuple(args, "O", &keylist)) {
        return NULL;
    }
    if (!PyList_Check(keylist)) {
        PyCore::CrtScriptErrorException_SetMessage("The keylist parameter must be a list of strings");
        PyErr_SetString(PyCore::getQuardCRTException(), "The keylist parameter must be a list of strings");
        return NULL;
    } else {
        QStringList qkeylist;
        int size = PyList_Size(keylist);
        if(size >= 5) {
            PyCore::CrtScriptErrorException_SetMessage("The keylist parameter must be less than 5");
            PyErr_SetString(PyCore::getQuardCRTException(), "The keylist parameter must be less than 5");
            return NULL;
        }
        for (int i = 0; i < size; i++) {
            PyObject* item = PyList_GetItem(keylist, i);
            if (!PyUnicode_Check(item)) {
                PyCore::CrtScriptErrorException_SetMessage("The keylist parameter must be a list of strings");
                PyErr_SetString(PyCore::getQuardCRTException(), "The keylist parameter must be a list of strings");
                return NULL;
            }
            const char* key = PyUnicode_AsUTF8(item);
            qkeylist.append(QString::fromUtf8(key));
        }
        PyCore::getCore()->screenSendKeys(qkeylist,self->id);
    }
    Py_RETURN_NONE;
}

static PyObject* Screen_ReadString(Screen* self, PyObject* args, PyObject* kwargs) {
    static const char* kwlist[] = {"strlist", "timeout", "bcaseInsensitive", NULL};
    PyObject* strlist;
    int timeout = 0;
    bool bcaseInsensitive = self->ignoreCase;
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|ip", (char**)kwlist, &strlist, &timeout, &bcaseInsensitive)) {
        return NULL;
    }
    QStringList qstrlist;
    if (!PyList_Check(strlist)) {
        PyCore::CrtScriptErrorException_SetMessage("The strlist parameter must be a list of strings");
        PyErr_SetString(PyCore::getQuardCRTException(), "The strlist parameter must be a list of strings");
        return NULL;
    } else {
        int size = PyList_Size(strlist);
        for (int i = 0; i < size; i++) {
            PyObject* item = PyList_GetItem(strlist, i);
            if (!PyUnicode_Check(item)) {
                PyCore::CrtScriptErrorException_SetMessage("The strlist parameter must be a list of strings");
                PyErr_SetString(PyCore::getQuardCRTException(), "The strlist parameter must be a list of strings");
                return NULL;
            }
            const char* str = PyUnicode_AsUTF8(item);
            qstrlist.append(QString::fromUtf8(str));
        }
    }
    if(self->ignoreEscape) {
        foreach (QString str, qstrlist) {
            str.remove(QChar(27));
            str.remove(QChar(7));
        }
    }
    QString ret = PyCore::getCore()->screenReadString(qstrlist,timeout,bcaseInsensitive,self->matchIndex,self->id);
    return PyUnicode_FromString(ret.toUtf8().constData());
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
static PyMethodDef Screen_methods[] = {
    {"WaitForString", (PyCFunction)Screen_WaitForString, METH_VARARGS|METH_KEYWORDS, "WaitForString"},
    {"WaitForStrings", (PyCFunction)Screen_WaitForStrings, METH_VARARGS|METH_KEYWORDS, "WaitForStrings"},
    {"Send", (PyCFunction)Screen_Send, METH_VARARGS, "Send"},
    {"Clear", (PyCFunction)Screen_Clear, METH_NOARGS, "Clear"},
    {"Get", (PyCFunction)Screen_Get, METH_VARARGS, "Get"},
    {"Get2", (PyCFunction)Screen_Get2, METH_VARARGS, "Get2"},
    {"IgnoreCase", (PyCFunction)Screen_IgnoreCase, METH_VARARGS|METH_KEYWORDS, "IgnoreCase"},
    {"Print", (PyCFunction)Screen_Print, METH_NOARGS, "Print"},
    {"Shortcut", (PyCFunction)Screen_Shortcut, METH_VARARGS, "Shortcut"},
    {"SendKeys", (PyCFunction)Screen_SendKeys, METH_VARARGS, "SendKeys"},
    {"ReadString", (PyCFunction)Screen_ReadString, METH_VARARGS|METH_KEYWORDS, "ReadString"},
    // TODO:not implemented
    // "WaitForCursor" 
    // "WaitForKey" 
    {NULL,NULL,0,NULL}  // Sentinel
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

static PyObject* Screen_getSynchronous(Screen* self, void* closure) {
    Q_UNUSED(closure);
    return Py_BuildValue("i", self->synchronous);
}

static int Screen_setSynchronous(Screen* self, PyObject* value, void* closure) {
    Q_UNUSED(closure);
    if (value == NULL) {
        PyCore::CrtScriptErrorException_SetMessage("Cannot delete the Synchronous attribute");
        PyErr_SetString(PyCore::getQuardCRTException(), "Cannot delete the Synchronous attribute");
        return -1;
    }
    if (!PyLong_Check(value)) {
        PyCore::CrtScriptErrorException_SetMessage("The Synchronous attribute value must be an integer");
        PyErr_SetString(PyCore::getQuardCRTException(), "The Synchronous attribute value must be an integer");
        return -1;
    }
    self->synchronous = (bool)PyLong_AsLong(value);
    return 0;
}

static PyObject* Screen_getCurrentColumn(Screen* self, void* closure) {
    Q_UNUSED(closure);
    int ret = PyCore::getCore()->screenGetCurrentColumn(self->id);
    return Py_BuildValue("i", ret);
}

static PyObject* Screen_getCurrentRow(Screen* self, void* closure) {
    Q_UNUSED(closure);
    int ret = PyCore::getCore()->screenGetCurrentRow(self->id);
    return Py_BuildValue("i", ret);
}

static PyObject* Screen_getRows(Screen* self, void* closure) {
    Q_UNUSED(closure);
    int ret = PyCore::getCore()->screenGetRows(self->id);
    return Py_BuildValue("i", ret);
}

static PyObject* Screen_getColumns(Screen* self, void* closure) {
    Q_UNUSED(closure);
    int ret = PyCore::getCore()->screenGetColumns(self->id);
    return Py_BuildValue("i", ret);
}

static PyObject* Screen_getIgnoreEscape(Screen* self, void* closure) {
    Q_UNUSED(closure);
    return Py_BuildValue("i", self->ignoreEscape);
}

static int Screen_setIgnoreEscape(Screen* self, PyObject* value, void* closure) {
    Q_UNUSED(closure);
    if (value == NULL) {
        PyCore::CrtScriptErrorException_SetMessage("Cannot delete the IgnoreEscape attribute");
        PyErr_SetString(PyCore::getQuardCRTException(), "Cannot delete the IgnoreEscape attribute");
        return -1;
    }
    if (!PyLong_Check(value)) {
        PyCore::CrtScriptErrorException_SetMessage("The IgnoreEscape attribute value must be an integer");
        PyErr_SetString(PyCore::getQuardCRTException(), "The IgnoreEscape attribute value must be an integer");
        return -1;
    }
    self->ignoreEscape = (bool)PyLong_AsLong(value);
    return 0;
}

static PyObject* Screen_getMatchIndex(Screen* self, void* closure) {
    Q_UNUSED(closure);
    return Py_BuildValue("i", self->matchIndex);
}

static PyObject* Screen_getSelection(Screen* self, void* closure) {
    Q_UNUSED(closure);
    QString ret = PyCore::getCore()->screenGetSelection(self->id);
    return PyUnicode_FromString(ret.toUtf8().constData());
}

static PyObject* Screen_getId(Screen* self, void* closure) {
    Q_UNUSED(closure);
    int id = self->id;
    if(id == -1) {
        id = PyCore::getCore()->getActiveSessionId();
    }
    return Py_BuildValue("i", id);
}

static PyGetSetDef Screen_getseters[] = {
    {"Synchronous", (getter)Screen_getSynchronous, (setter)Screen_setSynchronous, "Synchronous", NULL},
    {"CurrentColumn", (getter)Screen_getCurrentColumn, NULL, "CurrentColumn", NULL},
    {"CurrentRow", (getter)Screen_getCurrentRow, NULL, "CurrentRow", NULL},
    {"Rows", (getter)Screen_getRows, NULL, "Rows", NULL},
    {"Columns", (getter)Screen_getColumns, NULL, "Rows", NULL},
    {"IgnoreEscape",(getter)Screen_getIgnoreEscape,(setter)Screen_setIgnoreEscape,"IgnoreEscape",NULL},
    {"MatchIndex",(getter)Screen_getMatchIndex,NULL,"MatchIndex",NULL},
    {"Selection",(getter)Screen_getSelection,NULL,"Selection",NULL},
    {"Id",(getter)Screen_getId,NULL,"Id",NULL},
    {NULL,NULL,NULL,NULL,NULL}  /* Sentinel */
};

static PyObject* Screen_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    Screen* self;
    int id = -1;
    static const char* kwlist[] = {"id", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", (char**)kwlist, &id)) {
        return NULL;
    }

    self = (Screen*)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->id = id;
        self->synchronous = true;
        self->ignoreEscape = false;
        self->ignoreCase = false;
        self->matchIndex = -1;
    }

    return (PyObject*)self;
}

static void Screen_dealloc(PyObject* ptr) {
    Screen* self = (Screen*)ptr;
    Py_TYPE(self)->tp_free(ptr);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
PyTypeObject ScreenType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "ScreenClass",          /* tp_name */
    sizeof(Screen),         /* tp_basicsize */
    0,                      /* tp_itemsize */
    Screen_dealloc,         /* tp_dealloc */
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
    "Screen Class",         /* tp_doc */
    0,                      /* tp_traverse */
    0,                      /* tp_clear */
    0,                      /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    Screen_methods,         /* tp_methods */
    0,                      /* tp_members */
    Screen_getseters,       /* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    0,                      /* tp_descr_get */
    0,                      /* tp_descr_set */
    0,                      /* tp_dictoffset */
    0,                      /* tp_init */
    0,                      /* tp_alloc */
    Screen_new,             /* tp_new */
    0,                      /* tp_free */
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

}
