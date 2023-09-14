#ifndef JULIACALL_COMMON_H
#define JULIACALL_COMMON_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <tyjuliacapi.hpp>

static PyObject *JuliaCallError;
static PyObject *name_jlslot;
static JSym errorSym;

PyObject* HandleJLErrorAndReturnNULL()
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
    return NULL;
}


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
    int64_t t_Nothing;
    int64_t t_Integer;
    int64_t t_AbstractFloat;
    int64_t t_AbstractString;
    int64_t t_Bool;
    int64_t t_Complex;
    int64_t t_AbstractArray;
    int64_t t_BitArray;
    int64_t t_String;
    int64_t t_Tuple;

    JV f_eltype;
    JV f_length;
    JV f_repr;
    JV f_isa;
    JV f_ncodeunits;
    JV f_square;
    JV f_subtype;
    JV f_add;
    JV f_sub;
    JV f_mul;
    JV f_matmul;
    JV f_truediv;
    JV f_floordiv;
    JV f_mod;
    JV f_pow;
    JV f_lshift;
    JV f_rshift;
    JV f_bitor;
    JV f_bitxor;
    JV f_bitand;
    JV f_eq;
    JV f_ne;
    JV f_le;
    JV f_ge;
    JV f_lt;
    JV f_gt;
    JV f_abs;
    JV f_invert;
    JV f_in;
    JV f_hash;

    JV obj_true;
    JV obj_false;
    JV obj_nothing;
    JV obj_JNumPySupportedNumPyArrayBoxingElementTypes;
};

typedef ErrorCode (*t_pycast2jl)(/* out */ JV* out, JV T, PyObject* py);
typedef PyObject *(*t_pycast2py)(JV jv);
static t_pycast2jl pycast2jl = NULL;
static t_pycast2py pycast2py = NULL;
static const JV JV_NULL = 0;

static t_PyAPI MyPyAPI;
static t_JLAPI MyJLAPI;


static void init_JLAPI()
{
    JV t;
    JLEval(&t, NULL, "Nothing");
    JLTypeToIdent(&MyJLAPI.t_Nothing, t);
    JLEval(&t, NULL, "Integer");
    JLTypeToIdent(&MyJLAPI.t_Integer, t);
    JLEval(&t, NULL, "AbstractFloat");
    JLTypeToIdent(&MyJLAPI.t_AbstractFloat, t);
    JLEval(&t, NULL, "AbstractString");
    JLTypeToIdent(&MyJLAPI.t_AbstractString, t);
    JLEval(&t, NULL, "Bool");
    JLTypeToIdent(&MyJLAPI.t_Bool, t);
    JLEval(&t, NULL, "Complex");
    JLTypeToIdent(&MyJLAPI.t_Complex, t);
    JLEval(&t, NULL, "AbstractArray");
    JLTypeToIdent(&MyJLAPI.t_AbstractArray, t);
    JLEval(&t, NULL, "BitArray");
    JLTypeToIdent(&MyJLAPI.t_BitArray, t);
    JLEval(&t, NULL, "String");
    JLTypeToIdent(&MyJLAPI.t_String, t);
    JLEval(&t, NULL, "Tuple");
    JLTypeToIdent(&MyJLAPI.t_Tuple, t);

    JLEval(&MyJLAPI.f_eltype, NULL, "Base.eltype");
    JLEval(&MyJLAPI.f_repr, NULL, "Base.repr");
    JLEval(&MyJLAPI.f_isa, NULL, "Base.isa");
    JLEval(&MyJLAPI.f_ncodeunits, NULL, "Base.ncodeunits");
    JLEval(&MyJLAPI.f_square, NULL, "square(x) = x .^ 2");
    JLEval(&MyJLAPI.f_subtype, NULL, "<:");
    JLEval(&MyJLAPI.f_add, NULL, "Base.:+");
    JLEval(&MyJLAPI.f_sub, NULL, "Base.:-");
    JLEval(&MyJLAPI.f_mul, NULL, ".*");
    JLEval(&MyJLAPI.f_matmul, NULL, "Base.:*");
    JLEval(&MyJLAPI.f_truediv, NULL, "Base.:/");
    JLEval(&MyJLAPI.f_floordiv, NULL, "Base.div");
    JLEval(&MyJLAPI.f_mod, NULL, "Base.mod");
    JLEval(&MyJLAPI.f_pow, NULL, "Base.:^");
    JLEval(&MyJLAPI.f_lshift, NULL, "Base.:<<");
    JLEval(&MyJLAPI.f_rshift, NULL, "Base.:>>");
    JLEval(&MyJLAPI.f_bitor, NULL, "Base.:|");
    JLEval(&MyJLAPI.f_bitxor, NULL, "Base.:⊻");
    JLEval(&MyJLAPI.f_bitand, NULL, "Base.:&");
    JLEval(&MyJLAPI.f_eq, NULL, "Base.:(==)");
    JLEval(&MyJLAPI.f_ne, NULL, "Base.:(!=)");
    JLEval(&MyJLAPI.f_le, NULL, "Base.:(<=)");
    JLEval(&MyJLAPI.f_ge, NULL, "Base.:(>=)");
    JLEval(&MyJLAPI.f_lt, NULL, "Base.:<");
    JLEval(&MyJLAPI.f_gt, NULL, "Base.:>");
    JLEval(&MyJLAPI.f_abs, NULL, "Base.abs");
    JLEval(&MyJLAPI.f_abs, NULL, "Base.abs");
    JLEval(&MyJLAPI.f_invert, NULL, "Base.:~");
    JLEval(&MyJLAPI.f_in, NULL, "Base.in");
    JLEval(&MyJLAPI.f_hash, NULL, "Base.hash");

    JLEval(&MyJLAPI.obj_true, NULL, "true");
    JLEval(&MyJLAPI.obj_false, NULL, "false");
    JLEval(&MyJLAPI.obj_nothing, NULL, "nothing");
    JLEval(&MyJLAPI.obj_JNumPySupportedNumPyArrayBoxingElementTypes, NULL, "Union{Int8, Int16, Int32, Int64, UInt8, UInt16, UInt32, UInt64, Float16, Float32, Float64, ComplexF16, ComplexF32, ComplexF64, Bool}");
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
