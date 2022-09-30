from tyjuliasetup import *
from tyjuliasetup import _load_pyjulia_core

setup()

JV = _load_pyjulia_core().JV
sys.meta_path.insert(0, JuliaFinder())  # type: ignore
