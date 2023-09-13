#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <tyjuliacapi.hpp>
#include <common.hpp>
#include <assert.h>
#include <time.h>

DLLEXPORT int init_libjuliacall(void *lpfnJLCApiGetter, void *lpfnJlToJLPy)
{
  if (to_JLPy != NULL)
  {
    return 0;
  }

  if (library_init((_get_capi_t)(lpfnJLCApiGetter)) == 0)
  {
    printf("library_init failed: TyJuliaCAPI is invalid\n");
    return 1;
  }

  to_JLPy = (t_to_JLPy)lpfnJlToJLPy;

  return 0;
}

static double jl_square(float x)
{
  // a simple example: call julia function and convert between julia and native types
  JV jv_x;
  ToJLFloat64(&jv_x, x);

  if (f_jl_square == JV_NULL)
  {
    const char *_square_code = "square(x) = x^2";
    char *square_code = const_cast<char *>(_square_code);
    JLEval(&f_jl_square, NULL, SList_adapt(reinterpret_cast<uint8_t *>(square_code), strlen(square_code)));
  }

  JV jret;
  JV arguments[1];
  arguments[0] = jv_x;
  JLCall(&jret, f_jl_square, SList_adapt(&arguments[0], 1), emptyKwArgs());

  double ret;
  JLGetDouble(&ret, jret, false);

  JLFreeFromMe(jv_x);
  JLFreeFromMe(jret);

  return ret;
}

static PyObject *square_wrapper(PyObject *self, PyObject *args)
{
  // convert bewteen python and native types
  double input, result;
  if (!PyArg_ParseTuple(args, "d", &input))
  {
    return NULL;
  }
  result = jl_square(input);
  return PyFloat_FromDouble(result);
}

static void PyCapsule_Destruct_JuliaAsPython(PyObject *capsule)
{
  // destruct of capsule(__jlslot__)
  JV *jv = (JV *)PyCapsule_GetPointer(capsule, NULL);
  JLFreeFromMe(*jv);
  free(jv);
}

static PyObject *box_julia(JV jv)
{
  // JV(julia value) -> PyObject(python's JV with __jlslot__)
  JV *ptr_boxed = (JV *)malloc(sizeof(JV));
  *ptr_boxed = jv;

  PyObject *capsule = PyCapsule_New(
      ptr_boxed,
      NULL,
      &PyCapsule_Destruct_JuliaAsPython);

  PyObject *pyjv = PyObject_CallObject(MyPyAPI.t_JV, NULL);
  if (pyjv == NULL)
  {
    PyErr_SetString(PyExc_RuntimeError, "box_julia: failed to create a new instance of JV");
    return NULL;
  }

  PyObject_SetAttr(pyjv, name_slot, capsule);
  return pyjv;
}

static JV *unbox_julia(PyObject *pyjv)
{
  // assume pyjv is a python's JV instance with __jlslot__
  PyObject *capsule = PyObject_GetAttr(pyjv, name_slot);
  JV *jv = (JV *)PyCapsule_GetPointer(capsule, NULL);
  Py_DecRef(capsule);
  return jv;
}

static PyObject *jl_eval_wrapper(PyObject *self, PyObject *args)
{
  const char *_command;
  JV result;
  if (!PyArg_ParseTuple(args, "s", &_command))
  {
    return NULL;
  }
  char *command = const_cast<char *>(_command);
  ErrorCode ret = JLEval(&result, NULL, SList_adapt(reinterpret_cast<uint8_t *>(command), strlen(command)));
  if (ret != ErrorCode::ok)
  {
    char errorBytes[2048] = {(char)0};
    if (ErrorCode::ok == JLError_FetchMsgStr(&errorSym, SList_adapt(reinterpret_cast<uint8_t *>(errorBytes), sizeof(errorBytes))))
    {
      PyErr_SetString(JuliaCallError, errorBytes);
    }
    else
    {
      PyErr_SetString(JuliaCallError, "juliacall: unknown error");
    }
    return NULL;
  }
  return box_julia(result);
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
    Py_IncRef(arg);
    init_JLAPI();
    init_PyAPI(arg);
  }

  Py_INCREF(Py_None);
  return Py_None;
}

static PyObject *jl_display(PyObject *self, PyObject *arg)
{
  // 1. unbox jv from arg (use unbox_julia)
  // 2. call julia function "repr" to get julia's string
  // 3. convert julia's string to native string
  // 4. wrap native string to python string and return

  // note: 可能很多地方都会注意到错误处理，可以暂时简单地标记一下，对错误处理后面会整理成一些框架或者宏
  // note: 可以暂时先假设入参一定是 Python 的 JV 类型
  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef example_methods[] = {
    {"jl_square", square_wrapper, METH_VARARGS, "Square function"},
    {"jl_eval", jl_eval_wrapper, METH_VARARGS, "eval julia function and return a python capsule"},
    {"setup_api", setup_api, METH_O, "setup JV class and init MyPyAPI/MyJLAPI"},
    {"jl_display", jl_display, METH_O, "display JV as string"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef example_module = {PyModuleDef_HEAD_INIT, "_tyjuliacall_jnumpy",
                                            NULL, -1, example_methods};

DLLEXPORT PyObject *init_PyModule(void)
{
  name_slot = PyUnicode_FromString("__jlslot__");
  JuliaCallError = PyErr_NewException("_tyjuliacall_jnumpy.error", NULL, NULL);
  return PyModule_Create(&example_module);
}
