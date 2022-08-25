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

2. 安装Python版本>=3.7，要求Python以动态链接方式编译，并预装matplotlib。

3. 安装Julia版本>=1.6，并先后预装PyCall和PythonCall包。


此外，对Julia用户，如果想要使用PythonCall前，应先导入PyCall。

## 镜像加速

Python用户设置`TYPY_JL_SYSIMAGE`设置为镜像路径。


## Troubleshooting

1. 导入`tyjuliacall`时，报错 `ERROR: InitError: AssertionError: pystr_asstring(jl.__version__) == string(VERSION)`。

    解决：删除`$JULIA_DEPOT_PATH\packages\PythonCall`文件夹，并在Julia中重装PythonCall。


