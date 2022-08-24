from __future__ import annotations
import contextlib
import subprocess
import io
import json
import os
import sys
import pathlib
from types import ModuleType

PYTHONPATH = pathlib.Path(sys.executable).resolve().as_posix()

def invoke_interpreted_julia(jl_exepath: str, args: list[str], *, supress_errors: bool = True):
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
        return env.get(varname, '')

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

    PYTHON_JULIACALL_SYSIMAGE: str
    PYTHON_JULIACALL_OPTIMIZE: str
    PYTHON_JULIACALL_COMPILE: str
    JULIA_DEPOT_PATH : str
    PYTHON: str

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
        fullnames = name.split('.')[1:]
        m = JuliaModule(cls, name, _jl_using(fullnames))
        sys.modules[name] = m
        return m

def _jl_using(fullnames: tuple[str, ...]):
    from juliacall import Main  # type: ignore
    M = Main.seval("import {0};{0}".format(fullnames[0]))
    for submodulename in fullnames[1:]:
        M = Main.getfield(M, Main.Symbol(submodulename))
    return M

def setup():
    Environment.PYTHON_JULIAPKG_OFFLINE = 'yes'
    Environment.JULIA_CONDAPKG_BACKEND = "Null"
    Environment.JULIA_PYTHONCALL_EXE = "@PyCall"
    Environment.add_path(os.path.dirname(PYTHONPATH))
    Environment.PYTHON = PYTHONPATH
    result = invoke_interpreted_julia("julia", ['-e', 'import PyCall;println(PyCall.python)'])
    if isinstance(result, bytes):
        result = result.decode('utf-8')
    if not isinstance(result, str) or pathlib.Path(result.strip()).resolve().as_posix() != PYTHONPATH:
        if invoke_interpreted_julia("julia", ["-e", r'import Pkg;Pkg.build("PyCall");import PyCall'], supress_errors=False) is None:
            raise RuntimeError("julia or PyCall.jl is not installed")
    from julia import Main  # type: ignore
    import juliacall

setup()
sys.meta_path.insert(0, JuliaFinder()) # type: ignore
