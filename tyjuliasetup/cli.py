from __future__ import annotations
import wisepy2
import subprocess


@wisepy2.wise
def mkimage(*pkgs: str, out, python: str = ""):
    """Create a sysimage with PyCall and PythonCall bundled.
    e.g.,
        mk-ty-sysimage DataFrames TyDSPSystem  --python "<Python exe path>"
    """
    from tyjuliasetup import PYTHONPATH, Environment, assure_setupenv

    python = python or PYTHONPATH
    Environment.TYJULIASETUP_PKGS = ";".join(pkgs)
    Environment.TYJULIASETUP_SYSIMAGE_OUT = out
    subprocess.check_call(
        [python, "-c", rf"import tyjuliacall;tyjuliacall.create_image()"]
    )
