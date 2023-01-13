module TyJuliaSetup

using TyPython
using TyPython.CPython
import TyPython.C
import TyPython.CPython: UnsafeNew, BorrowReference, PyObject, Py_ssize_t, PyTypeObject, PyMethodDef, Py_NULLPTR
import TyPython.CPython: PyAPI, unsafe_set!, unsafe_unwrap, G_PyBuiltin, py_seterror!, capture_out, _to_py_error
import TyPython.CPython: attribute_symbol_to_pyobject

# if isdefined(Base, :Experimental) && isdefined(Base.Experimental, Symbol("@compiler_options"))
#     @eval Base.Experimental.@compiler_options compile=min optimize=0 infer=no
# end

export auto_box, auto_unbox

const IsInit = Ref(false)

include("exception.jl")
include("box.jl")
include("unbox.jl")
include("ops.jl")
include("pymeths.jl")
include("juliabase.jl")
include("boot.jl")

function init()
    if IsInit[]
        return nothing
    end
    CPython.init()
    init_jlwrap()
    IsInit[] = true
    return nothing
end

precompile(init, ())

end # module
