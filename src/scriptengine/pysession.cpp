#include "pysession.h"

#include <QString>
#include <QMessageBox>

namespace PySession {

typedef struct {
    PyObject_HEAD
    int id;
} Session;

// Session.Connect(cmd: str) -> int
static PyObject* Session_Connect(Session* self, PyObject* args) {
    const char* cmd;
    Q_UNUSED(self);
    if (!PyArg_ParseTuple(args, "s", &cmd)) {
        return NULL;
    }
    QString cmdStr = QString::fromUtf8(cmd);
    int ret = PyCore::getCore()->sessionConnect(cmdStr,self->id);
    return Py_BuildValue("i", ret);
}

// Session.Disconnect() -> None
static PyObject* Session_Disconnect(Session* self) {
    Q_UNUSED(self);
    PyCore::getCore()->sessionDisconnect(self->id);
    Py_RETURN_NONE;
}

// Session.Log(enable: bool) -> None
static PyObject* Session_Log(Session* self, PyObject* args) {
    int enable;
    Q_UNUSED(self);
    if (!PyArg_ParseTuple(args, "i", &enable)) {
        return NULL;
    }
    PyCore::getCore()->sessionLog(enable,self->id);
    Py_RETURN_NONE;
}

// Session.Lock(prompt: str, password: str, lockallsessions: int) -> int
static PyObject* Session_Lock(Session* self, PyObject* args, PyObject* kwds) {
    const char* prompt = "";
    const char* password = "";
    int lockallsessions = 0;
    Q_UNUSED(self);
    static const char* kwlist[] = {"prompt", "password", "lockallsessions", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ssi", (char**)kwlist, &prompt, &password, &lockallsessions)) {
        return NULL;
    }
    QString promptStr = QString::fromUtf8(prompt);
    QString passwordStr = QString::fromUtf8(password);
    int ret = PyCore::getCore()->sessionLock(promptStr, passwordStr, lockallsessions,self->id);
    return Py_BuildValue("i", ret);
}

// Session.Unlock(prompt: str, password: str, lockallsessions: int) -> int
static PyObject* Session_Unlock(Session* self, PyObject* args, PyObject* kwds) {
    const char* prompt = "";
    const char* password = "";
    int lockallsessions = 0;
    Q_UNUSED(self);
    static const char* kwlist[] = {"prompt", "password", "lockallsessions", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|ssi", (char**)kwlist, &prompt, &password, &lockallsessions)) {
        return NULL;
    }
    QString promptStr = QString::fromUtf8(prompt);
    QString passwordStr = QString::fromUtf8(password);
    int ret = PyCore::getCore()->sessionUnlock(promptStr, passwordStr, lockallsessions,self->id);
    return Py_BuildValue("i", ret);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
static PyMethodDef Session_methods[] = {
    {"Connect", (PyCFunction)Session_Connect, METH_VARARGS, "Connect"},
    {"Disconnect", (PyCFunction)Session_Disconnect, METH_NOARGS, "Disconnect"},
    {"Log", (PyCFunction)Session_Log, METH_VARARGS, "Log"},
    {"Lock", (PyCFunction)Session_Lock, METH_VARARGS|METH_KEYWORDS, "Lock"},
    {"Unlock", (PyCFunction)Session_Unlock, METH_VARARGS|METH_KEYWORDS, "Unlock"},
    {NULL,NULL,0,NULL}  // Sentinel
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

static PyObject* Session_getLocked(Session* self, void* closure) {
    Q_UNUSED(self);
    Q_UNUSED(closure);
    bool ret = PyCore::getCore()->sessionGetLocked(self->id);
    return Py_BuildValue("i", ret);
}

static PyObject* Session_getConnected(Session* self, void* closure) {
    Q_UNUSED(self);
    Q_UNUSED(closure);
    bool ret = PyCore::getCore()->sessionGetConnected(self->id);
    return Py_BuildValue("i", ret);
}

static PyObject* Session_getLogging(Session* self, void* closure) {
    Q_UNUSED(self);
    Q_UNUSED(closure);
    bool ret = PyCore::getCore()->sessionGetLogging(self->id);
    return Py_BuildValue("i", ret);
}

static PyObject* Session_getId(Session* self, void* closure) {
    Q_UNUSED(closure);
    int id = self->id;
    if(id == -1) {
        id = PyCore::getCore()->getActiveSessionId();
    }
    return Py_BuildValue("i", id);
}

static PyGetSetDef Session_getseters[] = {
    {"Locked", (getter)Session_getLocked, NULL, "Locked", NULL},
    {"Connected", (getter)Session_getConnected, NULL, "Connected", NULL},
    {"Logging", (getter)Session_getLogging, NULL, "Logging", NULL},
    {"Id",(getter)Session_getId,NULL,"Id",NULL},
    // TODO:not implemented
    // "Config"
    {NULL,NULL,NULL,NULL,NULL}  // Sentinel
};

static PyObject* Session_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
    Session* self;
    int id = -1;
    static const char* kwlist[] = {"id", NULL};
    
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", (char**)kwlist, &id)) {
        return NULL;
    }
    
    self = (Session*)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->id = id;
    }

    return (PyObject*)self;
}

static void Session_dealloc(PyObject* ptr) {
    Session* self = (Session*)ptr;
    Py_TYPE(self)->tp_free(ptr);
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif
PyTypeObject SessionType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "SessionClass",         /* tp_name */
    sizeof(Session),        /* tp_basicsize */
    0,                      /* tp_itemsize */
    Session_dealloc,        /* tp_dealloc */
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
    "Session Class",        /* tp_doc */
    0,                      /* tp_traverse */
    0,                      /* tp_clear */
    0,                      /* tp_richcompare */
    0,                      /* tp_weaklistoffset */
    0,                      /* tp_iter */
    0,                      /* tp_iternext */
    Session_methods,        /* tp_methods */
    0,                      /* tp_members */
    Session_getseters,      /* tp_getset */
    0,                      /* tp_base */
    0,                      /* tp_dict */
    0,                      /* tp_descr_get */
    0,                      /* tp_descr_set */
    0,                      /* tp_dictoffset */
    0,                      /* tp_init */
    0,                      /* tp_alloc */
    Session_new,            /* tp_new */
    0,                      /* tp_free */
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

}
