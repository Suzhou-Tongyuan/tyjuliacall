function handle_except(e::Exception)
    if e isa CPython.PyException
        CPython.PyAPI.PyErr_SetObject(e.type, e.value)
    else
        errmsg = capture_out() do
            Base.showerror(stderr, e, catch_backtrace())
        end
        py_seterror!(_to_py_error(e), errmsg)
    end
    return Py_NULLPTR
end

function handle_except(e::Exception, pyerr::Py)
    if e isa CPython.PyException
        CPython.PyAPI.PyErr_SetObject(e.type, e.value)
    else
        errmsg = capture_out() do
            Base.showerror(stderr, e, catch_backtrace())
        end
        py_seterror!(pyerr, errmsg)
    end
    return Py_NULLPTR
end
