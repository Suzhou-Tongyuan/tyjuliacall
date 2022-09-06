module TyJuliaSetup
using TyPython
using TyPython.CPython
using TyPython.CPython: PyAPI, Py_NULLPTR, py_throw
using TyPython: C

const _store_string_symbols = Dict{String, Symbol}()

function attr_name_to_symbol(s::String)::Symbol
    get!(_store_string_symbols, s) do
        v = Symbol(s)
        _store_string_symbols[s] = v
        return v
    end
end

mutable struct RequiredFromPythonAPIStruct
    JV::Py
    class::Py  # the 'type' object in Python
    next::Py
    none::Py
    # buultin datatypes
    dict::Py

    tuple::Py
    int::Py
    float::Py
    str::Py
    bool::Py
    ndarray::Py
    complex::Py
    RequiredFromPythonAPIStruct() = new()
end

const MyPyAPI = RequiredFromPythonAPIStruct()

function classof(x::Py)::Py
    classof(x)
end

function is_type_exact(x::Py, t::Py)::Py
    return CPython.py_equal_identity(classof(x), t)
end

struct JuliaAsPython
    slot::Int
end

const JlValuePools = Any[]
const JlValueUnusedSlots = Int[]

function PyCapsule_Destruct_JuliaAsPython(o::Ptr{CPython.PyObject})::Cvoid
    o = PyAPI.PyCapsule_GetPointer(C.Ptr(o), reinterpret(Cstring, C_NULL))
    if o == C_NULL && PyAPI.PyErr_Occurred() != Py_NULLPTR
        py_throw()
    end
    ptr = C.Ptr{JuliaAsPython}(o)
    slot = ptr.slot[]
    push!(JlValueUnusedSlots, slot)
    JlValuePools[slot] = nothing
    Base.Libc.free(o)
    nothing
end

function box_julia(val::Any)
    ptr_boxed = reinterpret(C.Ptr{JuliaAsPython}, Base.Libc.malloc(sizeof(JuliaAsPython)))
    slot = if isempty(JlValueUnusedSlots)
        push!(JlValuePools, nothing)
        length(JlValuePools)
    else
        pop!(JlValueUnusedSlots)
    end

    ptr_boxed[] = JuliaAsPython(slot)
    JlValuePools[slot] = val

    capsule = Py(PyAPI.PyCapsule_New(
        reinterpret(Ptr{Cvoid}, ptr_boxed),
        reinterpret(Cstring, C_NULL),
        @cfunction(PyCapsule_Destruct_JuliaAsPython, Cvoid, (Ptr{CPython.PyObject}, ))))

    jv = MyPyAPI.JV()
    jv.__jlslot__ = capsule
    return jv
end

@inline function unbox_julia(x::Py)
    o = PyAPI.PyCapsule_GetPointer(CPython.unsafe_unwrap(x.__jlslot__), reinterpret(Cstring, C_NULL))
    if o == C_NULL && PyAPI.PyErr_Occurred() != Py_NULLPTR
        py_throw()
    end
    ptr = C.Ptr{JuliaAsPython}(o)
    slot = ptr.slot[]
    return JlValuePools[slot]
end

function reasonable_unbox(py::Py)
    if CPython.py_equal_identity(py, MyPyAPI.none)
        return nothing
    end
    if CPython.py_equal_identity(py, MyPyAPI.JV)
        return unbox_julia(py)
    end

    t = classof(py)
    if CPython.py_equal_identity(t, MyPyAPI.int)
        return py_cast(Int, py)
    end
    if CPython.py_equal_identity(t, MyPyAPI.float)
        return py_cast(Float64, py)
    end
    if CPython.py_equal_identity(t, MyPyAPI.str)
        return py_cast(String, py)
    end
    if CPython.py_equal_identity(t, MyPyAPI.bool)
        return py_cast(Bool, py)
    end
    if CPython.py_equal_identity(t, MyPyAPI.complex)
        return py_cast(Complex, py)
    end
    if CPython.py_equal_identity(t, MyPyAPI.ndarray)
        return CPython.from_ndarray(py)
    end
    if CPython.py_equal_identity(t, MyPyAPI.tuple)
        n = length(t)
        return Tuple(reasonable_unbox(t[i-1]) for i in 1:n)
    end
    return py
end


function reasonable_box(x::Any)::Py
    # fast path
    if x === nothing
        return MyPyAPI.none
    end
    if x isa Core.BuiltinInts
        return py_cast(Py, x)
    end
    if x isa Union{Float16, Float32, Float64}
        return py_cast(Py, x)
    end
    if x isa String
        return py_cast(Py, x)
    end
    if x isa Bool
        return py_cast(Py, x)
    end
    if x isa Union{ComplexF16, ComplexF32, ComplexF64}
        return py_cast(Py, x)
    end

    # semantics

    if x isa AbstractArray
        return py_cast(AbstractArray, x)
    end

    if x isa Integer
        return py_cast(Py, x)
    end

    if x isa AbstractFloat
        return py_cast(Py, x)
    end

    if x isa Complex
        return py_cast(Py, x)
    end

    if x isa AbstractString
        return py_cast(Py, x)
    end

    if x isa Tuple
        N = length(x)
        argtuple = PyAPI.PyTuple_New(N)
        for i = 1:N
            arg = reasonable_box(args[i])
            PyAPI.Py_IncRef(arg)
            PyAPI.PyTuple_SetItem(argtuple, i-1, arg)
        end
        return Py(argtuple)
    end

    return box_julia(x)
end

@export_py function jl_call(self::Py, args::Py, kwargs::Py)::Py
    if !is_type_exact(self, MyPyAPI.JV)
        error("The first argument must be a Julia object.")
    end

    if is_type_exact(args, MyPyAPI.tuple)
        error("JV.__call__: args must be a tuple.")
    end

    if is_type_exact(kwargs, MyPyAPI.dict)
        error("JV.__call__: kwargs must be a dict.")
    end

    nargs = length(args)
    nkwargs = length(args)
    jlargs = Any[]
    for i = 0:nargs-1
        push!(jlargs, reasonable_unbox(args[i]))
    end

    jlkwargs = Pair{Symbol, Any}[]
    kwargs_iter = MyPyAPI.iter(kwargs)
    for _ = 0:nkwargs-1
        k = MyPyAPI.next(kwargs_iter)
        sym = Symbol(py_cast(String, k))
        v = reasonable_unbox(kwargs[k])
        push!(jlkwargs, sym => v)
    end

    jv = unbox_julia(self)
    return reasonable_box(jv(jlargs...; jlkwargs...))
end


@export_py function jl_getattr(self::Py, attr::String)::Py
    n = attr_name_to_symbol(attr)
    return reasonable_box(getproperty(unbox_julia(self), n))
end

@export_py function jl_setattr(self::Py, attr::String, val::Py)::Py
    n = attr_name_to_symbol(attr)
    setproperty!(unbox_julia(self), n, reasonable_unbox(val))
    return PyAPI.Py_None
end

@export_py function jl_getitem(self::Py, item::Py)::Py
    if is_type_exact(item, MyPyAPI.tuple)
        reasonable_box(
            getindex(unbox_julia(self), reasonable_unbox(item)...))
    else
        reasonable_box(
            getindex(unbox_julia(self), reasonable_unbox(item)))
    end
end

@export_py function jl_setitem(self::Py, item::Py, val::Py)::Py
    if is_type_exact(item, MyPyAPI.tuple)
        setindex!(
            unbox_julia(self),
            reasonable_unbox(val),
            reasonable_unbox(item)...)
    else
        setindex!(
            unbox_julia(self),
            reasonable_unbox(val),
            reasonable_unbox(item))
    end
    return PyAPI.Py_None
end

@export_py function jl_add(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) + reasonable_unbox(other))
end

@export_py function jl_sub(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) - reasonable_unbox(other))
end

@export_py function jl_mul(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) .* reasonable_unbox(other))
end

@export_py function jl_matmul(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) * reasonable_unbox(other))
end

@export_py function jl_truediv(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) / reasonable_unbox(other))
end

@export_py function jl_floordiv(self::Py, other::Py)::Py
    reasonable_box(div(unbox_julia(self), reasonable_unbox(other)))
end

@export_py function jl_mod(self::Py, other::Py)::Py
    reasonable_box(Base.mod(unbox_julia(self), reasonable_unbox(other)))
end

@export_py function jl_pow(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) ^ reasonable_unbox(other))
end

@export_py function jl_lshift(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) << reasonable_unbox(other))
end

@export_py function jl_rshift(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) >> reasonable_unbox(other))
end

@export_py function jl_bitor(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) | reasonable_unbox(other))
end

@export_py function jl_bitxor(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) ⊻ reasonable_unbox(other))
end

@export_py function jl_bitand(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) & reasonable_unbox(other))
end

@export_py function jl_eq(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) == reasonable_unbox(other))
end

@export_py function jl_ne(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) != reasonable_unbox(other))
end

@export_py function jl_lt(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) < reasonable_unbox(other))
end

@export_py function jl_le(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) <= reasonable_unbox(other))
end

@export_py function jl_gt(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) > reasonable_unbox(other))
end

@export_py function jl_ge(self::Py, other::Py)::Py
    reasonable_box(unbox_julia(self) >= reasonable_unbox(other))
end

@export_py function jl_contains(self::Py, other::Py)::Py
    reasonable_box(reasonable_unbox(other) in unbox_julia(self))
end

@export_py function jl_invert(self::Py)::Py
    reasonable_box(~unbox_julia(self))
end

@export_py function jl_bool(self::Py)::Bool
    # TODO: fast path
    o = unbox_julia(self)
    if o isa Number
        return o == 0
    end
    if (o isa AbstractArray || o isa AbstractDict || o isa AbstractSet
        || o isa AbstractString || o isa AbstractPattern)
        return isempty(o)
    end
    return false
end

@export_py function jl_pos(self::Py)::Py
    reasonable_box(+unbox_julia(self))
end

@export_py function jl_neg(self::Py)::Py
    reasonable_box(-unbox_julia(self))
end

@export_py function jl_abs(self::Py)::Py
    reasonable_box(abs(unbox_julia(self)))
end

@export_py function jl_hash(self::Py)::UInt64
    reasonable_box(hash(unbox_julia(self)))
end

@export_py function jl_repr(self::Py)::String
    address = reinterpret(UInt, CPython.unsafe_unwrap(self))
    "<JV(" * repr(unbox_julia(self)) * ") at $(address)>"
end

@export_py function jl_display(self::Py)::String
    display(unbox_julia(self))
end

@export_py function setup_jv(jvt::Py)::Nothing
    jvt.__invoke__ = Pyfunc(jl_call)
    jvt.__getattr__ = Pyfunc(jl_getattr)
    jvt.__setattr__ = Pyfunc(jl_setattr)
    jvt.__getitem__ = Pyfunc(jl_getitem)
    jvt.__setitem__ = Pyfunc(jl_setitem)
    jvt.__add__ = Pyfunc(jl_add)
    jvt.__sub__ = Pyfunc(jl_sub)
    jvt.__mul__ = Pyfunc(jl_mul)
    jvt.__matmul__ = Pyfunc(jl_matmul)
    jvt.__truediv__ = Pyfunc(jl_truediv)
    jvt.__floordiv__ = Pyfunc(jl_floordiv)
    jvt.__mod__ = Pyfunc(jl_mod)
    jvt.__pow__ = Pyfunc(jl_pow)
    jvt.__lshift__ = Pyfunc(jl_lshift)
    jvt.__rshift__ = Pyfunc(jl_rshift)
    jvt.__bitor__ = Pyfunc(jl_bitor)
    jvt.__bitxor__ = Pyfunc(jl_bitxor)
    jvt.__bitand__ = Pyfunc(jl_bitand)
    jvt.__eq__ = Pyfunc(jl_eq)
    jvt.__ne__ = Pyfunc(jl_ne)
    jvt.__lt__ = Pyfunc(jl_lt)
    jvt.__le__ = Pyfunc(jl_le)
    jvt.__gt__ = Pyfunc(jl_gt)
    jvt.__ge__ = Pyfunc(jl_ge)
    jvt.__contains__ = Pyfunc(jl_contains)
    jvt.__invert__ = Pyfunc(jl_invert)
    jvt.__bool__ = Pyfunc(jl_bool)
    jvt.__pos__ = Pyfunc(jl_pos)
    jvt.__neg__ = Pyfunc(jl_neg)
    jvt.__abs__ = Pyfunc(jl_abs)
    jvt.__hash__ = Pyfunc(jl_hash)
    jvt.__repr__ = Pyfunc(jl_repr)
    jvt._repr_pretty_ = Pyfunc(jl_display)
    nothing
end



# this is called after CPython.init()
function init()
    builtins = CPython.get_py_builtin()
    numpy = CPython.get_numpy()
    MyPyAPI.class = builtins.type
    MyPyAPI.dict = builtins.dict
    MyPyAPI.next = builtins.next
    MyPyAPI.tuple = builtins.tuple
    MyPyAPI.none = builtins.None
    MyPyAPI.int = builtins.int
    MyPyAPI.float = builtins.float
    MyPyAPI.bool = builtins.bool
    MyPyAPI.str = builtins.str
    MyPyAPI.complex = builtins.complex
    MyPyAPI.ndarray = numpy.ndarray
    @export_pymodule _tyjuliacall_jnumpy begin
        setup_jv = Pyfunc(setup_jv)
    end
end

function __init__()
    empty!(_store_string_symbols)
    empty!(JlValuePools)
    empty!(JlValueUnusedSlots)
end

end