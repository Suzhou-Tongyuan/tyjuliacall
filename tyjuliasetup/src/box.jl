const JNumPySupportedNumPyArrayBoxingElementTypes = Union{
    Int8, Int16, Int32, Int64, UInt8, UInt16, UInt32, UInt64,
    Float16, Float32, Float64,
    ComplexF16, ComplexF32, ComplexF64, Bool
}

function auto_box(x::Any)::Py
    # fast path
    if x === nothing
        return py_cast(Py, nothing)
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
            arg = auto_box(x[i])
            PyAPI.Py_IncRef(arg)
            PyAPI.PyTuple_SetItem(argtuple, i-1, arg)
        end
        return Py(argtuple)
    end

    return box_julia(x)
end


function box_julia(v)
    @nospecialize v
    o = Py(PyJuliaValue_New(unsafe_unwrap(PyJV_Type), v))
    return o
end

box_julia(v::Py) = v
