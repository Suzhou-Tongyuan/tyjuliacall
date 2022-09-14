# TyJuliaCall

[![CI](https://github.com/Suzhou-Tongyuan/tyjuliacall/actions/workflows/ci.yml/badge.svg)](https://github.com/Suzhou-Tongyuan/tyjuliacall/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/Suzhou-Tongyuan/tyjuliacall/branch/master/graph/badge.svg?token=NMRDY32QIC)](https://codecov.io/gh/Suzhou-Tongyuan/tyjuliacall)
[![versions](https://img.shields.io/pypi/pyversions/tyjuliacall.svg)](https://pypi.org/project/tyjuliacall/#history)
[![pypi](https://img.shields.io/pypi/v/tyjuliacall.svg)](https://pypi.org/project/tyjuliacall/)
[![License](https://img.shields.io/badge/License-BSD_2--Clause-green.svg)](https://github.com/Suzhou-Tongyuan/tyjuliacall/blob/main/LICENSE)


Calling Julia from Python for the real world.

Features:

1. **Cross-platform support for both dynamically linked Python and statically linked Python.**

2. **Support Julia system images.**

## Installation

Prerequisites: Python (>=3.7)

Then install the `tyjuliacall` Python package.

```bash
pip install -U tyjuliacall
```

## Using System Images

```python
from tyjuliasetup import use_sysimage  # CAUTIOUS: not 'tyjuliacall'!
use_sysimage(r"/path/to/sysimg")
# if your sysimage contains TyPython,
# you could call use_system_typython() to reduce the time cost of setting up julia.
from tyjuliacall import Base
print(
    "current sysimage in use",
    Base.unsafe_string(Base.JLOptions().image_file))
# out: /path/to/sysimg
```

## 受信赖的Python-Julia数据类型转换

虽然tyjuliacall允许在Python和Julia之间传递任意数据，但由于是两门不同的语言，数据转换的类型对应关系是复杂的。

为了保证代码的后向兼容性，使得规范的代码在不同版本的Syslab/tyjuliacall上都可以运行，建议只使用如下的数据类型转换。

### Python数据传递到Julia

Python向Julia函数传参时，推荐只使用下表左边的数据类型，以保证代码的后向兼容。

|  Python Type | Julia Type  |
|:-----:|:----:|
| 基本类型 | |
| `int` | `Int64`|
| `float` | `Float64` |
| `bool` | `Bool` |
| `complex` | `ComplexF64` |
| `None`  | `nothing` |
| `str`   | `String` |
| 组合类型 |   |
| `numpy.ndarray` (dtype为数字或字符串或bool)  | 原生`Array` |
| `tuple`，且元素均为表中数据类型 | `Tuple` |

对于Python传递给Julia的`tuple`，其各个元素按照以上规则依次转换。

TIPS: 如何传递`bytearray`或者`bytes`到Julia?

1. 向Julia函数传递bytes时，可以改为传递一个uint8的数组。

   无拷贝传参： `np.array(memoryview(b'mybytes'), dtype=np.uint8)`
   拷贝传参： `np.array(list(b'mybytes'), dtype=np.uint8)`

2. 向Julia函数传递bytearray时，可以改为传递一个uint8的数组。

    无拷贝传参： `np.asarray(bytearray(b'mybytes')))`

### Julia数据传递到Python

当获取Julia函数返回值，或导入Julia模块的非函数对象时，将发生Julia到Python的数据传递。

保证后向兼容的Julia到Python数据转换关系如下表所示：

|  Julia | Python  |
|:-----:|:----:|
| 基本类型 |  |
| `Integer`子类型  | `int`|
| `AbstractFloat`子类型 | `float`|
| `Bool` | `bool` |
| `Complex`子类型 | `complex` |
| `nothing`对象 | `None`  |
| `AbstractString`子类型 | `str`   |
| `Vector{UInt8}` | `bytearray` |
| 组合类型 | |
| `AbstrctArray{T}` (T见下方说明) | `numpy.ndarray` |
| `Tuple{T1, ..., Tn}`, 且`Ti`为该表中的类型 | `tuple` |
| 其余Julia类型            | `tyjuliacall.JV` |

一个Julia AbstrctArray能转换为numpy数组，当且仅当其元素类型`T`是以下类型之一

- `Int8, Int16, Int32, Int64, UInt8, UInt16, UInt32, UInt64`
- `Float16, Float32, Float64`
- `ComplexF16, ComplexF32, ComplexF64`
- `Bool`

注意，当类型为`Vector{String}`或者`Array{String, 2}`的Julia对象被返回给Python时，它被封装为一个`tyjuliacall.JV`类型。

## 其他说明

1. 不要对Julia包/模块使用`from ... import *`。
2. `Vector{String}`传到Python是一个`tyjuliacall.JV`，这是一个纯Julia对象的包装，因此下标索引是从1开始的。
