#ifndef TYPYTHON_H
#define TYPYTHON_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <tyjuliacapi.hpp>
#include <common.hpp>
#include <assert.h>

ErrorCode py_cast(JV *out, JV T, PyObject* py)
{
    JV args[2];

    JV jlpy = to_JLPy(py);

    args[0] = T;
    args[1] = jlpy;
    ErrorCode ret = JLCall(out, MyJLAPI.f_TyPython_py_cast, SList_adapt(args, 2), emptyKwArgs());
    if (ret != ErrorCode::ok)
    {
        char errorBytes[2048] = {(char)0};
        if (ErrorCode::ok == JLError_FetchMsgStr(&errorSym, SList_adapt(reinterpret_cast<uint8_t *>(errorBytes), sizeof(errorBytes))))
        {
        PyErr_SetString(JuliaCallError, errorBytes);
        }
        else
        {
        PyErr_SetString(JuliaCallError, "juliacall: unknown error");
        }
    }
    JLFreeFromMe(jlpy);
    return ret;
}


ErrorCode TOJLInt64FromPy(JV *out, PyObject* py)
{
    Py_ssize_t i = PyLong_AsSsize_t(py);
    if (i == -1 && PyErr_Occurred() != NULL)
        return ErrorCode::error;

    ToJLInt64(out, i);
    return ErrorCode::ok;
}


ErrorCode ToJLFloat64FromPy(JV *out, PyObject* py)
{
    double i = PyFloat_AsDouble(py);
    if (i == -1.0 && PyErr_Occurred() != NULL)
        return ErrorCode::error;

    ToJLFloat64(out, i);
    return ErrorCode::ok;
}

ErrorCode ToJLBoolFromPy(JV *out, PyObject* py)
{
    if (py == Py_True)
    {
        ToJLBool(out, 1);
    }
    else if (py == Py_False)
    {
        ToJLBool(out, 0);
    }
    else
    {
        int i = PyObject_IsTrue(py);
        bool8_t flag = (i != 0);
        ToJLBool(out, flag);
    }
    return ErrorCode::ok;
}

// ErrorCode ToJLComplexFromPy(JV *out, PyObject* py)
// {
//     Py_complex i = PyComplex_AsCComplex(py);
//     if (i.real == -1.0 && PyErr_Occurred() != NULL)
//         return ErrorCode::error;

//     ToJLComplexF64(out, reinterpret_cast<complex_t>(i));
//     return ErrorCode::ok;
// }

#endif



