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
#include "pycommandwindow.h"

namespace PyCommandWindow {

typedef struct {
    PyObject_HEAD
    bool sendCharactersImmediately;
    bool sendToAllSessions;
    bool visible;
} CommandWindow;

static PyObject* CommandWindow_getSendCharactersImmediately(CommandWindow* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    return PyBool_FromLong(self->sendCharactersImmediately);
}

static int CommandWindow_setSendCharactersImmediately(CommandWindow* self, PyObject* value, void* closure) {
    Q_UNUSED(closure);
    if(!PyBool_Check(value)) {
        PyCore::CrtScriptErrorException_SetMessage("The value must be a boolean.");
        PyErr_SetString(PyCore::getQuardCRTException(), "The value must be a boolean.");
        return -1;
    }
    self->sendCharactersImmediately = (value == Py_True);
    return 0;
}

static PyObject* CommandWindow_getSendToAllSessions(CommandWindow* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    return PyBool_FromLong(self->sendToAllSessions);
}

static int CommandWindow_setSendToAllSessions(CommandWindow* self, PyObject* value, void* closure) {
    Q_UNUSED(closure);
    if(!PyBool_Check(value)) {
        PyCore::CrtScriptErrorException_SetMessage("The value must be a boolean.");
        PyErr_SetString(PyCore::getQuardCRTException(), "The value must be a boolean.");
        return -1;
    }
    self->sendToAllSessions = (value == Py_True);
    return 0;
}

static PyObject* CommandWindow_getVisible(CommandWindow* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    self->visible = PyCore::getCore()->getCommandWindowVisibled();
    return PyBool_FromLong(self->visible);
}

static int CommandWindow_setVisible(CommandWindow* self, PyObject* value, void* closure) {
    Q_UNUSED(closure);
    if(!PyBool_Check(value)) {
        PyCore::CrtScriptErrorException_SetMessage("The value must be a boolean.");
        PyErr_SetString(PyCore::getQuardCRTException(), "The value must be a boolean.");
        return -1;
    }
    self->visible = (value == Py_True);
    PyCore::getCore()->setCommandWindowVisibled(self->visible);
    return 0;
}

static PyObject* CommandWindow_getText(CommandWindow* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    QString text = PyCore::getCore()->getCommandWindowText();
    return PyUnicode_FromString(text.toUtf8().constData());
}

static int CommandWindow_setText(CommandWindow* self, PyObject* value, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    if(!PyUnicode_Check(value)) {
        PyCore::CrtScriptErrorException_SetMessage("The value must be a string.");
        PyErr_SetString(PyCore::getQuardCRTException(), "The value must be a string.");
        return -1;
    }
    const char* input = PyUnicode_AsUTF8(value);
    QString text = QString::fromUtf8(input);
    PyCore::getCore()->setCommandWindowText(text);
    if(self->sendCharactersImmediately)
        PyCore::getCore()->commandWindowSend();
    return 0;
}

static PyGetSetDef CommandWindow_getseters[] = {
    {"SendCharactersImmediately", (getter)CommandWindow_getSendCharactersImmediately, (setter)CommandWindow_setSendCharactersImmediately, "SendCharactersImmediately", NULL},
    {"SendToAllSessions", (getter)CommandWindow_getSendToAllSessions, (setter)CommandWindow_setSendToAllSessions, "SendToAllSessions", NULL},
    {"Visible", (getter)CommandWindow_getVisible, (setter)CommandWindow_setVisible, "Visible", NULL},
    {"Text", (getter)CommandWindow_getText, (setter)CommandWindow_setText, "Text", NULL},
    {NULL,NULL,NULL,NULL,NULL}  /* Sentinel */
};

// CommandWindow.Send() -> None
static PyObject* CommandWindow_Send(CommandWindow* self, PyObject* args) {
    Q_UNUSED(args);Q_UNUSED(self);
    PyCore::getCore()->commandWindowSend();
    Py_RETURN_NONE;
}

static PyMethodDef CommandWindow_methods[] = {
    {"Send", (PyCFunction)CommandWindow_Send, METH_NOARGS, "Send"},
    {NULL,NULL,0,NULL}  // Sentinel
};

static PyObject* CommandWindow_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    CommandWindow* self;
    Q_UNUSED(args);Q_UNUSED(kwds);
    self = (CommandWindow*)type->tp_alloc(type, 0);
    if(self != NULL) {
        self->sendCharactersImmediately = false;
        self->sendToAllSessions = false;
        self->visible = false;
    }
    return (PyObject*)self;
}

static void CommandWindow_dealloc(PyObject* ptr) {
    CommandWindow* self = (CommandWindow*)ptr;
    Py_TYPE(self)->tp_free(ptr);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
PyTypeObject CommandWindowType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "CommandWindowClass",   /* tp_name */
    sizeof(CommandWindow),  /* tp_basicsize */
    0,                      /* tp_itemsize */
    CommandWindow_dealloc,  /* tp_dealloc */
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
    "CommandWindow Class",  /* tp_doc */
    0,                      /* tp_traverse */
    0,                      /* tp_clear */
    0,                      /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    CommandWindow_methods,  /* tp_methods */
    0,                      /* tp_members */
    CommandWindow_getseters,/* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    0,                      /* tp_descr_get */
    0,                      /* tp_descr_set */
    0,                      /* tp_dictoffset */
    0,                      /* tp_init */
    0,                      /* tp_alloc */
    CommandWindow_new,      /* tp_new */
    0,                      /* tp_free */
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

}
