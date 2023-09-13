#ifndef JULIACALL_COMMON_H
#define JULIACALL_COMMON_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <tyjuliacapi.hpp>

struct t_PyAPI
{
    PyObject* m_builtin;
    PyObject* m_NumPy;
    PyObject* t_JV;
    PyObject* t_dict;
    PyObject* t_tuple;
    PyObject* t_int;
    PyObject* t_float;
    PyObject* t_str;
    PyObject* t_bool;
    PyObject* t_ndarray;
    PyObject* t_complex;
    PyObject* f_next;
    PyObject* f_iter;
};

struct t_JLAPI
{
    JV t_Integer;
    JV t_AbstractFloat;
    JV t_AbstractString;
    JV t_Bool;
    JV t_Complex;
    JV t_AbstractArray;
    JV t_BitArray;
    JV t_String;
    JV t_Tuple;
    JV t_JNumPySupportedNumPyArrayBoxingElementTypes;

    JV f_eltype;
    JV f_TyPython_py_cast;
    JV f_TyPython_Py;
    JV ojb_TyPython_BorrowReference;
    JV f_TyPython_PtrPyObject;

    JV obj_true;
    JV obj_false;
    JV obj_nothing;
};

// typedef void *(*t_jl_eval_string)(const char *code);
// typedef struct _jl_value_t jl_value_t;
// typedef jl_value_t *(*t_jl_exception_occurred)();
typedef JV (*t_to_JLPy)(PyObject* py);
static t_to_JLPy to_JLPy = NULL;
static const JV JV_NULL = 0;
// static t_jl_eval_string jl_eval_string = NULL;
// static t_jl_exception_occurred jl_exception_occurred = NULL;
static t_PyAPI MyPyAPI;
static t_JLAPI MyJLAPI;
static PyObject *JuliaCallError;
static PyObject *JVType;
static PyObject *name_slot;
static JSym errorSym;
static JV f_jl_square;
static JV f_jl_display;
// static JV obj_TyPython_CPython_BorrowReference;
// static JV f_TyPython_CPython_Py;
// static JV f_TyPython_CPython_py_cast;


static void init_JLAPI()
{
    JLEval(&MyJLAPI.t_Integer, NULL, "Integer");
    JLEval(&MyJLAPI.t_AbstractFloat, NULL, "AbstractFloat");
    JLEval(&MyJLAPI.t_AbstractString, NULL, "AbstractString");
    JLEval(&MyJLAPI.t_Bool, NULL, "Bool");
    JLEval(&MyJLAPI.t_Complex, NULL, "Complex");
    JLEval(&MyJLAPI.t_AbstractArray, NULL, "AbstractArray");
    JLEval(&MyJLAPI.t_BitArray, NULL, "BitArray");
    JLEval(&MyJLAPI.t_String, NULL, "String");
    JLEval(&MyJLAPI.t_Tuple, NULL, "Tuple");
    JLEval(&MyJLAPI.t_JNumPySupportedNumPyArrayBoxingElementTypes, NULL, "Union{Int8, Int16, Int32, Int64, UInt8, UInt16, UInt32, UInt64, Float16, Float32, Float64, ComplexF16, ComplexF32, ComplexF64, Bool}");

    JLEval(&MyJLAPI.f_eltype, NULL, "Base.eltype");
    JLEval(&MyJLAPI.f_TyPython_py_cast, NULL, "TyPython.CPython.py_cast");
    JLEval(&MyJLAPI.f_TyPython_Py, NULL, "TyPython.CPython.Py");
    JV* t_TyPython_BorrowReference = (JV*)malloc(sizeof(JV));
    JLEval(t_TyPython_BorrowReference, NULL, "TyPython.CPython.BorrowReference");
    JLCall(&MyJLAPI.ojb_TyPython_BorrowReference, *t_TyPython_BorrowReference, emptyArgs(), emptyKwArgs());
    JLEval(&MyJLAPI.f_TyPython_Py, NULL, "TyPython.CPython.Py");
    JLEval(&MyJLAPI.f_TyPython_py_cast, NULL, "TyPython.CPython.py_cast");
    JLEval(&MyJLAPI.f_TyPython_PtrPyObject, NULL, "TyPython.C.Ptr{TyPython.CPython.PyObject}");
    JLEval(&MyJLAPI.obj_true, NULL, "true");
    JLEval(&MyJLAPI.obj_false, NULL, "false");
    JLEval(&MyJLAPI.obj_nothing, NULL, "nothing");
}

static void init_PyAPI(PyObject* t_JV)
{
    MyPyAPI.t_JV = t_JV;
    MyPyAPI.m_builtin = PyImport_ImportModule("builtins");
    MyPyAPI.m_NumPy = PyImport_ImportModule("numpy");
    MyPyAPI.t_dict = PyObject_GetAttrString(MyPyAPI.m_builtin, "dict");
    MyPyAPI.t_tuple = PyObject_GetAttrString(MyPyAPI.m_builtin, "tuple");
    MyPyAPI.t_int = PyObject_GetAttrString(MyPyAPI.m_builtin, "int");
    MyPyAPI.t_float = PyObject_GetAttrString(MyPyAPI.m_builtin, "float");
    MyPyAPI.t_str = PyObject_GetAttrString(MyPyAPI.m_builtin, "str");
    MyPyAPI.t_bool = PyObject_GetAttrString(MyPyAPI.m_builtin, "bool");
    MyPyAPI.t_ndarray = PyObject_GetAttrString(MyPyAPI.m_NumPy, "ndarray");
    MyPyAPI.t_complex = PyObject_GetAttrString(MyPyAPI.m_builtin, "complex");
    MyPyAPI.f_next = PyObject_GetAttrString(MyPyAPI.m_builtin, "next");
    MyPyAPI.f_iter = PyObject_GetAttrString(MyPyAPI.m_builtin, "iter");
}


#endif
