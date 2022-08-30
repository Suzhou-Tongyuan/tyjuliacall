# TyJuliaCall

同元封装的Python调用Julia方案。

特性：

1. 提供Python-Julia环境同步功能。

    执行以下命令后，当前环境的Python和Julia环境正常链接，且解决PythonCall和PyCall冲突。

    ```bash
    python -c "import tyjuliacall"
    ```

2. 支持Python import语法导入任意Julia包

    ```python
    from tyjuliacall.Julia包名 import Julia函数或变量名
    ```

## 安装

1. 设置永久环境变量：

    ```bash
    JULIA_CONDAPKG_BACKEND=Null
    PYTHON_JULIAPKG_OFFLINE=yes
    JULIA_PYTHONCALL_EXE="@PyCall"
    ```

2. 安装Python版本>=3.7，要求Python以动态链接方式编译，并预装如下包:

    - matplotlib (TyPlot)
    - PyQt5 (TyPlot)
    - scipy (TySignalProcessing)

3. 安装`tyjuliacall`:

    ```bash
    cd <TyJuliaCall文件夹>
    pip install .
    ```

4. 安装Julia版本>=1.6，并先后预装PyCall和PythonCall包。


此外，对Julia用户，如果想要使用PythonCall前，应先导入PyCall。

## 映像加速

1. 使用当前环境中的Python和Julia创建映像 (需要在当前环境中安装 `tyjuliacall`)

    ```bash
    mk-ty-sysimage <Julia包1> <Julia包2> ... --out <映像输出路径> --python <可选：使用的Python解释器路径，默认为当前Python>

    # 打包PyCall, PythonCall, TySignalProcessing, DataFrames, 输出到'a.dll'，使用python解释器'xxx/python.exe'
    mk-ty-sysimage PyCall PythonCall TySignalProcessing DataFrames --out a.dll --python xxx/python.exe
    ```

    注意：如果用`--python`指定Python环境，则该python需要已安装`tyjuliacall`。**如果该环境是conda环境，则需要预先开启相应的conda环境。**

    如果需要使用Syslab自带的Python制作映像，建议使用Syslab配置环境，流程如下：

    1. 在Syslab中选择Python解释器，激活环境。如果Syslab切换/激活Python环境失败，尝试启动VSCode进行相同操作。

    2. 激活Python环境后，在Syslab/VSCode终端中使用`mk-ty-sysimage`命令。

2. 使用映像:

    ```python
    from tyjuliasetup import use_sysimage
    use_sysimage("<映像路径>")

    # 设置镜像后，导入tyjuliacall包
    import tyjuliacall
    ```

## 受信赖的Python-Julia数据类型转换

虽然tyjuliacall允许在Python和Julia之间传递任意数据，但由于是两门不同的语言，数据转换的类型对应关系是复杂的。

为了保证代码的后向兼容性，使得规范的代码在不同版本的Syslab/tyjuliacall上都可以运行，最好只使用如下的数据类型转换。

### Python数据传递到Julia

Python向Julia函数传参时，推荐只使用下表左边的数据类型，以保证代码的后向兼容。

|  Python Type | Julia Type  |
|:-----:|:----:|
| `int` | 尽可能`Int64`，否则`BigInt`|
| `float` | `Float64` |
| `bool` | `Bool` |
| `complex` | `ComplexF64` |
| `None`  | `nothing` |
| `str`   | `String` |
| `bytes`     | `PyArray{UInt8}` (不可变) |
| `bytearray`     | `PyArray{UInt8}` (可变) |
| `tuple`，且元素均为表中数据类型 | `Tuple` |
| `numpy`的array (元素为数值) | `PyArray` |
| `juliacall.ArrayValue/juliacall.VectorValue` | `PyArray` |


对于Python传递给Julia的`tuple`，其各个元素按照以上规则依次转换。

对于Python传递给Julia的`numpy` array，不做任何拷贝。

此外，tyjuliacall在传递可变数据 (如数组) 时，不做任何拷贝，这在大多数情况下是一个明显的性能提升，但在特殊情况下可能引起性能下降。
例如，当传入大数组进行矩阵运算时，可以在传参前将其拷贝为原生Julia数组 (使用`collect(array)`)，性能会明显提升。

```python
from tyjuliacall import TySignalProcessing as sp  # type: ignore
from tyjuliacall import Base  # type: ignore
import numpy as np

fs = 10000
t = np.arange(fs + 1) / fs
x = np.sin(2 * np.pi * t*3) + 0.25*np.sin(2 * np.pi * t*40)

y = sp.medfilt1(x, 9)

# 使用collect拷贝传参更快的例子：
%timeit sp.medfilt1(x, 9)
# 3.96 ms ± 918 µs per loop (mean ± std. dev. of 7 runs, 100 loops each)
%timeit sp.medfilt1(Base.collect(x), 9)
# 2.64 ms ± 255 µs per loop (mean ± std. dev. of 7 runs, 100 loops each)


# 使用collect拷贝传参更慢的例子：
%timeit Base.sum(x)
# 15.4 µs ± 305 ns per loop (mean ± std. dev. of 7 runs, 100,000 loops each)
%timeit Base.sum(collect(x))
# 44.7 µs ± 29.5 µs per loop (mean ± std. dev. of 7 runs, 10,000 loops each)
```

### Julia数据传递到Python

获取Julia函数的返回值，或导入Julia的非函数对象时，将发生Julia到Python的数据传递。

保证后向兼容的Julia到Python数据转换关系如下表所示：

|  Julia Type | Python  |
|:-----:|:----:|
| 有符号数和无符号数 | `int`|
| `Float64` | `float` |
| `Bool` | `bool` |
|  `ComplexF16, ComplexF32, ComplexF64` | `complex` |
| `nothing` | `None`  |
| `AbstractString` | `str`   |
| `PyArray{T, 1}` (T为数值类型) | `juliacall.VectorValue` |
| `PyArray{T, N}` (T为数值类型, N不为2) | `juliacall.ArrayValue` |
| `Tuple` | `tuple` |

此外，可以使用`numpy.asarray`将Julia返回的VectorValue或ArrayValue转为`numpy`的array。但手动转换通常不是必要的：`VectorValue`和`ArrayValue`可以与`numpy`的array做运算，且所有的numpy API如`np.mean`，都对它们工作。


## Troubleshooting

1. 导入`tyjuliacall`时，报错 `ERROR: InitError: AssertionError: pystr_asstring(jl.__version__) == string(VERSION)`。

    解决：删除`$JULIA_DEPOT_PATH\packages\PythonCall`文件夹，并在Julia中重装PythonCall。

