import time


def get_timespan():
    global _t0
    t1 = time.time()
    span = t1 - _t0
    _t0 = t1
    return span


_t0 = time.time()

import tyjuliacall

print(
    f"TyJuliaCall 启动Julia+自动环境切换和检查+初始化PyCall+初始化PythonCall: {get_timespan():.2f} s"
)

import numpy as np

from tyjuliacall import TyPlot as typ
from tyjuliacall import TySignalProcessing as sp

print(f"TyJuliaCall 导入TySignalProcess, TyPlot和NumPy: {get_timespan():.2f} s")

fs = 100
t = np.arange(fs + 1) / fs
print(t)
x = np.sin(2 * np.pi * t * 3) + 0.25 * np.sin(2 * np.pi * t * 40)
print(x)


_ = get_timespan()

# 调用信号库函数
for i in range(10000):
    y = sp.medfilt1(x, 9)
print(f"运行耗时: {get_timespan():.2f}秒")

typ.plot(t, np.vstack(x), t, np.vstack(y))
typ.legend(["Original", "Filtered"])

typ.plt.show()
