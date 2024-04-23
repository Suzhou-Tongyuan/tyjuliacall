import os

from tyjuliasetup import use_sysimage

if os.path.exists("my-image.dll"):
    use_sysimage("my-image.dll")

import numpy as np

from tyjuliacall import Base  # type: ignore
from tyjuliacall import TySignalProcessing as sp  # type: ignore

fs = 10000
t = np.arange(fs + 1) / fs
x = np.sin(2 * np.pi * t * 3) + 0.25 * np.sin(2 * np.pi * t * 40)

y = sp.medfilt1(x, 9)

%timeit sp.medfilt1(x, 9)
%timeit sp.medfilt1(Base.collect(x), 9)


%timeit Base.sum(x)
%timeit Base.sum(Base.collect(x))
