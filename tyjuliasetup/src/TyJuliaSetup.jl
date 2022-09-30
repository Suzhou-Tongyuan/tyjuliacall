module TyJuliaSetup
using TyPython
using TyPython.CPython
using TyPython.CPython: PyAPI, Py_NULLPTR, py_throw
using TyPython: C

if isdefined(Base, :Experimental) && isdefined(Base.Experimental, Symbol("@compiler_options"))
    @eval Base.Experimental.@compiler_options compile=min optimize=0 infer=no
end

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
    iter::Py
    # buultin datatypes
    dict::Py
    object::Py

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
    Py(
        CPython.BorrowReference(),
        reinterpret(
            C.Ptr{CPython.PyObject},
            CPython.unsafe_unwrap(x).type[])
    )
end

function is_type_exact(x::Py, t::Py)::Bool
    reinterpret(UInt, CPython.unsafe_unwrap(x).type[]) === reinterpret(UInt, CPython.unsafe_unwrap(t))
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
    MyPyAPI.object.__setattr__(
        jv,
        CPython.attribute_symbol_to_pyobject(:__jlslot__),
        capsule
    )
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
    if is_type_exact(py, MyPyAPI.JV)
        return unbox_julia(py)
    end
    if is_type_exact(py, MyPyAPI.int)
        return py_cast(Int, py)
    end
    if is_type_exact(py, MyPyAPI.float)
        return py_cast(Float64, py)
    end
    if is_type_exact(py, MyPyAPI.str)
        return py_cast(String, py)
    end
    if is_type_exact(py, MyPyAPI.bool)
        return py_cast(Bool, py)
    end
    if is_type_exact(py, MyPyAPI.complex)
        return py_cast(ComplexF64, py)
    end
    if is_type_exact(py, MyPyAPI.ndarray)
        try
            return CPython.from_ndarray(py)
        catch
            typename = py_cast(String, py.dtype.name)
            if startswith(typename, "str")
                let data = String[]
                    flat_pyarray = py.flatten()
                    shape = reasonable_unbox(py.shape)
                    for i = 0:length(flat_pyarray)-1
                        elem = flat_pyarray[py_cast(Py, i)]
                        push!(data, py_cast(String, elem))
                    end
                    return reshape(data, shape)
                end
            else
                return box_julia(py)
            end
        end
    end
    if is_type_exact(py, MyPyAPI.tuple)
        n = length(py)
        return Tuple(reasonable_unbox(py[py_cast(Py, i-1)]) for i in 1:n)
    end
    error("unbox failed: cannot convert a Python object (type: $(classof(py))) to julia value.")
end

const JNumPySupportedNumPyArrayBoxingElementTypes = Union{
    Int8, Int16, Int32, Int64, UInt8, UInt16, UInt32, UInt64,
    Float16, Float32, Float64,
    ComplexF16, ComplexF32, ComplexF64, Bool
}

function reasonable_box(x::Any)::Py
    # fast path
    if x === nothing
        return MyPyAPI.none
    end

    if x isa Union{Int8, Int16, Int32, Int64, UInt8, UInt16, UInt32, UInt64}
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
        elt = eltype(typeof(x))
        if x isa BitArray
            return box_julia(x)
        elseif elt <: JNumPySupportedNumPyArrayBoxingElementTypes
            return py_cast(Py, x)
        else
            return box_julia(x)
        end
    end

    if x isa Integer
        return py_cast(Py, convert(Int, x))
    end

    if x isa AbstractFloat
        return py_cast(Py, convert(Float64, x))
    end

    if x isa Complex
        return py_cast(Py, convert(ComplexF64, x))
    end

    if x isa AbstractString
        return py_cast(Py, convert(String, x))
    end

    if x isa Tuple
        N = length(x)
        argtuple = PyAPI.PyTuple_New(N)
        for i = 1:N
            arg = reasonable_box(x[i])
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

    if !is_type_exact(args, MyPyAPI.tuple)
        error("JV.__call__: args must be a tuple, got ($(classof(args))).")
    end

    if !is_type_exact(kwargs, MyPyAPI.dict)
        error("JV.__call__: kwargs must be a dict.")
    end

    nargs = length(args)
    nkwargs = length(kwargs)
    jlargs = Any[]
    for i = 0:nargs-1
        arg = args[py_cast(Py, i)]
        jlarg = reasonable_unbox(arg)
        push!(jlargs, jlarg)
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
    # Python multi-indexing is translated to indexing using a tuple.
    # So we do multi-indexing if `item` is a tuple.
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
        return o != 0
    end
    if (o isa AbstractArray || o isa AbstractDict ||
        o isa AbstractSet || o isa AbstractString)
        return !isempty(o)
    end
    # return `true` is the default semantics of a Python object
    return true
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

@export_py function jl_hash(self::Py)::Int64
    hash(unbox_julia(self)) % Int64
end

@export_py function jl_repr(self::Py)::String
    address = reinterpret(UInt, CPython.unsafe_unwrap(self))
    "<JV(" * repr(unbox_julia(self)) * ") at $(repr(address))>"
end

@export_py function jl_display(self::Py)::String
    old_stdout = stdout
    rd, wr = redirect_stdout()
    try
        show(wr, "text/plain", unbox_julia(self))
    finally
        try
            close(wr)
        catch
        end
        redirect_stdout(old_stdout)
    end
    read(rd, String)
end

@export_py function setup_jv(jvt::Py, jv_module::Py)::Nothing
    MyPyAPI.JV = jvt
    jv_module.__jl_invoke__ = Pyfunc(jl_call)
    jv_module.__jl_getattr__ = Pyfunc(jl_getattr)
    jv_module.__jl_setattr__ = Pyfunc(jl_setattr)
    jv_module.__jl_getitem__ = Pyfunc(jl_getitem)
    jv_module.__jl_setitem__ = Pyfunc(jl_setitem)
    jv_module.__jl_add__ = Pyfunc(jl_add)
    jv_module.__jl_sub__ = Pyfunc(jl_sub)
    jv_module.__jl_mul__ = Pyfunc(jl_mul)
    jv_module.__jl_matmul__ = Pyfunc(jl_matmul)
    jv_module.__jl_truediv__ = Pyfunc(jl_truediv)
    jv_module.__jl_floordiv__ = Pyfunc(jl_floordiv)
    jv_module.__jl_mod__ = Pyfunc(jl_mod)
    jv_module.__jl_pow__ = Pyfunc(jl_pow)
    jv_module.__jl_lshift__ = Pyfunc(jl_lshift)
    jv_module.__jl_rshift__ = Pyfunc(jl_rshift)
    jv_module.__jl_bitor__ = Pyfunc(jl_bitor)
    jv_module.__jl_bitxor__ = Pyfunc(jl_bitxor)
    jv_module.__jl_bitand__ = Pyfunc(jl_bitand)
    jv_module.__jl_eq__ = Pyfunc(jl_eq)
    jv_module.__jl_ne__ = Pyfunc(jl_ne)
    jv_module.__jl_lt__ = Pyfunc(jl_lt)
    jv_module.__jl_le__ = Pyfunc(jl_le)
    jv_module.__jl_gt__ = Pyfunc(jl_gt)
    jv_module.__jl_ge__ = Pyfunc(jl_ge)
    jv_module.__jl_contains__ = Pyfunc(jl_contains)
    jv_module.__jl_invert__ = Pyfunc(jl_invert)
    jv_module.__jl_bool__ = Pyfunc(jl_bool)
    jv_module.__jl_pos__ = Pyfunc(jl_pos)
    jv_module.__jl_neg__ = Pyfunc(jl_neg)
    jv_module.__jl_abs__ = Pyfunc(jl_abs)
    jv_module.__jl_hash__ = Pyfunc(jl_hash)
    jv_module.__jl_repr__ = Pyfunc(jl_repr)
    jv_module._jl_repr_pretty_ = Pyfunc(jl_display)
    nothing
end

function evaluate(s::String)
    Base.eval(Main, Meta.parseall(s))
end

@export_py function setup_basics(ns::Py)::Nothing
    ns.Base = reasonable_box(Base)
    ns.Main = reasonable_box(Main)
    ns.evaluate = reasonable_box(evaluate)
    nothing
end

# this is called after CPython.init()
function init()
    builtins = CPython.get_py_builtin()
    numpy = CPython.get_numpy()
    MyPyAPI.iter = builtins.iter
    MyPyAPI.class = builtins.type
    MyPyAPI.dict = builtins.dict
    MyPyAPI.next = builtins.next
    MyPyAPI.tuple = builtins.tuple
    MyPyAPI.none = builtins.None
    MyPyAPI.int = builtins.int
    MyPyAPI.float = builtins.float
    MyPyAPI.bool = builtins.bool
    MyPyAPI.str = builtins.str
    MyPyAPI.object = builtins.object
    MyPyAPI.complex = builtins.complex
    MyPyAPI.ndarray = numpy.ndarray
    @export_pymodule _tyjuliacall_jnumpy begin
        setup_jv = Pyfunc(setup_jv)
        setup_basics = Pyfunc(setup_basics)
    end
    nothing
end

precompile(init, ())

function __init__()
    empty!(_store_string_symbols)
    empty!(JlValuePools)
    empty!(JlValueUnusedSlots)
end

end