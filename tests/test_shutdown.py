import ctypes

import tyjuliasetup


class FakeAtexitHook:
    def __init__(self):
        self.argtypes = None
        self.restype = object()
        self.calls = []

    def __call__(self, status):
        self.calls.append(status)


class FakeLibJulia:
    def __init__(self):
        self.jl_atexit_hook = FakeAtexitHook()


def test_registers_shutdown_and_calls_julia_atexit_once(monkeypatch):
    registered = []
    lib = FakeLibJulia()

    monkeypatch.setattr(tyjuliasetup, "_LIBJULIA", None)
    monkeypatch.setattr(tyjuliasetup, "_JULIA_SHUTDOWN", False)
    monkeypatch.setattr(tyjuliasetup, "_JULIA_ATEXIT_REGISTERED", False)
    monkeypatch.setattr(tyjuliasetup.atexit, "register", registered.append)

    tyjuliasetup._register_julia_shutdown(lib)

    assert registered == [tyjuliasetup._shutdown_julia]
    assert lib.jl_atexit_hook.argtypes == [ctypes.c_int]
    assert lib.jl_atexit_hook.restype is None

    tyjuliasetup._shutdown_julia()
    tyjuliasetup._shutdown_julia()

    assert lib.jl_atexit_hook.calls == [0]
    assert tyjuliasetup._LIBJULIA is None


def test_registers_python_atexit_only_once(monkeypatch):
    registered = []

    monkeypatch.setattr(tyjuliasetup, "_LIBJULIA", None)
    monkeypatch.setattr(tyjuliasetup, "_JULIA_SHUTDOWN", False)
    monkeypatch.setattr(tyjuliasetup, "_JULIA_ATEXIT_REGISTERED", False)
    monkeypatch.setattr(tyjuliasetup.atexit, "register", registered.append)

    tyjuliasetup._register_julia_shutdown(FakeLibJulia())
    tyjuliasetup._register_julia_shutdown(FakeLibJulia())

    assert registered == [tyjuliasetup._shutdown_julia]
