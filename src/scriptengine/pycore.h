#ifndef PYCORE_H
#define PYCORE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <marshal.h>

#include "pyrun.h"

namespace PyCore {

PyMODINIT_FUNC PyInit_quardCRT(PyRun* parent);
void PyDeInit_quardCRT(PyObject* m);
PyRun* getCore(void);
PyObject* getQuardCRTException(void);
void CrtScriptErrorException_SetMessage(const char* errorMessage);
int stop_script();

}

#endif // PYCORE_H
