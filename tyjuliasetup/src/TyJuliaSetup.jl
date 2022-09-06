module TyJuliaSetup
using TyPython
using TyPython.CPython

if isdefined(Base, :Experimental) && isdefined(Base.Experimental, Symbol("@compiler_options"))
    @eval Base.Experimental.@compiler_options compile=min optimize=0 infer=no
end

function evalutate(s::String)
    Base.eval(Main, Meta.parseall(s))
end

const _store_string_symbols = Dict{String, Symbol}()

function attr_name_to_symbol(s::String)::Symbol
    get!(_store_string_symbols, s) do
        v = Symbol(s)
        _store_string_symbols[s] = v
        return v
    end
end

function py_getproperty(x::Any, attr::String)
    getproperty(x, attr_name_to_symbol(attr))
end

function py_setproperty(x::Any, attr::String, val::Any)
    setproperty!(x, attr_name_to_symbol(attr), val)
end

function py_applytype(x::Type, args::Any...)
    Core.apply_type(x, args...)
end

function py_call(self::Any, args::Any...; @nospecialize(kwargs...))
    self(args...; kwargs...)
end

function module_names(x::Module)
    names(x; all=true, imported=true)
end

@export_py function setup_pycall()::Nothing
    PyCall = Main.PyCall
    mod = PyCall.pyimport("_tyjuliacall")
    mod."PyCall" = PyCall
    mod."Base" = Base
    mod."Main" = Main
    mod."evaluate" = evalutate
    mod."py_getproperty" = py_getproperty
    mod."py_setproperty" = py_setproperty
    mod."py_call" = py_call
    mod."py_module_names" = module_names
    mod."py_applytype" = py_applytype
    nothing
end

function init()
    @export_pymodule _tyjuliacall begin
        setup_pycall = Pyfunc(setup_pycall)
    end
end

function __init__()
    empty!(_store_string_symbols)
end

end