from __future__ import annotations
from . import compat
import time
import contextlib
import ctypes
import shutil
import subprocess
import io
import os
import sys
import pathlib
import typing
import jnumpy
import shlex
from types import ModuleType

PYTHONPATH = pathlib.Path(sys.executable).resolve().as_posix()
_PYJULIA_CORE = None

del compat


def invoke_julia(jl_exepath: str, args: list[str], *, supress_errors: bool = True):
    try:
        if supress_errors:
            with contextlib.redirect_stderr(io.StringIO()):
                return subprocess.check_output(
                    [
                        jl_exepath,
                        "--startup-file=no",
                        *args,
                    ],
                    env=os.environ,
                )
        else:
            return subprocess.check_output(
                [
                    jl_exepath,
                    "--startup-file=no",
                    *args,
                ],
                env=os.environ,
            )
    except subprocess.CalledProcessError:
        return None


def env_descriptor(varname):
    @property
    def get(self):
        if self._env is None:
            env = os.environ
        else:
            env = self._env
        return env.get(varname, "")

    @get.setter
    def set(self, value):
        if self._env is None:
            env = os.environ
        else:
            env = self._env
        if not value:
            if varname in env:
                del env[varname]
        else:
            env[varname] = value

    return set


class ENV:
    PYTHON: str
    PYCALL_INPROC_LIBPYPTR: str
    PYCALL_INPROC_PROCID: str
    TYPY_JL_SYSIMAGE: str
    TYPY_JL_OPTS: str
    TYPY_VERBOSE: str
    PATH: str
    HOME: str

    PYJULIA_CORE: str

    JULIA_PYTHONCALL_LIBPTR: str
    JULIA_PYTHONCALL_EXE: str
    JULIA_CONDAPKG_BACKEND: str
    PYTHON_JULIAPKG_OFFLINE: str
    PYTHON_JULIACALL_SYSIMAGE: str

    def __init__(self, env=None):
        self._env = env

    def add_path(self, s: str):
        sections = self.PATH.split(os.pathsep)
        if s not in sections:
            self.PATH = os.pathsep.join((s, *sections))


for varname in ENV.__annotations__:
    setattr(ENV, varname, env_descriptor(varname))

Environment = ENV()


def _get_pyjulia_core_provider():
    global _PYJULIA_CORE
    if _PYJULIA_CORE is None:
        _PYJULIA_CORE = Environment.PYJULIA_CORE.lower() or "jnumpy"
    return _PYJULIA_CORE


def _load_pyjulia_core() -> ModuleType:
    pyjulia_core_provider = _get_pyjulia_core_provider()

    if pyjulia_core_provider == "pycall":
        import _pyjulia_core  # type: ignore

        return _pyjulia_core
    elif pyjulia_core_provider == "jnumpy":
        import _tyjuliacall_jnumpy  # type: ignore

        return _tyjuliacall_jnumpy
    else:
        raise EnvironmentError(
            "Unknown PyJulia-Core provider: {0}".format(pyjulia_core_provider)
        )


@contextlib.contextmanager
def tictoc(msg):
    t0 = time.time()
    try:
        yield
    finally:
        if Environment.TYPY_VERBOSE:
            print(msg.format(time.time() - t0))


class JuliaModule(ModuleType):
    _jlapi: typing.Any

    def __new__(cls, loader, name, jl_mod):
        pyjulia_core = _load_pyjulia_core()  # type: ignore

        o = ModuleType.__new__(cls)
        object.__setattr__(o, "_jlapi", pyjulia_core)
        o.__init__(loader, name, jl_mod)
        return o

    def __init__(self, loader, name, jl_mod):
        self.__it = jl_mod
        self.__loader__ = loader
        self.__name__ = name
        self.__package__ = name
        self.__spec__ = None

    def __getattr__(self, name):
        return getattr(self.__it, name)

    def __dir__(self):
        return list(self._jlapi.Main.names(self.__it, all=True, imported=True))

    __path__ = []

    @property
    def __all__(self):
        return dir(self.__it)

    def __repr__(self):
        return "JuliaModule(%s)" % self.__it


class JuliaFinder:
    def find_module(self, fullname: str, path=None):
        if fullname.startswith("tyjuliacall."):
            return JuliaLoader
        return


class JuliaLoader:
    @classmethod
    def load_module(cls, name):
        fullnames = name.split(".")[1:]
        m = JuliaModule(cls, name, _jl_using(fullnames))
        sys.modules[name] = m
        return m


def _jl_using(fullnames: tuple[str, ...]):
    pyjulia_core = _load_pyjulia_core()
    evaluate = pyjulia_core.evaluate
    Main = pyjulia_core.Main

    M = evaluate("import {0};{0}".format(fullnames[0]))
    for submodulename in fullnames[1:]:
        M = Main.getfield(M, Main.Symbol(submodulename))
    return M


def use_sysimage(path: str | pathlib.Path):
    """
    This function only works before importing `tyjuliacall`.
    """
    if not isinstance(path, pathlib.Path):
        path = pathlib.Path(path)
    Environment.TYPY_JL_SYSIMAGE = path.absolute().as_posix()


def use_system_typython(yes: bool = True):
    pass

def use_backend(backend : typing.Literal['pycall', 'jnumpy']):
    Environment.PYJULIA_CORE = backend

class _JuliaCodeEvaluatorClass:
    _eval_func: typing.Any

    def __init__(self):
        self._eval_func = None

    def assure_pythoncall(self):
        if self._eval_func is None:
            self._eval_func = _load_pyjulia_core().evaluate
        return self._eval_func

    def __getitem__(self, arg) -> typing.Any:
        eval_func = self.assure_pythoncall()
        o = None
        if isinstance(arg, tuple):
            for code in arg:
                if isinstance(code, str):
                    o = eval_func(code)
                else:
                    raise TypeError(f"Code must be a string, got {code!r}")
        elif isinstance(arg, str):
            o = eval_func(arg)
        else:
            raise TypeError(f"Code must be a string, got {arg!r}")
        return o


JuliaEvaluator = _JuliaCodeEvaluatorClass()


def setup():
    global BASE_IMAGE
    jl_exe = shutil.which("julia")
    if not jl_exe:
        raise RuntimeError("Julia not found")

    # sync PyCall and PythonCall
    if Environment.TYPY_JL_SYSIMAGE:
        BASE_IMAGE = Environment.TYPY_JL_SYSIMAGE
    else:
        sysimage = invoke_julia(
            jl_exe,
            [
                "--compile=min",
                "-O0",
                "-e",
                "println(unsafe_string(Base.JLOptions().image_file))",
            ],
        )
        if not sysimage or not isinstance(sysimage, bytes) or not sysimage.strip():
            raise ValueError("Julia.exe failed")
        BASE_IMAGE = sysimage.strip().decode("utf-8")

    Environment.PYTHON = PYTHONPATH
    Environment.PYCALL_INPROC_LIBPYPTR = hex(ctypes.pythonapi._handle)
    Environment.PYCALL_INPROC_PROCID = str(os.getpid())
    Environment.TYPY_JL_OPTS = shlex.join(["--sysimage", BASE_IMAGE])
    Environment.add_path(os.path.dirname(PYTHONPATH))

    # in case that users work with PythonCall
    Environment.JULIA_CONDAPKG_BACKEND = "Null"
    Environment.PYTHON_JULIAPKG_OFFLINE = "yes"
    Environment.JULIA_PYTHONCALL_EXE = "@PyCall"
    Environment.PYTHON_JULIACALL_SYSIMAGE = BASE_IMAGE

    lib: typing.Any = None

    def _init(_lib):
        nonlocal lib
        lib = _lib

    user_set_pyjulia_core = Environment.PYJULIA_CORE
    with tictoc("Julia initialized in {} seconds"):
        jnumpy.init.init_libjulia(_init, experimental_fast_init=True)

    # to workaround sysimage `__init__`
    if user_set_pyjulia_core:
        Environment.PYJULIA_CORE = user_set_pyjulia_core
    else:
        Environment.PYJULIA_CORE = "jnumpy"

    pyjulia_core_provider = _get_pyjulia_core_provider()
    with tictoc("PyJulia-Core initialized in {} seconds"):
        if pyjulia_core_provider == "jnumpy":
            jnumpy.init.init_jl_from_lib(lib)
            jnumpy.init_project(__file__)
            jnumpy.exec_julia("Pkg.activate(io=devnull)")

            import _tyjuliacall_jnumpy  # type: ignore
            from tyjuliasetup import jv

            _tyjuliacall_jnumpy.setup_jv(jv.JV, jv)
            _tyjuliacall_jnumpy.setup_basics(_tyjuliacall_jnumpy)
            _tyjuliacall_jnumpy.JV = jv.JV
        elif pyjulia_core_provider == "pycall":
            lib.jl_eval_string("import PyCall".encode("utf-8"))
            lib.jl_eval_string("Pkg.activate(io=devnull)".encode("utf-8"))
            _load_pyjulia_core()
        else:
            raise EnvironmentError(
                "Unknown PyJulia-Core provider: {0}".format(pyjulia_core_provider)
            )
