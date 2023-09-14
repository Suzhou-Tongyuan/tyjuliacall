import tyjuliacall
import numpy as np
import _tyjuliacall_jnumpy # type: ignore


print(_tyjuliacall_jnumpy.jl_square(np.random.rand(2,3)))

# a = _tyjuliacall_jnumpy.jl_eval("rand(2,3)")
# s = _tyjuliacall_jnumpy.jl_display(a)
# print(s)
