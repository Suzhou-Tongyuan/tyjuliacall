# MIT License

# Copyright (c) 2021 cjdoris

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# https://github.com/cjdoris/PythonCall.jl/blob/main/src/cpython/jlwrap.jl
Base.@kwdef struct PyJuliaValueObject
    ob_base::PyObject = PyObject()
    value::Int = 0
    weaklist::C.Ptr{PyObject} = C_NULL
end

const METH_NOARGS = Cint(0x0004)   # no arguments (NULL argument pointer)
const METH_O = Cint(0x0008)        # single argument (not wrapped in tuple)
const METH_FASTCALL = Cint(0x0080) # only positional arguments (self, Ptr{C.Ptr{PyObject}}, Py_ssize_t)

# the `value` field of `PyJuliaValueObject` indexes into here
const PYJLVALUES = []
# unused indices in PYJLVALUES
const PYJLFREEVALUES = Int[]

Py_Type(x::C.Ptr{PyObject}) = C.Ptr{PyObject}(x[].type)
Py_Type(x::Py) = Py_Type(unsafe_unwrap(x))

function _pyjl_new(t::C.Ptr{PyObject}, ::C.Ptr{PyObject}, ::C.Ptr{PyObject})
    o = ccall(C.Ptr{PyTypeObject}(t).tp_alloc[], C.Ptr{PyObject}, (C.Ptr{PyObject}, Py_ssize_t), t, 0)
    o == Py_NULLPTR && return Py_NULLPTR
    C.Ptr{PyJuliaValueObject}(o).weaklist[] = Py_NULLPTR
    C.Ptr{PyJuliaValueObject}(o).value[] = 0
    return o
end

function _pyjl_dealloc(o::C.Ptr{PyObject})
    idx = C.Ptr{PyJuliaValueObject}(o).value[]
    if idx != 0
        PYJLVALUES[idx] = nothing
        push!(PYJLFREEVALUES, idx)
    end
    C.Ptr{PyJuliaValueObject}(o).weaklist[] == Py_NULLPTR || PyAPI.PyObject_ClearWeakRefs(o)
    ccall(C.Ptr{PyTypeObject}(Py_Type(o)).tp_free[], Cvoid, (C.Ptr{PyObject},), o)
    nothing
end

PyJuliaValue_IsNull(o::C.Ptr{PyObject}) = C.Ptr{PyJuliaValueObject}(o).value[] == 0

PyJuliaValue_GetValue(o::C.Ptr{PyObject}) = PYJLVALUES[C.Ptr{PyJuliaValueObject}(o).value[]]

function PyJuliaValue_SetValue(o::C.Ptr{PyObject}, @nospecialize(v))
    idx = C.Ptr{PyJuliaValueObject}(o).value[]
    if idx == 0
        if isempty(PYJLFREEVALUES)
            push!(PYJLVALUES, v)
            idx = length(PYJLVALUES)
        else
            idx = pop!(PYJLFREEVALUES)
            PYJLVALUES[idx] = v
        end
        C.Ptr{PyJuliaValueObject}(o).value[] = idx
    else
        PYJLVALUES[idx] = v
    end
    nothing
end

function PyJuliaValue_New(t::C.Ptr{PyObject}, @nospecialize(v))
    if PyAPI.PyType_IsSubtype(t, PyJuliaBase_Type) != 1
        py_seterror!(G_PyBuiltin.TypeError, "Expecting a subtype of 'JuliaBase'")
        return Py_NULLPTR
    end
    o = PyAPI.PyObject_CallObject(t, Py_NULLPTR)
    o == Py_NULLPTR && return Py_NULLPTR
    PyJuliaValue_SetValue(o, v)
    return o
end

const _pyjlbase_name = "JuliaBase"
const _pyjlbase_methods = Vector{PyMethodDef}()
const _pyjlbase_type = fill(PyTypeObject(ob_base=PyObject(), ob_size = 0))
const Py_TPFLAGS_BASETYPE = (0x00000001 << 10)
const Py_TPFLAGS_HAVE_VERSION_TAG = (0x00000001 << 18)
const Meth_Names = (
    __getattr__ = "__getattr__",
    __setattr__ = "__setattr__",
    __getitem__ = "__getitem__",
    __setitem__ = "__setitem__",
    __pos__ = "__pos__",
    __neg__ = "__neg__",
    __len__ = "__len__",
    __invert__ = "__invert__",
    __abs__ = "__abs__",
    __hash__ = "__hash__",
    __bool__ = "__bool__",
    _jl_repr_pretty_ = "_jl_repr_pretty_",
    _jl_first_iter_ = "_jl_first_iter_",
    __add__ = "__add__",
    __sub__ = "__sub__",
    __matmul__ = "__matmul__",
    __mul__ = "__mul__",
    __truediv__ = "__truediv__",
    __floordiv__ = "__floordiv__",
    __mod__ = "__mod__",
    __lshift__ = "__lshift__",
    __rshift__ = "__rshift__",
    __and__ = "__and__",
    __xor__ = "__xor__",
    __or__ = "__or__",
    __eq__ = "__eq__",
    __ne__ = "__ne__",
    __lt__ = "__lt__",
    __le__ = "__le__",
    __gt__ = "__gt__",
    __ge__ = "__ge__",
    __pow__ = "__pow__",
    __contains__ = "__contains__",
    _jl_next_iter_ = "_jl_next_iter_",
)

function _init_juliabase()
    empty!(_pyjlbase_methods)
    # todo(songjhaha): codegen
    push!(_pyjlbase_methods,
        PyMethodDef(
            ml_name = pointer(Meth_Names.__getattr__),
            ml_meth = @cfunction(_pyjl_getattr, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__setattr__),
            ml_meth = @cfunction(_pyjl_setattr, C.Ptr{PyObject}, (C.Ptr{PyObject}, Ptr{C.Ptr{PyObject}}, Py_ssize_t)),
            ml_flags = METH_FASTCALL,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__getitem__),
            ml_meth = @cfunction(_pyjl_getitem, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__setitem__),
            ml_meth = @cfunction(_pyjl_setitem, C.Ptr{PyObject}, (C.Ptr{PyObject}, Ptr{C.Ptr{PyObject}}, Py_ssize_t)),
            ml_flags = METH_FASTCALL,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__pos__),
            ml_meth = @cfunction(_pyjl_pos, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_NOARGS,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__neg__),
            ml_meth = @cfunction(_pyjl_neg, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_NOARGS,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__len__),
            ml_meth = @cfunction(_pyjl_len, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_NOARGS,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__invert__),
            ml_meth = @cfunction(_pyjl_invert, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_NOARGS,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__abs__),
            ml_meth = @cfunction(_pyjl_abs, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_NOARGS,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__hash__),
            ml_meth = @cfunction(_pyjl_hash, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_NOARGS,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__bool__),
            ml_meth = @cfunction(_pyjl_bool, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_NOARGS,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names._jl_repr_pretty_),
            ml_meth = @cfunction(_pyjl_repr_pretty, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_NOARGS,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names._jl_first_iter_),
            ml_meth = @cfunction(_pyjl_first_iter, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_NOARGS,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__add__),
            ml_meth = @cfunction(_pyjl_add, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__sub__),
            ml_meth = @cfunction(_pyjl_sub, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__matmul__),
            ml_meth = @cfunction(_pyjl_matmul, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__mul__),
            ml_meth = @cfunction(_pyjl_mul, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__truediv__),
            ml_meth = @cfunction(_pyjl_truediv, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__floordiv__),
            ml_meth = @cfunction(_pyjl_floordiv, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__mod__),
            ml_meth = @cfunction(_pyjl_mod, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__lshift__),
            ml_meth = @cfunction(_pyjl_lshift, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__rshift__),
            ml_meth = @cfunction(_pyjl_rshift, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__and__),
            ml_meth = @cfunction(_pyjl_and, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),PyMethodDef(
            ml_name = pointer(Meth_Names.__xor__),
            ml_meth = @cfunction(_pyjl_xor, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__or__),
            ml_meth = @cfunction(_pyjl_or, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__eq__),
            ml_meth = @cfunction(_pyjl_eq, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__ne__),
            ml_meth = @cfunction(_pyjl_ne, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__le__),
            ml_meth = @cfunction(_pyjl_le, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__lt__),
            ml_meth = @cfunction(_pyjl_lt, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__ge__),
            ml_meth = @cfunction(_pyjl_ge, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__gt__),
            ml_meth = @cfunction(_pyjl_gt, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__pow__),
            ml_meth = @cfunction(_pyjl_pow, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names.__contains__),
            ml_meth = @cfunction(_pyjl_contains, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(
            ml_name = pointer(Meth_Names._jl_next_iter_),
            ml_meth = @cfunction(_pyjl_next_iter, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject})),
            ml_flags = METH_O,
        ),
        PyMethodDef(),
    )

    _pyjlbase_type[] = PyTypeObject(
            ob_base = PyObject(),
            ob_size = 0,
            tp_name = pointer(_pyjlbase_name),
            tp_basicsize = sizeof(PyJuliaValueObject),
            tp_new = @cfunction(_pyjl_new, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject}, C.Ptr{PyObject})),
            tp_dealloc = @cfunction(_pyjl_dealloc, Cvoid, (C.Ptr{PyObject},)),
            tp_flags = Py_TPFLAGS_HAVE_VERSION_TAG | Py_TPFLAGS_BASETYPE,
            tp_weaklistoffset = fieldoffset(PyJuliaValueObject, 3),
            tp_methods = pointer(_pyjlbase_methods),
            tp_call = @cfunction(_pyjl_call, C.Ptr{PyObject}, (C.Ptr{PyObject}, C.Ptr{PyObject}, C.Ptr{PyObject})),
            tp_repr = @cfunction(_pyjl_repr, C.Ptr{PyObject}, (C.Ptr{PyObject},)),
            tp_base = reinterpret(Base.Ptr{PyObject}, unsafe_unwrap(Pyobject_Type)),
        )

    o = C.Ptr{PyObject}(pointer(_pyjlbase_type))
    if PyAPI.PyType_Ready(o) == -1
        error("Error initializing 'JuliaBase'")
    end
    PyAPI.Py_IncRef(o)
    unsafe_set!(PyJuliaBase_Type, o)
end
