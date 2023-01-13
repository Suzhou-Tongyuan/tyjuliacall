const G_STRING_SYM_MAP = Dict{String, Symbol}()
function attribute_string_to_symbol(x::String)
    get!(G_STRING_SYM_MAP, x) do
        Symbol(x)
    end
end

mutable struct RequiredFromPythonAPIStruct
    JV::Py
    class::Py  # the 'type' object in Python
    next::Py
    none::Py
    iter::Py
    # builtin datatypes
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

function is_type_exact(x::Union{Py,C.Ptr{PyObject}}, t::Py)::Bool
    reinterpret(UInt, unsafe_unwrap(x).type[]) === reinterpret(UInt, unsafe_unwrap(t))
end

function classof(x::Py)::Py
    Py(
        BorrowReference(),
        reinterpret(
            C.Ptr{PyObject},
            unsafe_unwrap(x).type[])
    )
end

function auto_unbox(py::Py)
    if CPython.py_equal_identity(py, MyPyAPI.none)
        return nothing
    end
    if is_type_exact(py, MyPyAPI.JV)
        return PyJuliaValue_GetValue(unsafe_unwrap(py))
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
                    shape = auto_unbox(py.shape)
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
        return Tuple(auto_unbox(py[py_cast(Py, i-1)]) for i in 1:n)
    end
    error("unbox failed: cannot convert a Python object (type: $(classof(py))) to julia value.")
end

function auto_unbox_args(pyargs::Py, nargs::Int)
    args = Vector{Any}(undef, nargs)
    for i in 1:nargs
        args[i] = auto_unbox(Py(NewReference(), PyAPI.PyTuple_GetItem(pyargs, i-1)))
    end
    return args
end

function auto_unbox_kwargs(pykwargs::Py, nkwargs::Int)
    kwargs = Pair{Symbol, Any}[]
    pyk = PyAPI.PyDict_Keys(pykwargs)
    pyv = PyAPI.PyDict_Values(pykwargs)
    for i in 1:nkwargs
        k = auto_unbox(Py(NewReference(), PyAPI.PyList_GetItem(pyk, i-1)))
        v = auto_unbox(Py(NewReference(), PyAPI.PyList_GetItem(pyv, i-1)))
        push!(kwargs, attribute_string_to_symbol(k) => v)
    end
    return kwargs
end

function _init_mypyapi()
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
    MyPyAPI.JV = PyJV_Type
end
