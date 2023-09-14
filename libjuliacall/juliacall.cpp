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

static PyObject *square_wrapper(PyObject *self, PyObject *args)
{
  JV v = reasonable_unbox(args);

  JV jret;
  JV arguments[1];
  arguments[0] = v;
  JLCall(&jret, MyJLAPI.f_square, SList_adapt(&arguments[0], 1), emptyKwArgs());

  PyObject* py = reasonable_box(jret);
  JLFreeFromMe(jret);
  return py;
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
    // TODO: better error handle
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

  // convert Julia's String to Python's str.
  // pycast2py is a C Function from julia,
  // we could use JLGetUTF8String and PyUnicode_FromString instead,
  // but this one is simple.
  PyObject* pyjv = pycast2py(jret);

  // don't need this, free it
  JLFreeFromMe(jret);
  return pyjv;
}

static PyObject *jl_getattr(PyObject *self, PyObject *args)
{
  // TODO
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
    // TODO: better error handle
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

  PyObject* pyout = reasonable_box(out);

  return pyout;
}


static PyMethodDef methods[] = {
    {"jl_square", square_wrapper, METH_O, "Square function"},
    {"jl_eval", jl_eval_wrapper, METH_VARARGS, "eval julia function and return a python capsule"},
    {"setup_api", setup_api, METH_O, "setup JV class and init MyPyAPI/MyJLAPI"},
    {"jl_display", jl_display, METH_O, "display JV as string"},
    {"jl_getattr", jl_getattr, METH_VARARGS, "get attr of JV object"},
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
