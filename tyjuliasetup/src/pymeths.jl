function _pyjl_repr(self_::C.Ptr{PyObject})
    if PyJuliaValue_IsNull(self_)
        ans = py_cast(Py, "<JV NULL>")
        out = unsafe_unwrap(ans)
        PyAPI.Py_IncRef(out)
        return out
    end
    self = PyJuliaValue_GetValue(self_)
    try
        address = reinterpret(UInt, CPython.unsafe_unwrap(self_))
        ans = py_cast(Py, "<JV($(repr(self))) at $(repr(address))>")
        out = unsafe_unwrap(ans)
        PyAPI.Py_IncRef(out)
        return out
    catch e
        handle_except(e)
    end
end

function _pyjl_call(self_::C.Ptr{PyObject}, pyargs::C.Ptr{PyObject}, pykwargs::C.Ptr{PyObject})
    if PyJuliaValue_IsNull(self_)
        py_seterror!(G_PyBuiltin.TypeError, "Julia object is NULL")
        return Py_NULLPTR
    end
    self = PyJuliaValue_GetValue(self_)
    nargs = PyAPI.PyTuple_Size(pyargs)
    try
        if pykwargs !== Py_NULLPTR
            nkwargs = PyAPI.PyDict_Size(pykwargs)
            args = auto_unbox_args(Py(NewReference(), pyargs), nargs)
            kwargs = auto_unbox_kwargs(Py(NewReference(), pykwargs), nkwargs)
            ans = auto_box(self(args...; kwargs...))
        elseif nargs > 0
            args = auto_unbox_args(Py(NewReference(), pyargs), nargs)
            ans = auto_box(self(args...))
        else
            ans = auto_box(self())
        end
        out  = unsafe_unwrap(ans)
        PyAPI.Py_IncRef(out)
        return out
    catch e
        handle_except(e)
    end
end

function _pyjl_getattr(self_::C.Ptr{PyObject}, k_::C.Ptr{PyObject})
    if PyJuliaValue_IsNull(self_)
        py_seterror!(G_PyBuiltin.TypeError, "Julia object is NULL")
        return Py_NULLPTR
    end
    self = PyJuliaValue_GetValue(self_)
    try
        k = attribute_string_to_symbol(py_coerce(String, Py(NewReference(), k_)))
        ans = auto_box(getproperty(self, k))
        out = unsafe_unwrap(ans)
        PyAPI.Py_IncRef(out)
        return out
    catch e
        handle_except(e, G_PyBuiltin.AttributeError)
    end
end

function _pyjl_setattr(self_::C.Ptr{PyObject}, vectorargs::Ptr{C.Ptr{PyObject}}, n::Py_ssize_t)
    if PyJuliaValue_IsNull(self_)
        py_seterror!(G_PyBuiltin.TypeError, "Julia object is NULL")
        return Py_NULLPTR
    end
    self = PyJuliaValue_GetValue(self_)
    if n != 2
        py_seterror!(G_PyBuiltin.TypeError, "__setattr__ takes exactly 2 arguments")
        return Py_NULLPTR
    end
    k_ = unsafe_load(vectorargs, 1)
    v_ = unsafe_load(vectorargs, 2)
    try
        k = attribute_string_to_symbol(py_coerce(String, Py(NewReference(), k_)))
        v = auto_unbox(Py(NewReference(), v_))
        setproperty!(self, k, v)
        out = unsafe_unwrap(PyAPI.Py_None)
        PyAPI.Py_IncRef(out)
        return out
    catch e
        handle_except(e, G_PyBuiltin.AttributeError)
    end
end

function _pyjl_getitem(self_::C.Ptr{PyObject}, item_::C.Ptr{PyObject})
    if PyJuliaValue_IsNull(self_)
        py_seterror!(G_PyBuiltin.TypeError, "Julia object is NULL")
        return Py_NULLPTR
    end
    self = PyJuliaValue_GetValue(self_)
    try
        if is_type_exact(item_, MyPyAPI.tuple)
            ans = auto_box(getindex(self, auto_unbox(Py(NewReference(), item_))...))
        else
            ans = auto_box(getindex(self, auto_unbox(Py(NewReference(), item_))))
        end
        out = unsafe_unwrap(ans)
        PyAPI.Py_IncRef(out)
        return out
    catch e
        handle_except(e)
    end
end

function _pyjl_setitem(self_::C.Ptr{PyObject}, vectorargs::Ptr{C.Ptr{PyObject}}, n::Py_ssize_t)
    if PyJuliaValue_IsNull(self_)
        py_seterror!(G_PyBuiltin.TypeError, "Julia object is NULL")
        return Py_NULLPTR
    end
    self = PyJuliaValue_GetValue(self_)
    if n != 2
        py_seterror!(G_PyBuiltin.TypeError, "__setitem__ takes exactly 2 arguments")
        return Py_NULLPTR
    end
    item_ = unsafe_load(vectorargs, 1)
    val_ = unsafe_load(vectorargs, 2)
    try
        if is_type_exact(item_, MyPyAPI.tuple)
            setindex!(
                self,
                auto_unbox(Py(NewReference(), val_)),
                auto_unbox(Py(NewReference(), item_))...
            )
        else
            setindex!(
                self,
                auto_unbox(Py(NewReference(), val_)),
                auto_unbox(Py(NewReference(), item_))
            )
        end
        out = unsafe_unwrap(PyAPI.Py_None)
        PyAPI.Py_IncRef(out)
        return out
    catch e
        handle_except(e)
    end
end
