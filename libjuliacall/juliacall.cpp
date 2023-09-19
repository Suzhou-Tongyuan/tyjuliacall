#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <tyjuliacapi.hpp>
#include <TyPython.hpp>
#include <common.hpp>
#include <assert.h>
#include <time.h>

DLLEXPORT int init_libjuliacall(void *lpfnJLCApiGetter, void *lpfnPyCast2JL, void *lpfnPyCast2Py)
{
  if (pycast2jl != NULL && pycast2py != NULL)
  {
    return 0;
  }

  if (library_init((_get_capi_t)(lpfnJLCApiGetter)) == 0)
  {
    printf("library_init failed: TyJuliaCAPI is invalid\n");
    return 1; 
  }

  pycast2jl = (t_pycast2jl)lpfnPyCast2JL;
  pycast2py = (t_pycast2py)lpfnPyCast2Py;

  return 0;
}

static PyObject *setup_api(PyObject *self, PyObject *arg)
{
  // check arg type
  if (!PyType_Check(arg))
  {
    PyErr_SetString(PyExc_TypeError, "setup_api: arg must be a type");
    return NULL;
  }

  if (MyPyAPI.t_JV == NULL)
  {
    Py_IncRef(arg); //Py_IncRef 函数用于增加 Python 对象的引用计数。
    init_JLAPI();
    init_PyAPI(arg);//自定义函数或库初始化函数的调用。
  }

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *jl_eval(PyObject *self, PyObject *args)
{
  const char *_command;
  JV result;
  if (!PyArg_ParseTuple(args, "s", &_command))
  {
    return NULL;
  }
  char *command = const_cast<char *>(_command); //const_cast 用于将一个指向常量数据的指针转换为指向非常量数据的指针。
  ErrorCode ret = JLEval(&result, NULL, SList_adapt(reinterpret_cast<uint8_t *>(command), strlen(command)));//这段代码调用了一个自定义函数 JLEval，并将 command 的数据以某种方式传递给它，然后将结果存储在 result 中。
  if (ret != ErrorCode::ok)
  {
    return HandleJLErrorAndReturnNULL();//如果是错误的话，则处理
  }
  return box_julia(result);
}

static PyObject *jl_square(PyObject *self, PyObject *args)
{
  JV v = reasonable_unbox(args);

  JV jret;
  ErrorCode ret = JLCall(&jret, MyJLAPI.f_square, SList_adapt(&v, 1), emptyKwArgs());
  if (ret != ErrorCode::ok)
  {
    return HandleJLErrorAndReturnNULL();
  }

  PyObject* py = reasonable_box(jret);//jret变成python类型
  JLFreeFromMe(jret);
  return py;
}

static PyObject *jl_display(PyObject *self, PyObject *arg)
{
  // check arg type
  if (!PyObject_IsInstance(arg, MyPyAPI.t_JV))
  {
    PyErr_SetString(JuliaCallError, "jl_display: expect object of JV class.");
    return NULL;
  }

  // unbox jv from arg (use unbox_julia)
  JV jv = unbox_julia(arg);
  JV jret;

  // call julia function repr
  ErrorCode ret = JLCall(&jret, MyJLAPI.f_repr, SList_adapt(&jv, 1), emptyKwArgs());
  if (ret != ErrorCode::ok)
  {
    return HandleJLErrorAndReturnNULL();
  }

  // convert Julia's String to Python's str.
  // pycast2py is a C Function from julia,
  // we could use JLGetUTF8String and PyUnicode_FromString instead,
  // but this one is simple.
  PyObject* pyjv = pycast2py(jret);

  // free this Julia String
  JLFreeFromMe(jret);
  return pyjv;
}

static PyObject *jl_getattr(PyObject *self, PyObject *args)
{
  PyObject* pyjv;
  const char* attr;
  if (!PyArg_ParseTuple(args, "Os", &pyjv, &attr))
  {
    return NULL;
  }

  JV slf;
  if (!PyObject_IsInstance(pyjv, MyPyAPI.t_JV))
  {
    PyErr_SetString(JuliaCallError, "jl_getattr: expect object of JV class.");
    return NULL;
  }
  else
  {
    slf = unbox_julia(pyjv);
  }

  JSym sym;
  JSymFromString(&sym, attr);

  JV out;
  ErrorCode ret = JLGetProperty(&out, slf, sym);
  if (ret != ErrorCode::ok)
  {
    return HandleJLErrorAndReturnNULL();
  }

  PyObject* pyout = reasonable_box(out);
  if (!PyObject_IsInstance(pyout, MyPyAPI.t_JV))
  {
    // if pyout is a JV object, we should not free it from Julia.
    JLFreeFromMe(out);
  }
  return pyout;
}

static PyObject *jl_setattr(PyObject *self, PyObject *args)
{
  // jl_setattr(self: JV, attr: str, value)
  // 1. check args type, we should get 3 args: PyObject* pyjv, const char* attr, PyObject* value
  PyObject* pyjv;
  PyObject* value;
  const char* attr;
  if (!PyArg_ParseTuple(args, "OsO", &pyjv, &attr,&value))
  {
    return NULL;
  }
  // 2. check pyjv is a JV object, and unbox it as JV
  JV slf;
  if (!PyObject_IsInstance(pyjv, MyPyAPI.t_JV))
  {
    PyErr_SetString(JuliaCallError, "jl_getattr: expect object of JV class.");
    return NULL;
  }
  else
  {
    slf = unbox_julia(pyjv);
  }
  // 3. unbox value as JV
  JV v = reasonable_unbox(value);
  // 4. call JLSetProperty
  JSym sym;
  JSymFromString(&sym, attr);
  ErrorCode ret = JLSetProperty(slf,sym,v);
  // 5. check if error occurs, if so, handle it and return NULL
   if (ret != ErrorCode::ok)
  {
    return HandleJLErrorAndReturnNULL();
  }

 if (!PyObject_IsInstance(value, MyPyAPI.t_JV))
  {
    // if pyout is a JV object, we should not free it from Julia.
    JLFreeFromMe(v);
  }
  // 6. if success, return Py_None
  Py_INCREF(Py_None);
  return Py_None;
}


static PyObject *jl_arithmetic_operation(PyObject *self, PyObject *args, JV f)
{
  //1. check args type
  PyObject* pyjv;
  PyObject* value;
  if (!PyArg_ParseTuple(args, "OO", &pyjv, &value))
  {
    return NULL;
  }
  // 2. check pyjv is a JV object, and unbox it as JV
  JV slf;
  if (!PyObject_IsInstance(pyjv, MyPyAPI.t_JV))
  {
    PyErr_SetString(JuliaCallError, "jl_add: expect object of JV class.");
    return NULL;
  }
  else
  {
    slf = unbox_julia(pyjv);
  }
  // 3. unbox value as JV
  JV v = reasonable_unbox(value);
  // 4. call JLCallS
  JV jret;
  JV jargs[2];
  jargs[0] = slf;
  jargs[1] = v;

  ErrorCode ret;
  ret = JLCall(&jret, f, SList_adapt(jargs, 2), emptyKwArgs());
  // 5. check if error occurs, if so, handle it and return NULL 
  if (ret != ErrorCode::ok)
  {
    return HandleJLErrorAndReturnNULL();
  }

  if (!PyObject_IsInstance(value, MyPyAPI.t_JV))
  {
    // if pyout is a JV object, we should not free it from Julia.
    JLFreeFromMe(v);
  }
  PyObject* py = reasonable_box(jret);
  JLFreeFromMe(jret);
  return py;
}

static PyObject *jl_add(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_add);
}

static PyObject *jl_sub(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_sub);
}

static PyObject *jl_mul(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_mul);
}

static PyObject *jl_matmul(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_matmul);
}

static PyObject *jl_truediv(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_truediv);
}

static PyObject *jl_floordiv(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_floordiv);
}

static PyObject *jl_mod(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_mod);
}

static PyObject *jl_pow(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_pow);
}

static PyObject *jl_lshift(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_lshift);
}

static PyObject *jl_rshift(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_rshift);
}

static PyObject *jl_bitor(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_bitor);
}

static PyObject *jl_bitxor(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_bitxor);
}

static PyObject *jl_bitand(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_bitand);
}

static PyObject *jl_eq(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_eq);
}

static PyObject *jl_ne(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_ne);
}

static PyObject *jl_lt(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_lt);
}

static PyObject *jl_le(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_le);
}

static PyObject *jl_gt(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_gt);
}

static PyObject *jl_ge(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_ge);
}

static PyObject *jl_contains(PyObject *self, PyObject *args)
{
    return jl_arithmetic_operation(self, args, MyJLAPI.f_in);
}


static PyObject *jl_unary_opertation(PyObject *self, PyObject *args, JV f)
{
  //1. check args type
  PyObject* pyjv;
  // 2. check pyjv is a JV object, and unbox it as JV
  JV slf;
  if (!PyObject_IsInstance(pyjv, MyPyAPI.t_JV))
  {
    PyErr_SetString(JuliaCallError, "jl_add: expect object of JV class.");
    return NULL;
  }
  else
  {
    slf = unbox_julia(pyjv);
  }
  // 3. call JLCallS
  JV jret;
  ErrorCode ret;
  ret = JLCall(&jret, f, SList_adapt(&slf, 1), emptyKwArgs());
  // 4. check if error occurs, if so, handle it and return NULL 
  if (ret != ErrorCode::ok)
  {
    return HandleJLErrorAndReturnNULL();
  }
  PyObject* py = reasonable_box(jret);
  JLFreeFromMe(jret);
  return py;
}

static PyObject *jl_invert(PyObject *self, PyObject *args)
{
    return jl_unary_opertation(self, args, MyJLAPI.f_invert);
}

static PyObject *jl_pos(PyObject *self, PyObject *args)
{
    return jl_unary_opertation(self, args, MyJLAPI.f_add);
}

static PyObject *jl_neg(PyObject *self, PyObject *args)
{
    return jl_unary_opertation(self, args, MyJLAPI.f_sub);
}

static PyObject *jl_abs(PyObject *self, PyObject *args)
{
    return jl_unary_opertation(self, args, MyJLAPI.f_abs);
}

static PyMethodDef methods[] = {
    {"setup_api", setup_api, METH_O, "setup JV class and init MyPyAPI/MyJLAPI"},
    {"jl_square", jl_square, METH_O, "Square function"},
    {"jl_eval", jl_eval, METH_VARARGS, "eval julia function and return a python capsule"},
    {"jl_display", jl_display, METH_O, "display JV as string"},
    {"jl_getattr", jl_getattr, METH_VARARGS, "get attr of JV object"},
    {"jl_setattr", jl_setattr, METH_VARARGS, "set attr of JV object"},
    {"jl_add",jl_add,METH_VARARGS, "add function" },
    {"jl_sub",jl_sub,METH_VARARGS, "sub function" },
    {"jl_mul",jl_mul,METH_VARARGS, "mul function" },
    {"jl_matmul",jl_matmul,METH_VARARGS, "matmul function" },
    {"jl_truediv",jl_truediv,METH_VARARGS, "truediv function" },
    {"jl_floordiv",jl_floordiv,METH_VARARGS, "floordiv function" },
    {"jl_mod",jl_mod,METH_VARARGS, "mod function" },
    {"jl_pow",jl_pow,METH_VARARGS, "pow function" },
    {"jl_lshift",jl_lshift,METH_VARARGS, "lshift function" },
    {"jl_rshift",jl_rshift,METH_VARARGS, "rshift function" },
    {"jl_bitor",jl_bitor,METH_VARARGS, "bitor function" },
    {"jl_bitxor",jl_bitxor,METH_VARARGS, "bitxor function" },
    {"jl_bitand",jl_bitand,METH_VARARGS, "bitand function" },
    {"jl_eq",jl_eq,METH_VARARGS, "eq function" },
    {"jl_ne",jl_ne,METH_VARARGS, "ne function" },
    {"jl_lt",jl_lt,METH_VARARGS, "lt function" },
    {"jl_le",jl_le,METH_VARARGS, "le function" },
    {"jl_gt",jl_gt,METH_VARARGS, "gt function" },
    {"jl_ge",jl_ge,METH_VARARGS, "ge function" },
    {"jl_contains",jl_contains,METH_VARARGS, "contains function" }, 
    {"jl_invert",jl_invert,METH_O, "invert function" },
    {"jl_pos",jl_pos,METH_O, "pos function" },
    {"jl_neg",jl_neg,METH_O, "neg function" },
    {"jl_abs",jl_abs,METH_O, "abs function" },
    {NULL, NULL, 0, NULL}}; 

static struct PyModuleDef juliacall_module = {PyModuleDef_HEAD_INIT, "_tyjuliacall_jnumpy",
                                            NULL, -1, methods};

DLLEXPORT PyObject *init_PyModule(void)
{
  name_jlslot = PyUnicode_FromString("__jlslot__");
  JuliaCallError = PyErr_NewException("_tyjuliacall_jnumpy.error", NULL, NULL);
  PyObject* m = PyModule_Create(&juliacall_module);
  PyObject* sys = PyImport_ImportModule("sys");
  PyObject* sys_module = PyObject_GetAttrString(sys, "modules");
  Py_IncRef(m);
  PyDict_SetItemString(sys_module, "_tyjuliacall_jnumpy", m);
  Py_DecRef(sys_module);
  Py_DECREF(sys);
  return m;
}
