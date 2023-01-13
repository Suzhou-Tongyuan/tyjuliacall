const G_pymodule = Py(UnsafeNew())
const PyJuliaBase_Type = Py(UnsafeNew())
const Pyobject_Type = Py(UnsafeNew())
const PyJV_Type = Py(UnsafeNew())

function _init_object_type()
    builtins = get_py_builtin()
    pyobject = unsafe_unwrap(builtins.object)
    PyAPI.Py_IncRef(pyobject)
    unsafe_set!(Pyobject_Type, pyobject)
end

function _init_pymodule()
    builtins = get_py_builtin()
    _tyjuliacall_jnumpy = builtins.__import__(py_cast(Py, "types")).SimpleNamespace()
    _tyjuliacall_jnumpy.name = py_cast(Py, "_tyjuliacall_jnumpy")
    builtins.__import__(py_cast(Py, "sys")).modules[py_cast(Py, "_tyjuliacall_jnumpy")] = _tyjuliacall_jnumpy
    unsafe_set!(G_pymodule, unsafe_unwrap(_tyjuliacall_jnumpy))
end

function init_jlwrap()
    _init_object_type()
    _init_pymodule()
    _init_juliabase()
    G_pymodule.JuliaBase = PyJuliaBase_Type
    _init_jv_type()
    _init_mypyapi()
    pyMain = auto_box(Main)
    pyBase = auto_box(Base)
    pyjl_eval = auto_box(jl_evaluate)
    G_pymodule.Main = pyMain
    G_pymodule.Base = pyBase
    G_pymodule.evaluate = pyjl_eval
    nothing
end

function _init_jv_type()
    builtins = get_py_builtin()
    builtins.exec(builtins.compile(py_cast(Py,"""
    $("\n"^(@__LINE__()-1))
    class JV(JuliaBase):
        __slots__ = ()

        def _repr_pretty_(self, p, cycle):
            p.text(self._jl_repr_pretty_() if not cycle else "...")

        def __iter__(self):
            pair = self._jl_first_iter_()
            while pair is not None:
                element, state = pair
                yield element
                pair = self._jl_next_iter_(state)
    """), py_cast(Py,@__FILE__()), py_cast(Py, "exec")), G_pymodule.__dict__)
    o = unsafe_unwrap(G_pymodule.JV)
    PyAPI.Py_IncRef(o)
    unsafe_set!(PyJV_Type, o)
end
