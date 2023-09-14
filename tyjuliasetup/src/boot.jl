using Libdl
using TyJuliaCAPI
import TyPython.CPython
import TyPython.CPython: Py, py_cast, UnsafeNew, PyObject, BorrowReference

# const _jl_eval_string = Ref{Ptr{Cvoid}}(C_NULL)
# const _jl_exception_occurred = Ref{Ptr{Cvoid}}(C_NULL)
const _get_capi = Ref{Ptr{Cvoid}}(C_NULL)
const _pycast2jl = Ref{Ptr{Cvoid}}(C_NULL)
const _pycast2py = Ref{Ptr{Cvoid}}(C_NULL)

function pycast2jl(out::Ptr{TyJuliaCAPI.JV}, T::Int64, p::Ptr{Cvoid})
    py = Py(BorrowReference(), reinterpret(CPython.C.Ptr{CPython.PyObject}, p))
    t = TyJuliaCAPI.JTypeFromIdent(T)
    try
        p = TyJuliaCAPI.JV_ALLOC(py_cast(t,py))
        unsafe_store!(out, p)
    catch e
        TyJuliaCAPI.produce_error!(e)
        return TyJuliaCAPI.ERROR
    end
    return TyJuliaCAPI.OK
end

get_pycast2jl() = @cfunction(pycast2jl, TyJuliaCAPI.ErrorCode, (Ptr{TyJuliaCAPI.JV}, Int64, Ptr{Cvoid}, ))

function pycast2py(v::TyJuliaCAPI.JV)
    v′ = TyJuliaCAPI.JV_LOAD(v)
    try
        py = py_cast(TyPython.CPython.Py, v′)
        TyPython.CPython.PyAPI.Py_IncRef(py)
        ptr = reinterpret(Ptr{TyPython.CPython.PyObject}, TyPython.CPython.unsafe_unwrap(py))
        return ptr
    catch e
        TyJuliaCAPI.produce_error!(e)
        return C_NULL
    end
end

get_pycast2py() = @cfunction(pycast2py, Ptr{TyPython.CPython.PyObject}, (TyJuliaCAPI.JV, ))

const LibJuliaCall = Ref{Ptr{Cvoid}}(C_NULL)

function boot()
    # _jl_eval_string[] = cglobal(:jl_eval_string)
    # _jl_exception_occurred[] = cglobal(:jl_exception_occurred)

    _get_capi[] = TyJuliaCAPI.get_capi_getter()
    _pycast2jl[] = get_pycast2jl()
    _pycast2py[] = get_pycast2py()
    LibJuliaCall[] = dlopen(joinpath(@__DIR__, "libjuliacall"))
    init_LibJuliaCall = dlsym(LibJuliaCall[], :init_libjuliacall)
    err = ccall(
        init_LibJuliaCall,
        Cint,
        (Ptr{Cvoid}, Ptr{Cvoid}, Ptr{Cvoid}),
        _get_capi[], _pycast2jl[], _pycast2py[]
    )
    if err != 0
        error("Failed to initialize LibJuliaCall")
    end

    init_PyModule = dlsym(LibJuliaCall[], :init_PyModule)
    LP_pym = ccall(
        init_PyModule,
        Ptr{Cvoid},
        ()
    )
    # PyModule = Py(CPython.NewReference(), CPython.C.Ptr{PyObject}(LP_pym))

    # CPython.G_PyBuiltin.__import__(py_cast(Py, "sys")).modules[py_cast(Py, "_tyjuliacall_jnumpy")] = PyModule

    return nothing
end

precompile(boot, ())







