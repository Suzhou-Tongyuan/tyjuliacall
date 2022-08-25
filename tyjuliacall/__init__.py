from tyjuliasetup import *

setup()
sys.meta_path.insert(0, JuliaFinder())  # type: ignore
