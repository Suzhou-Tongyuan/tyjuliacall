using Libdl
using TyJuliaCAPI
import TyPython.CPython
import TyPython.CPython: Py, py_cast, UnsafeNew, PyObject, BorrowReference

# const _jl_eval_string = Ref{Ptr{Cvoid}}(C_NULL)
# const _jl_exception_occurred = Ref{Ptr{Cvoid}}(C_NULL)
const _get_capi = Ref{Ptr{Cvoid}}(C_NULL)
const _tojlpy = Ref{Ptr{Cvoid}}(C_NULL)

function toJLPy(p::Ptr{Cvoid})
    py = Py(BorrowReference(), reinterpret(CPython.C.Ptr{CPython.PyObject}, p))
    jv = TyJuliaCAPI.JV_ALLOC(py)
    return jv
end

get_tojlpy() = @cfunction(toJLPy, TyJuliaCAPI.JV, (Ptr{Cvoid},))

const LibJuliaCall = Ref{Ptr{Cvoid}}(C_NULL)

function boot()
    # _jl_eval_string[] = cglobal(:jl_eval_string)
    # _jl_exception_occurred[] = cglobal(:jl_exception_occurred)

    _get_capi[] = TyJuliaCAPI.get_capi_getter()
    _tojlpy[] = get_tojlpy()
    LibJuliaCall[] = dlopen(joinpath(@__DIR__, "libjuliacall"))
    init_LibJuliaCall = dlsym(LibJuliaCall[], :init_libjuliacall)
    err = ccall(
        init_LibJuliaCall,
        Cint,
        (Ptr{Cvoid}, Ptr{Cvoid}),
        _get_capi[], _tojlpy[]
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
    PyModule = Py(CPython.UnsafeNew(), CPython.C.Ptr{PyObject}(LP_pym))

    CPython.G_PyBuiltin.__import__(py_cast(Py, "sys")).modules[py_cast(Py, "_tyjuliacall_jnumpy")] = PyModule

    return nothing
end







