#ifndef TYPYTHON_H
#define TYPYTHON_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <tyjuliacapi.hpp>
#include <common.hpp>
#include <assert.h>

static void PyCapsule_Destruct_JuliaAsPython(PyObject *capsule)
{
  // destruct of capsule(__jlslot__)
  JV *jv = (JV *)PyCapsule_GetPointer(capsule, NULL);
  JLFreeFromMe(*jv);
  free(jv);
}

static PyObject *box_julia(JV jv)
{
  // JV(julia value) -> PyObject(python's JV with __jlslot__)
  if (jv == JV_NULL)
  {
    PyErr_SetString(PyExc_RuntimeError, "box_julia: failed to create a new instance of JV");
    return NULL;
  }

  JV *ptr_boxed = (JV *)malloc(sizeof(JV));
  *ptr_boxed = jv;

  PyObject *capsule = PyCapsule_New(
      ptr_boxed,
      NULL,
      &PyCapsule_Destruct_JuliaAsPython);

  PyObject *pyjv = PyObject_CallObject(MyPyAPI.t_JV, NULL);
  if (pyjv == NULL)
  {
    PyErr_SetString(PyExc_RuntimeError, "box_julia: failed to create a new instance of JV");
    return NULL;
  }

  PyObject_SetAttr(pyjv, name_jlslot, capsule);
  return pyjv;
}

static JV unbox_julia(PyObject *pyjv)
{
  // assume pyjv is a python's JV instance with __jlslot__
  PyObject *capsule = PyObject_GetAttr(pyjv, name_jlslot);
  JV *jv = (JV *)PyCapsule_GetPointer(capsule, NULL);
  Py_DecRef(capsule);
  return *jv;
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

ErrorCode ToJLComplexFromPy(JV *out, PyObject* py)
{
    double re = PyComplex_RealAsDouble(py);
    if (re == -1.0 && PyErr_Occurred() != NULL)
        return ErrorCode::error;

    double im = PyComplex_ImagAsDouble(py);
    if (im == -1.0 && PyErr_Occurred() != NULL)
        return ErrorCode::error;

    ToJLComplexF64(out, complex_t{re, im});
    return ErrorCode::ok;
}

ErrorCode ToJLStringFromPy(JV *out, PyObject* py)
{
    Py_ssize_t len;
    const char* str = PyUnicode_AsUTF8AndSize(py, &len);
    if (str == NULL)
        return ErrorCode::error;

    ToJLString(out, SList_adapt(reinterpret_cast<uint8_t *>(const_cast<char *>(str)), len));
    return ErrorCode::ok;
}

ErrorCode ToJLNothingFromPy(JV *out, PyObject* py)
{
    if (py != Py_None)
        return ErrorCode::error;
    else
    {
        *out = MyJLAPI.obj_nothing;
        return ErrorCode::ok;
    }
}

JV reasonable_unbox(PyObject* py)
{
    if (py == Py_None)
        return MyJLAPI.obj_nothing;

    if (PyObject_IsInstance(py, MyPyAPI.t_JV))
        return unbox_julia(py);

    JV out;
    if (PyObject_IsInstance(py, MyPyAPI.t_int))
    {
        TOJLInt64FromPy(&out, py);
        return out;
    }

    if (PyObject_IsInstance(py, MyPyAPI.t_float))
    {
        ToJLFloat64FromPy(&out, py);
        return out;
    }

    if (PyObject_IsInstance(py, MyPyAPI.t_str))
    {
        ToJLStringFromPy(&out, py);
        return out;
    }

    if (PyObject_IsInstance(py, MyPyAPI.t_bool))
    {
        ToJLBoolFromPy(&out, py);
        return out;
    }

    if (PyObject_IsInstance(py, MyPyAPI.t_complex))
    {
        ToJLComplexFromPy(&out, py);
        return out;
    }

    if (PyObject_IsInstance(py, MyPyAPI.t_ndarray))
    {
        ErrorCode ret = pycast2jl(&out, MyJLAPI.t_AbstractArray, py);
        if (ret == ErrorCode::ok)
        {
            return out;
        }
        else
        {
            // todo: string array
        }
    }

    // todo: python's tuple

    PyErr_SetString(JuliaCallError, "unbox failed: cannot convert a Python object to Julia object");
    return JV_NULL;
}

// PyObject* UnsafePyLongFromJL(JV jv)
// {
//     int64_t i;
//     JLGetInt64(&i, jv, true);
//     return PyLong_FromLongLong(i);
// }

// PyObject* UnsafePyDoubleFromJL(JV jv)
// {
//     double i;
//     JLGetDouble(&i, jv, true);
//     return PyFloat_FromDouble(i);
// }

// PyObject* UnsafePyBoolFromJL(JV jv)
// {
//     bool8_t i;
//     JLGetBool(&i, jv, true);
//     return PyBool_FromLong(i);
// }

// PyObject* UnsafePyStringFromJL(JV jv)
// {
//     JV ncode;
//     JLCall(&ncode, MyJLAPI.f_ncodeunits, SList_adapt(&jv, 1), emptyKwArgs());
//     int64_t len;
//     JLGetInt64(&len, ncode, true);
//     JLFreeFromMe(ncode);

//     if (len == 0)
//         return PyUnicode_FromString("");
//     else
//     {
//         // this copy the string twice
//         char *buf = (char *)malloc(len + 1);
//         JLGetUTF8String(SList_adapt(reinterpret_cast<uint8_t *>(const_cast<char *>(buf)), len), jv);
//         buf[len] = '\0';
//         PyObject* str = PyUnicode_FromString(buf);
//         free(buf);
//         return str;
//     }
// }


PyObject * reasonable_box(JV jv)
{
    PyObject* py;

    if (JLIsInstanceWithTypeSlot(jv, MyJLAPI.t_Nothing))
    {
        Py_IncRef(Py_None);
        return Py_None;
    }

    if (JLIsInstanceWithTypeSlot(jv, MyJLAPI.t_Integer))
    {
        py = pycast2py(jv);
        return py;
    }

    if (JLIsInstanceWithTypeSlot(jv, MyJLAPI.t_AbstractFloat))
    {
        py = pycast2py(jv);
        return py;
    }

    if (JLIsInstanceWithTypeSlot(jv, MyJLAPI.t_Bool))
    {
        py = pycast2py(jv);
        return py;
    }

    if (JLIsInstanceWithTypeSlot(jv, MyJLAPI.t_Complex))
    {
        py = pycast2py(jv);
        return py;
    }

    if (JLIsInstanceWithTypeSlot(jv, MyJLAPI.t_String))
    {
        py = pycast2py(jv);
        return py;
    }

    if (JLIsInstanceWithTypeSlot(jv, MyJLAPI.t_AbstractArray))
    {
        // todo
        if (JLIsInstanceWithTypeSlot(jv, MyJLAPI.t_BitArray))
        {
            py = box_julia(jv);
        }
        else
        {
            py = pycast2py(jv);
            if (py == NULL) /* on fail */
                py = box_julia(jv);
        }
        return py;
    }

    // todo: tuple

    return box_julia(jv);
}

#endif



