from __future__ import annotations
import contextlib
import subprocess
import io
import json
import os
import sys
import pathlib
import typing
from types import ModuleType

PYTHONPATH = pathlib.Path(sys.executable).resolve().as_posix()


def invoke_interpreted_julia(
    jl_exepath: str, args: list[str], *, supress_errors: bool = True
):
    try:
        if supress_errors:
            with contextlib.redirect_stderr(io.StringIO()):
                return subprocess.check_output(
                    [
                        jl_exepath,
                        "--startup-file=no",
                        "-O0",
                        "--compile=min",
                        *args,
                    ]
                )
        else:
            return subprocess.check_output(
                [
                    jl_exepath,
                    "--startup-file=no",
                    "-O0",
                    "--compile=min",
                    *args,
                ]
            )
    except subprocess.CalledProcessError:
        return None


def escape_string(s: str):
    return json.dumps(s, ensure_ascii=False)


def escape_to_julia_rawstr(s: str):
    return "raw" + escape_string(s)


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
        env[varname] = value

    return set


class ENV:
    PYTHON_JULIAPKG_OFFLINE: str
    PYTHON_JULIAPKG_PROJECT: str
    PYTHON_JULIAPKG_EXE: str

    JULIA_PYTHONCALL_PROJECT: str
    JULIA_PYTHONCALL_LIBPTR: str
    JULIA_PYTHONCALL_EXE: str
    JULIA_CONDAPKG_BACKEND: str

    PYTHON_JULIACALL_OPTIMIZE: str
    PYTHON_JULIACALL_COMPILE: str
    JULIA_DEPOT_PATH: str
    PYTHON: str

    TYPY_JL_SYSIMAGE: str

    PATH: str
    HOME: str

    def __init__(self, env=None):
        self._env = env

    def add_path(self, s: str):
        sections = self.PATH.split(os.pathsep)
        if s not in sections:
            self.PATH = os.pathsep.join((s, *sections))


for varname in ENV.__annotations__:
    setattr(ENV, varname, env_descriptor(varname))

Environment = ENV()


class JuliaModule(ModuleType):
    def __new__(cls, loader, name, jl_mod):
        if isinstance(jl_mod, ModuleType):
            return jl_mod
        o = ModuleType.__new__(cls)
        o.__init__(loader, name, jl_mod)
        return o

    def __init__(self, loader, name, jl_mod):
        self.__it = jl_mod
        self.__loader__ = loader
        self.__name__ = name

    def __getattr__(self, name):
        return getattr(self.__it, name)

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
    from juliacall import Main  # type: ignore

    M = Main.seval("import {0};{0}".format(fullnames[0]))
    for submodulename in fullnames[1:]:
        M = Main.getfield(M, Main.Symbol(submodulename))
    return M


class _JuliaCodeEvaluatorClass:
    _eval_func: typing.Any

    def __init__(self, engine: typing.Literal["pycall", "pythoncall"]):
        self._pycall_eval = None
        self._pythoncall_eval = None
        self.engine = engine

    def assure_pycall(self):
        if self._pycall_eval is None:
            from julia import Main  # type: ignore

            self._pycall_eval = Main.eval
        return self._pycall_eval

    def assure_pythoncall(self):
        if self._pythoncall_eval is None:
            from juliacall import Main  # type: ignore

            self._pythoncall_eval = Main.seval
        return self._pythoncall_eval

    def __getitem__(self, arg):
        if self.engine == "pycall":
            eval_func = self.assure_pycall()
        else:
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


JuliaEvaluator = _JuliaCodeEvaluatorClass("pythoncall")
LegacyJuliaEvaluator = _JuliaCodeEvaluatorClass("pycall")


def setup():
    Environment.PYTHON_JULIAPKG_OFFLINE = "yes"
    Environment.JULIA_CONDAPKG_BACKEND = "Null"
    Environment.JULIA_PYTHONCALL_EXE = "@PyCall"
    Environment.add_path(os.path.dirname(PYTHONPATH))
    Environment.PYTHON = PYTHONPATH
    extra_opts = []
    if Environment.TYPY_JL_SYSIMAGE:
        extra_opts.extend(["--sysimage", Environment.TYPY_JL_SYSIMAGE])
    result = invoke_interpreted_julia(
        "julia", [*extra_opts, "-e", "import PyCall;import PythonCall;println(PyCall.python)"]
    )
    if isinstance(result, bytes):
        result = result.decode("utf-8")
    if (
        not isinstance(result, str)
        or pathlib.Path(result.strip()).resolve().as_posix() != PYTHONPATH
    ):
        if (
            invoke_interpreted_julia(
                "julia",
                [*extra_opts, "-e", r'import Pkg;Pkg.build("PyCall");import PyCall;import PythonCall;'],
                supress_errors=False,
            )
            is None
        ):
            raise RuntimeError(
                "julia or PyCall.jl is not installed or failed to build."
            )
        # sync PyCall and PythonCall
        if Environment.TYPY_JL_SYSIMAGE:
            from julia import Julia

            Julia(sysimage=Environment.TYPY_JL_SYSIMAGE)

    from julia import Pkg  # type: ignore

    Environment.TYPY_JL_SYSIMAGE = typing.cast(
        str, LegacyJuliaEvaluator["unsafe_string(Base.JLOptions().image_file)"]
    )
    import juliacall

    Pkg.activate()  # workaround to prevent juliacall from creating it own project


def create_image(*juliapkgs: str, out: str):
    assert all(isinstance(p, str) for p in juliapkgs), "pkgs must be a list of strings"
    assert isinstance(out, str), "sysimage_path must be a string"
    baseimage_path = typing.cast(
        str, JuliaEvaluator["unsafe_string(Base.JLOptions().image_file)"]
    )
    assert isinstance(baseimage_path, str), baseimage_path
    pkgs = list(juliapkgs)
    if "PyCall" not in pkgs:
        pkgs.append("PyCall")
    if "PythonCall" not in pkgs:
        pkgs.append("PythonCall")
    sysimage_path = os.path.abspath(out)
    JuliaEvaluator[
        "import PackageCompiler",
        rf"""PackageCompiler.create_sysimage(
            Symbol.({json.dumps(pkgs)}),
            sysimage_path = {escape_to_julia_rawstr(sysimage_path)},
            cpu_target = PackageCompiler.default_app_cpu_target()
        )""",
    ]


setup()
sys.meta_path.insert(0, JuliaFinder())  # type: ignore
