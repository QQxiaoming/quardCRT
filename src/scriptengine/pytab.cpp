#include "pytab.h"
#include "pyscreen.h"
#include "pysession.h"

namespace PyTab {

typedef struct {
    PyObject_HEAD
    int id;
} Tab;

// Tab.GetScreen(id: int) -> Screen
static PyObject* Tab_GetScreen(Tab* self, PyObject* args) {
    int screenId;
    if (!PyArg_ParseTuple(args, "i", &screenId)) {
        PyCore::CrtScriptErrorException_SetMessage("id is not an integer");
        PyErr_SetString(PyCore::getQuardCRTException(), "args is not an integer");
        return NULL;
    }
    int screenRealId = PyCore::getCore()->tabCheckScreenId(self->id,screenId);
    if(screenRealId == -1){
        PyCore::CrtScriptErrorException_SetMessage("id is invalid");
        PyErr_SetString(PyCore::getQuardCRTException(), "id is invalid");
        return NULL;
    }
    PyObject *arg = PyTuple_New(0);
    Py_INCREF(arg);
    PyObject *kwargs = Py_BuildValue("{s:i}", "id", screenRealId);
    Py_INCREF(kwargs);
    PyObject *screen = PyObject_Call((PyObject *)&PyScreen::ScreenType, arg, kwargs);
    Py_XDECREF(arg);
    Py_XDECREF(kwargs);
    //Py_INCREF(screen);
    return screen;
}

// Tab.GetSession(id: int) -> Session
static PyObject* Tab_GetSession(Tab* self, PyObject* args) {
    int sessionId;
    if (!PyArg_ParseTuple(args, "i", &sessionId)) {
        PyCore::CrtScriptErrorException_SetMessage("id is not an integer");
        PyErr_SetString(PyCore::getQuardCRTException(), "args is not an integer");
        return NULL;
    }
    int sessionRealId = PyCore::getCore()->tabCheckScreenId(self->id,sessionId);
    if(sessionRealId == -1){
        PyCore::CrtScriptErrorException_SetMessage("id is invalid");
        PyErr_SetString(PyCore::getQuardCRTException(), "id is invalid");
        return NULL;
    }
    PyObject *arg = PyTuple_New(0);
    Py_INCREF(arg);
    PyObject *kwargs = Py_BuildValue("{s:i}", "id", sessionRealId);
    Py_INCREF(kwargs);
    PyObject *session = PyObject_Call((PyObject *)&PySession::SessionType, arg, kwargs);
    Py_XDECREF(arg);
    Py_XDECREF(kwargs);
    //Py_INCREF(session);
    return session;
}

// Tab.Activate(id: int) -> None
static PyObject* Tab_Activate(Tab* self, PyObject* args, PyObject* kwds) {
    int tabId = -1;
    static const char* kwlist[] = {"id", NULL};
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|i", (char**)kwlist, &tabId)) {
        PyCore::CrtScriptErrorException_SetMessage("id is not an integer");
        PyErr_SetString(PyCore::getQuardCRTException(), "args is not an integer");
        return NULL;
    }
    PyCore::getCore()->tabActivate(self->id,tabId);
    Py_RETURN_NONE;
}

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
static PyMethodDef Tab_methods[] = {
    {"GetScreen", (PyCFunction)Tab_GetScreen, METH_VARARGS, "GetScreen"},
    {"GetSession", (PyCFunction)Tab_GetSession, METH_VARARGS, "GetSession"},
    {"Activate", (PyCFunction)Tab_Activate, METH_VARARGS|METH_KEYWORDS, "Activate"},
    {NULL,NULL,0,NULL}  // Sentinel
};
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

static PyObject* Tab_getNumber(Tab* self, void* closure) {
    Q_UNUSED(closure);
    int ret = PyCore::getCore()->tabGetnumber(self->id);
    return Py_BuildValue("i", ret);
}

static PyGetSetDef Tab_getseters[] = {
    {"Number", (getter)Tab_getNumber, NULL, "Number", NULL},
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
