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
int stop_script(...);

}

#endif // PYCORE_H
