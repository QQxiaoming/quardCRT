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
#include "pyarguments.h"

namespace PyArguments {

typedef struct {
    PyObject_HEAD
    int count;
    QStringList arguments;
} Arguments;

static PyObject* Arguments_getCount(Arguments* self, void* closure) {
    Q_UNUSED(self);Q_UNUSED(closure);
    return Py_BuildValue("i", self->count);
}

static PyGetSetDef Arguments_getseters[] = {
    {"Count", (getter)Arguments_getCount, NULL, "Count", NULL},
    {NULL,NULL,NULL,NULL,NULL}  /* Sentinel */
};

// Arguments.GetArg(idx: int) -> str
static PyObject* Arguments_GetArg(Arguments* self, PyObject* args) {
    int idx;
    if(!PyArg_ParseTuple(args, "i", &idx)) {
        return NULL;
    }

    if(idx < 0 || idx >= self->count) {
        PyCore::CrtScriptErrorException_SetMessage("Index out of range");
        PyErr_SetString(PyCore::getQuardCRTException(), "Index out of range");
        return NULL;
    }

    return Py_BuildValue("s", self->arguments.at(idx).toUtf8().constData());
}

static Py_ssize_t Arguments_len(Arguments* self) {
    return self->count;
}

static PyObject* Arguments___getitem__(Arguments* self, Py_ssize_t idx) {
    if(idx < 0 || idx >= self->count) {
        PyCore::CrtScriptErrorException_SetMessage("Index out of range");
        PyErr_SetString(PyCore::getQuardCRTException(), "Index out of range");
        return NULL;
    }
    return Py_BuildValue("s", self->arguments.at(idx).toUtf8().constData());
}

static PyMethodDef Arguments_methods[] = {
    {"GetArg", (PyCFunction)Arguments_GetArg, METH_VARARGS, "GetArg"},
    {NULL,NULL,0,NULL}  // Sentinel
};

static PySequenceMethods Arguments_as_sequence = {
    (lenfunc)Arguments_len, /* sq_length */
    0, /* sq_concat */
    0, /* sq_repeat */
    (ssizeargfunc)Arguments___getitem__, /* sq_item */
    0, /* sq_slice */
    0, /* sq_ass_item */
    0, /* sq_ass_slice */
    0, /* sq_contains */
    0, /* sq_inplace_concat */
    0, /* sq_inplace_repeat */
};

static PyObject* Arguments_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    Arguments* self;
    Q_UNUSED(args);Q_UNUSED(kwds);
    self = (Arguments*)type->tp_alloc(type, 0);
    if(self != NULL) {
        self->arguments = PyCore::getCore()->getArguments();
        self->count = self->arguments.count();
    }

    return (PyObject*)self;
}

static void Arguments_dealloc(PyObject* ptr) {
    Arguments* self = (Arguments*)ptr;
    Py_TYPE(self)->tp_free(ptr);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
PyTypeObject ArgumentsType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "ArgumentsClass",       /* tp_name */
    sizeof(Arguments),      /* tp_basicsize */
    0,                      /* tp_itemsize */
    Arguments_dealloc,      /* tp_dealloc */
    0,                      /* tp_print */
    0,                      /* tp_getattr */
    0,                      /* tp_setattr */
    0,                      /* tp_reserved */
    0,                      /* tp_repr */
    0,                      /* tp_as_number */
    &Arguments_as_sequence, /* tp_as_sequence */
    0,                      /* tp_as_mapping */
    0,                      /* tp_hash  */
    0,                      /* tp_call */
    0,                      /* tp_str */
    0,                      /* tp_getattro */
    0,                      /* tp_setattro */
    0,                      /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,     /* tp_flags */
    "Arguments Class",      /* tp_doc */
    0,                      /* tp_traverse */
    0,                      /* tp_clear */
    0,                      /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    Arguments_methods,      /* tp_methods */
    0,                      /* tp_members */
    Arguments_getseters,    /* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    0,                      /* tp_descr_get */
    0,                      /* tp_descr_set */
    0,                      /* tp_dictoffset */
    0,                      /* tp_init */
    0,                      /* tp_alloc */
    Arguments_new,          /* tp_new */
    0,                      /* tp_free */
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

}
