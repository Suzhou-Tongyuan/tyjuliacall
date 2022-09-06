from tyjuliasetup import *
from tyjuliasetup.jv import JV

setup()
sys.meta_path.insert(0, JuliaFinder())  # type: ignore
