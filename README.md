# TyJuliaCall

Calling Julia from Python for the real world.

Features:

1. Working with Suzhou-Tongyuan hacked PyCall: https://github.com/Suzhou-Tongyuan/PyCall.jl.

2. **Cross-platform support for both dynamically linked Python and statically linked Python.**

3. **Support Julia system images.**

4. Compatible to the PyCall ecosystem.


## Installation

Prerequisites: Python (>=3.7)

1. Install Tongyuan-hacked PyCall:

    ```bash
    Pkg.add(url="https://github.com/Suzhou-Tongyuan/PyCall.jl")
    ```

2. Install the `tyjuliacall` Python package.

    ```bash
    pip install -U tyjuliacall
    ```

## Using System Images

```python
from tyjuliasetup import use_sysimage  # CAUTIOUS: not 'tyjuliacall'!
use_sysimage(r"/path/to/sysimg")
from tyjuliacall import JuliaEvaluator
print(
    "current sysimage in use",
    JuliaEvaluator["Base.unsafe_string(Base.JLOptions().image_file)"])
```

## 受信赖的Python-Julia数据类型转换

虽然tyjuliacall允许在Python和Julia之间传递任意数据，但由于是两门不同的语言，数据转换的类型对应关系是复杂的。

为了保证代码的后向兼容性，使得规范的代码在不同版本的Syslab/tyjuliacall上都可以运行，建议只使用如下的数据类型转换。

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
| `bytes`     | `String` |
| `bytearray`     | `Vector{UInt8}` |
| `tuple`，且元素均为表中数据类型 | `Tuple` |
| `numpy.ndarray` (元素为数值) | 原生`Array` |


对于Python传递给Julia的`tuple`，其各个元素按照以上规则依次转换。

### Julia数据传递到Python

当获取Julia函数返回值，或导入Julia模块的非函数对象时，将发生Julia到Python的数据传递。

保证后向兼容的Julia到Python数据转换关系如下表所示：

|  Julia Type | Python  |
|:-----:|:----:|
| 有符号数和无符号数 | `int`|
| `Float64` | `float` |
| `Bool` | `bool` |
|  `ComplexF16, ComplexF32, ComplexF64` | `complex` |
| `nothing` | `None`  |
| `AbstractString` | `str`   |
| `Vector{UInt8}` | `bytearray` |
| `Array` (T为数值类型) | `numpy.ndarray` |
| `Tuple` | `tuple` |
