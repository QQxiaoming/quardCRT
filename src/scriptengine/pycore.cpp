/*
 * This file is part of the https://github.com/QQxiaoming/quardCRT.git
 * project.
 *
 * Copyright (C) 2024 Quard <2014500726@smail.xtu.edu.cn>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#include "pydialog.h"
#include "pysession.h"
#include "pyscreen.h"
#include "pytab.h"
#include "pywindow.h"
#include "pyarguments.h"
#include "pyclipboard.h"
#include "pyfiletransfer.h"
#include "pycommandwindow.h"
#include "pycore.h"

#include <QApplication>
#include <QThread>

namespace PyCore {

static PyObject *CrtScriptErrorException;

static PyRun* pyRun = nullptr;

void CrtScriptErrorException_SetMessage(const char* errorMessage) {
    if (errorMessage) {
        PyObject *message = PyUnicode_FromString(errorMessage);
        if (message != NULL) {
            PyObject_SetAttrString(CrtScriptErrorException, "message", message);
            Py_DECREF(message);
        }
    } else {
        PyObject *message = PyUnicode_FromString("");
        if (message != NULL) {
            PyObject_SetAttrString(CrtScriptErrorException, "message", message);
            Py_DECREF(message);
        }
    }
}

int stop_script(...) {
    if(pyRun->isStopScript()) {
        PyErr_SetString(CrtScriptErrorException, "Stop script");
        return -1;
    }
    return 0;
}

typedef struct {
    PyObject_HEAD
    PyObject *dialog;
    PyObject *activeSession;
    PyObject *activeScreen;
    PyObject *window;
    PyObject *arguments;
    PyObject *clipboard;
    PyObject *fileTransfer;
    PyObject *commandWindow;
    QString activePrinter;
    QString scriptFullName;
    QString version;
} QuardCRT;

static PyObject* QuardCRT_getdialog(QuardCRT* self, void* closure) {
    Py_INCREF(self->dialog);
    Q_UNUSED(closure);
    return self->dialog;
}

static PyObject* QuardCRT_getsession(QuardCRT* self, void* closure) {
    Py_INCREF(self->activeSession);
    Q_UNUSED(closure);
    return self->activeSession;
}

static PyObject* QuardCRT_getscreen(QuardCRT* self, void* closure) {
    Py_INCREF(self->activeScreen);
    Q_UNUSED(closure);
    return self->activeScreen;
}

static PyObject* QuardCRT_getwindow(QuardCRT* self, void* closure) {
    Py_INCREF(self->window);
    Q_UNUSED(closure);
    return self->window;
}

static PyObject* QuardCRT_getarguments(QuardCRT* self, void* closure) {
    Py_INCREF(self->arguments);
    Q_UNUSED(closure);
    return self->arguments;
}

static PyObject* QuardCRT_getclipboard(QuardCRT* self, void* closure) {
    Py_INCREF(self->clipboard);
    Q_UNUSED(closure);
    return self->clipboard;
}

static PyObject* QuardCRT_getfileTransfer(QuardCRT* self, void* closure) {
    Py_INCREF(self->fileTransfer);
    Q_UNUSED(closure);
    return self->fileTransfer;
}

static PyObject* QuardCRT_getcommandWindow(QuardCRT* self, void* closure) {
    Py_INCREF(self->commandWindow);
    Q_UNUSED(closure);
    return self->commandWindow;
}

static PyObject* QuardCRT_getversion(QuardCRT* self, void* closure) {
    QByteArray ba = self->version.toUtf8();
    Q_UNUSED(closure);
    return Py_BuildValue("s", ba.constData());
}

static PyObject* QuardCRT_getactivePrinter(QuardCRT* self, void* closure) {
    self->activePrinter = pyRun->getActivePrinter();
    QByteArray ba = self->activePrinter.toUtf8();
    Q_UNUSED(closure);
    return Py_BuildValue("s", ba.constData());
}

static int QuardCRT_setactivePrinter(QuardCRT* self, PyObject* value, void* closure) {
    Q_UNUSED(closure);
    if (value == NULL) {
        CrtScriptErrorException_SetMessage("Cannot delete the ActivePrinter attribute");
        PyErr_SetString(CrtScriptErrorException, "Cannot delete the ActivePrinter attribute");
        return -1;
    }
    if (!PyUnicode_Check(value)) {
        CrtScriptErrorException_SetMessage("The ActivePrinter attribute value must be a string");
        PyErr_SetString(CrtScriptErrorException, "The ActivePrinter attribute value must be a string");
        return -1;
    }
    self->activePrinter = PyUnicode_AsUTF8(value);
    pyRun->setActivePrinter(self->activePrinter);
    return 0;
}

static PyObject* QuardCRT_getscriptFullName(QuardCRT* self, void* closure) {
    self->scriptFullName = pyRun->getScriptFullName();
    QByteArray ba = self->scriptFullName.toUtf8();
    Q_UNUSED(closure);
    return Py_BuildValue("s", ba.constData());
}

static PyGetSetDef QuardCRT_getseters[] = {
    {"ActivePrinter", (getter)QuardCRT_getactivePrinter, (setter)QuardCRT_setactivePrinter, "ActivePrinter", NULL},
    {"Dialog", (getter)QuardCRT_getdialog, NULL, "Dialog", NULL},
    {"Screen", (getter)QuardCRT_getscreen, NULL, "Screen", NULL},
    {"Window", (getter)QuardCRT_getwindow, NULL, "Window", NULL},
    {"Arguments", (getter)QuardCRT_getarguments, NULL, "Arguments", NULL},
    {"Clipboard", (getter)QuardCRT_getclipboard, NULL, "Clipboard", NULL},
    {"FileTransfer", (getter)QuardCRT_getfileTransfer, NULL, "FileTransfer", NULL},
    {"CommandWindow", (getter)QuardCRT_getcommandWindow, NULL, "CommandWindow", NULL},
    {"ScriptFullName", (getter)QuardCRT_getscriptFullName, NULL, "ScriptFullName", NULL},
    {"Session", (getter)QuardCRT_getsession, NULL, "Session", NULL},
    {"Version", (getter)QuardCRT_getversion, NULL, "Version", NULL},
    {NULL,NULL,NULL,NULL,NULL}  /* Sentinel */
};

// crt.GetActiveTab() -> Obj:Tab
static PyObject* QuardCRT_GetActiveTab(QuardCRT* self, PyObject* args) {
    Q_UNUSED(self);Q_UNUSED(args);
    int activeTabId = pyRun->getActiveTabId();
    if(activeTabId<0) {
        CrtScriptErrorException_SetMessage("No active tab found");
        PyErr_SetString(CrtScriptErrorException, "No active tab found");
        return NULL;
    }
    PyObject *tabNameArg = Py_BuildValue("i", activeTabId);
    Py_INCREF(tabNameArg);
    PyObject *arg = PyTuple_Pack(1, tabNameArg);
    Py_XDECREF(tabNameArg);
    Py_INCREF(arg);
    PyObject *tab = PyObject_CallObject((PyObject *)&PyTab::TabType, arg);
    Py_XDECREF(arg);
    //Py_INCREF(tab);
    return tab;
}

// crt.GetLastError() -> Obj:errcode
static PyObject* QuardCRT_GetLastError(QuardCRT* self, PyObject* args) {
    Q_UNUSED(self);Q_UNUSED(args);
    Py_INCREF(CrtScriptErrorException);
    return CrtScriptErrorException;
}

// crt.GetLastErrorMessage() -> str
static PyObject* QuardCRT_GetLastErrorMessage(QuardCRT* self, PyObject* args) {
    Q_UNUSED(self);Q_UNUSED(args);
    const char *str = "";
    PyObject *message = PyObject_GetAttrString(CrtScriptErrorException, "message");
    if (message != NULL) {
        str = PyUnicode_AsUTF8(message);
        Py_XDECREF(message);
    }
    return Py_BuildValue("s", str);
}

// crt.ClearLastError() -> None
static PyObject* QuardCRT_ClearLastError(QuardCRT* self, PyObject* args) {
    Q_UNUSED(args);Q_UNUSED(self);
    PyErr_Clear();
    CrtScriptErrorException_SetMessage(NULL);
    Py_RETURN_NONE;
}

// crt.Sleep(millisec:int) -> None
static PyObject* QuardCRT_Sleep(QuardCRT* self, PyObject* args) {
    int millisec;
    Q_UNUSED(self);
    if (!PyArg_ParseTuple(args, "i", &millisec)) {
        CrtScriptErrorException_SetMessage("Invalid argument");
        PyErr_SetString(CrtScriptErrorException, "Invalid argument");
        return NULL;
    }
    QThread::msleep(millisec);
    Py_RETURN_NONE;
}

// crt.Quit() -> None
static PyObject* QuardCRT_Quit(QuardCRT* self, PyObject* args) {
    Q_UNUSED(args);Q_UNUSED(self);
    qApp->quit();
    Py_RETURN_NONE;
}

// crt.Cmd(cmd:str, [args:list]) -> str
static PyObject* QuardCRT_Cmd(QuardCRT* self, PyObject* args) {
    const char *cmd;
    PyObject *argList = NULL;
    Q_UNUSED(self);
    if (!PyArg_ParseTuple(args, "s|O", &cmd, &argList)) {
        CrtScriptErrorException_SetMessage("Invalid argument");
        PyErr_SetString(CrtScriptErrorException, "Invalid argument");
        return NULL;
    }
    if (argList != NULL && !PyList_Check(argList)) {
        CrtScriptErrorException_SetMessage("Invalid argument");
        PyErr_SetString(CrtScriptErrorException, "Invalid argument");
        return NULL;
    }
    QStringList arguments;
    if (argList != NULL) {
        Py_ssize_t size = PyList_Size(argList);
        for (Py_ssize_t i = 0; i < size; i++) {
            PyObject *item = PyList_GetItem(argList, i);
            if (PyUnicode_Check(item)) {
                const char *arg = PyUnicode_AsUTF8(item);
                arguments.append(arg);
            } else {
                CrtScriptErrorException_SetMessage("Invalid argument");
                PyErr_SetString(CrtScriptErrorException, "Invalid argument");
                return NULL;
            }
        }
    }
    int ret = pyRun->runSpecialCmd(cmd, arguments);
    if(ret<0) {
        return Py_BuildValue("s", "Special command failed");
    }
    return Py_BuildValue("s", pyRun->getSpecialCmdResult().toUtf8().constData());
}

static PyMethodDef QuardCRT_methods[] = {
    {"ClearLastError", (PyCFunction)QuardCRT_ClearLastError, METH_NOARGS, "ClearLastError"},
    {"GetActiveTab", (PyCFunction)QuardCRT_GetActiveTab, METH_NOARGS, "GetActiveTab"},
    {"GetLastError", (PyCFunction)QuardCRT_GetLastError, METH_NOARGS, "GetLastError"},
    {"GetLastErrorMessage", (PyCFunction)QuardCRT_GetLastErrorMessage, METH_NOARGS, "GetLastErrorMessage"},
    {"Sleep", (PyCFunction)QuardCRT_Sleep, METH_VARARGS, "Sleep"},
    {"Quit", (PyCFunction)QuardCRT_Quit, METH_NOARGS, "Quit"},
    {"Cmd", (PyCFunction)QuardCRT_Cmd, METH_VARARGS, "Cmd"},
    {NULL,NULL,0,NULL}  // Sentinel
};

static PyObject* QuardCRT_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    QuardCRT* self;
    Q_UNUSED(args);Q_UNUSED(kwds);
    self = (QuardCRT*)type->tp_alloc(type, 0);
    if (self != NULL) {
        if (PyType_Ready(&PyDialog::DialogType) < 0)
            return NULL;
        if (PyType_Ready(&PySession::SessionType) < 0)
            return NULL;
        if (PyType_Ready(&PyScreen::ScreenType) < 0)
            return NULL;
        if (PyType_Ready(&PyWindow::WindowType) < 0)
            return NULL;
        if (PyType_Ready(&PyArguments::ArgumentsType) < 0)
            return NULL;
        if (PyType_Ready(&PyClipboard::ClipboardType) < 0)
            return NULL;
        if (PyType_Ready(&PyFileTransfer::FileTransferType) < 0)
            return NULL;
        if (PyType_Ready(&PyCommandWindow::CommandWindowType) < 0)
            return NULL;
        if (PyType_Ready(&PyTab::TabType) < 0)
            return NULL;

        Py_INCREF(&PyDialog::DialogType);
        Py_INCREF(&PyScreen::ScreenType);
        Py_INCREF(&PySession::SessionType);
        Py_INCREF(&PyWindow::WindowType);
        Py_INCREF(&PyArguments::ArgumentsType);
        Py_INCREF(&PyClipboard::ClipboardType);
        Py_INCREF(&PyFileTransfer::FileTransferType);
        Py_INCREF(&PyCommandWindow::CommandWindowType);
        Py_INCREF(&PyTab::TabType);

        self->dialog  = PyObject_CallObject((PyObject *)&PyDialog::DialogType, NULL);
        Py_INCREF(self->dialog);
        self->activeSession = PyObject_CallObject((PyObject *)&PySession::SessionType, NULL);
        Py_INCREF(self->activeSession);
        self->activeScreen = PyObject_CallObject((PyObject *)&PyScreen::ScreenType, NULL);
        Py_INCREF(self->activeScreen);
        self->window = PyObject_CallObject((PyObject *)&PyWindow::WindowType, NULL);
        Py_INCREF(self->window);
        self->arguments = PyObject_CallObject((PyObject *)&PyArguments::ArgumentsType, NULL);
        Py_INCREF(self->arguments);
        self->clipboard = PyObject_CallObject((PyObject *)&PyClipboard::ClipboardType, NULL);
        Py_INCREF(self->clipboard);
        self->fileTransfer = PyObject_CallObject((PyObject *)&PyFileTransfer::FileTransferType, NULL);
        Py_INCREF(self->fileTransfer);
        self->commandWindow = PyObject_CallObject((PyObject *)&PyCommandWindow::CommandWindowType, NULL);
        Py_INCREF(self->commandWindow);

        self->version = pyRun->getVersion();
        self->activePrinter = pyRun->getActivePrinter();
        self->scriptFullName = pyRun->getScriptFullName();
    }

    return (PyObject*)self;
}

static void QuardCRT_dealloc(PyObject* ptr) {
    QuardCRT* self = (QuardCRT*)ptr;
    Py_XDECREF(self->dialog);
    Py_XDECREF(self->activeSession);
    Py_XDECREF(self->activeScreen);
    Py_XDECREF(self->window);
    Py_XDECREF(self->arguments);
    Py_XDECREF(self->clipboard);
    Py_XDECREF(self->fileTransfer);
    Py_XDECREF(self->commandWindow);
    Py_XDECREF(&PyDialog::DialogType);
    Py_XDECREF(&PySession::SessionType);
    Py_XDECREF(&PyScreen::ScreenType);
    Py_XDECREF(&PyWindow::WindowType);
    Py_XDECREF(&PyArguments::ArgumentsType);
    Py_XDECREF(&PyClipboard::ClipboardType);
    Py_XDECREF(&PyFileTransfer::FileTransferType);
    Py_XDECREF(&PyCommandWindow::CommandWindowType);
    Py_XDECREF(&PyTab::TabType);
    Py_TYPE(self)->tp_free(ptr);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
static PyTypeObject QuardCRTType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "QuardCRTClass",        /* tp_name */
    sizeof(QuardCRT),       /* tp_basicsize */
    0,                      /* tp_itemsize */
    QuardCRT_dealloc,       /* tp_dealloc */
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
    "QuardCRT Class",       /* tp_doc */
    0,                      /* tp_traverse */
    0,                      /* tp_clear */
    0,                      /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    QuardCRT_methods,       /* tp_methods */
    0,                      /* tp_members */
    QuardCRT_getseters,     /* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    0,                      /* tp_descr_get */
    0,                      /* tp_descr_set */
    0,                      /* tp_dictoffset */
    0,                      /* tp_init */
    0,                      /* tp_alloc */
    QuardCRT_new,           /* tp_new */
    0,                      /* tp_free */
};

static struct PyModuleDef quardCRTmodule = {
    PyModuleDef_HEAD_INIT,
    "quardCRTmodule",   /* name of module */
    "QuardCRT module",  /* module documentation, may be NULL */
    -1,           /* size of per-interpreter state of the module,
                     or -1 if the module keeps state in global variables. */
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

PyMODINIT_FUNC PyInit_quardCRT(PyRun* parent) {
    pyRun = parent;
    if (PyType_Ready(&QuardCRTType) < 0)
        return NULL;

    PyObject* m = PyModule_Create(&quardCRTmodule);
    if (m == NULL)
        return NULL;

    PyObject *dict = PyDict_New();
    PyDict_SetItemString(dict, "message", PyUnicode_FromString(""));
    CrtScriptErrorException = PyErr_NewException("quardCRT.ScriptError", NULL, dict);
    Py_INCREF(CrtScriptErrorException);
    PyObject* builtins = PyImport_ImportModule("builtins");
    if (builtins != NULL) {
        PyModule_AddObject(builtins, "ScriptError", CrtScriptErrorException);
        Py_DECREF(builtins);
    }

    Py_INCREF(&QuardCRTType);
    PyObject *crt = PyObject_CallObject((PyObject *)&QuardCRTType, NULL);
    if(PyModule_AddObject(m, "crt", crt) < 0) {
        Py_XDECREF(crt);
        Py_XDECREF(&QuardCRTType);
        Py_XDECREF(m);
        return NULL;
    }
    return m;
}

void PyDeInit_quardCRT(PyObject* m) {
    Py_XDECREF(&QuardCRTType);
    Py_XDECREF(CrtScriptErrorException);
    Py_XDECREF(m);
}

PyRun* getCore(void) {
    return pyRun;
}

PyObject* getQuardCRTException(void) {
    return CrtScriptErrorException;
}

}
