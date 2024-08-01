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
#include "pyfiletransfer.h"

namespace PyFileTransfer {

typedef struct {
    PyObject_HEAD
} Filetransfer;

static PyObject* Filetransfer_getDownloadFolder(Filetransfer* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    QString text = PyCore::getCore()->getDownloadFolder();
    return PyUnicode_FromString(text.toUtf8().constData());
}

static PyGetSetDef Filetransfer_getseters[] = {
    {"DownloadFolder", (getter)Filetransfer_getDownloadFolder, NULL, "DownloadFolder", NULL},
    {NULL,NULL,NULL,NULL,NULL}  /* Sentinel */
};

// Filetransfer.AddToUploadList(path: str) -> None
static PyObject* Filetransfer_AddToUploadList(Filetransfer* self, PyObject* args) {
    Q_UNUSED(self);
    const char* path;
    if(!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }

    PyCore::getCore()->addToUploadList(QString::fromUtf8(path));
    Py_RETURN_NONE;
}

// Filetransfer.ClearUploadList() -> None
static PyObject* Filetransfer_ClearUploadList(Filetransfer* self, PyObject* args) {
    Q_UNUSED(self);Q_UNUSED(args);
    PyCore::getCore()->clearUploadList();
    Py_RETURN_NONE;
}

// Filetransfer.ReceiveKermit() -> int
static PyObject* Filetransfer_ReceiveKermit(Filetransfer* self, PyObject* args) {
    Q_UNUSED(self);Q_UNUSED(args);
    return Py_BuildValue("i", PyCore::getCore()->receiveKermit());
}

// Filetransfer.SendKermit(path: str) -> int
static PyObject* Filetransfer_SendKermit(Filetransfer* self, PyObject* args) {
    Q_UNUSED(self);
    const char* path;
    if(!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }

    return Py_BuildValue("i", PyCore::getCore()->sendKermit(QString::fromUtf8(path)));
}

// Filetransfer.ReceiveXmodem(path: str) -> int
static PyObject* Filetransfer_ReceiveXmodem(Filetransfer* self, PyObject* args) {
    Q_UNUSED(self);
    const char* path;
    if(!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }

    return Py_BuildValue("i", PyCore::getCore()->receiveXmodem(QString::fromUtf8(path)));
}

// Filetransfer.SendXmodem(path: str) -> int
static PyObject* Filetransfer_SendXmodem(Filetransfer* self, PyObject* args) {
    Q_UNUSED(self);
    const char* path;
    if(!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }

    return Py_BuildValue("i", PyCore::getCore()->sendXmodem(QString::fromUtf8(path)));
}

// Filetransfer.ReceiveYmodem() -> int
static PyObject* Filetransfer_ReceiveYmodem(Filetransfer* self, PyObject* args) {
    Q_UNUSED(self);Q_UNUSED(args);
    return Py_BuildValue("i", PyCore::getCore()->receiveYmodem());
}

// Filetransfer.SendYmodem(path: str) -> int
static PyObject* Filetransfer_SendYmodem(Filetransfer* self, PyObject* args) {
    Q_UNUSED(self);
    const char* path;
    if(!PyArg_ParseTuple(args, "s", &path)) {
        return NULL;
    }

    return Py_BuildValue("i", PyCore::getCore()->sendYmodem(QString::fromUtf8(path)));
}

// Filetransfer.SendZmodem() -> int
static PyObject* Filetransfer_SendZmodem(Filetransfer* self, PyObject* args) {
    Q_UNUSED(self);Q_UNUSED(args);
    return Py_BuildValue("i", PyCore::getCore()->sendZmodem());
}

static PyMethodDef Filetransfer_methods[] = {
    {"AddToUploadList", (PyCFunction)Filetransfer_AddToUploadList, METH_VARARGS, "AddToUploadList"},
    {"ClearUploadList", (PyCFunction)Filetransfer_ClearUploadList, METH_NOARGS, "ClearUploadList"},
    {"ReceiveKermit", (PyCFunction)Filetransfer_ReceiveKermit, METH_NOARGS, "ReceiveKermit"},
    {"SendKermit", (PyCFunction)Filetransfer_SendKermit, METH_VARARGS, "SendKermit"},
    {"ReceiveXmodem", (PyCFunction)Filetransfer_ReceiveXmodem, METH_VARARGS, "ReceiveXmodem"},
    {"SendXmodem", (PyCFunction)Filetransfer_SendXmodem, METH_VARARGS, "SendXmodem"},
    {"ReceiveYmodem", (PyCFunction)Filetransfer_ReceiveYmodem, METH_NOARGS, "ReceiveYmodem"},
    {"SendYmodem", (PyCFunction)Filetransfer_SendYmodem, METH_VARARGS, "SendYmodem"},
    {"SendZmodem", (PyCFunction)Filetransfer_SendZmodem, METH_NOARGS, "SendZmodem"},
    {NULL,NULL,0,NULL}  // Sentinel
};

static PyObject* Filetransfer_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    Filetransfer* self;
    Q_UNUSED(args);Q_UNUSED(kwds);
    self = (Filetransfer*)type->tp_alloc(type, 0);

    return (PyObject*)self;
}

static void Filetransfer_dealloc(PyObject* ptr) {
    Filetransfer* self = (Filetransfer*)ptr;
    Py_TYPE(self)->tp_free(ptr);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
PyTypeObject FileTransferType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "FiletransferClass",    /* tp_name */
    sizeof(Filetransfer),   /* tp_basicsize */
    0,                      /* tp_itemsize */
    Filetransfer_dealloc,   /* tp_dealloc */
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
    "Filetransfer Class",   /* tp_doc */
    0,                      /* tp_traverse */
    0,                      /* tp_clear */
    0,                      /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    Filetransfer_methods,   /* tp_methods */
    0,                      /* tp_members */
    Filetransfer_getseters, /* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    0,                      /* tp_descr_get */
    0,                      /* tp_descr_set */
    0,                      /* tp_dictoffset */
    0,                      /* tp_init */
    0,                      /* tp_alloc */
    Filetransfer_new,       /* tp_new */
    0,                      /* tp_free */
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

}
