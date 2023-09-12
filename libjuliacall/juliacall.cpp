#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <tyjuliacapi.hpp>
#include <assert.h>
#include <time.h>

typedef void *(*t_jl_eval_string)(const char *code);
typedef struct _jl_value_t jl_value_t;
typedef jl_value_t *(*t_jl_exception_occurred)();
static t_jl_eval_string jl_eval_string = NULL;
static t_jl_exception_occurred jl_exception_occurred = NULL;
static PyObject *JuliaCallError;
static PyObject *JVType;
static PyObject *name_slot;
static JSym errorSym;
static JV f_jl_square;

DLLEXPORT int init_libjuliacall(void *lpfnJlEvalString, void *lpfnJlExceptionOccurred, void *lpfnJLCApiGetter)
{
  jl_eval_string = (t_jl_eval_string)lpfnJlEvalString;
  jl_exception_occurred = (t_jl_exception_occurred)lpfnJlExceptionOccurred;

  if (library_init((_get_capi_t)(lpfnJLCApiGetter)) == 0)
  {
    printf("library_init failed: TyJuliaCAPI is invalid\n");
    return 1;
  }
  return 0;
}

static double jl_square(float x)
{
  JV jv_x;
  ToJLFloat64(&jv_x, x);

  if (f_jl_square == 0)
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
  JV *jv = (JV *)PyCapsule_GetPointer(capsule, NULL);
  JLFreeFromMe(*jv);
  free(jv);
}

static PyObject *box_julia(JV jv)
{
  JV *ptr_boxed = (JV *)malloc(sizeof(JV));
  *ptr_boxed = jv;

  PyObject *capsule = PyCapsule_New(
      ptr_boxed,
      NULL,
      &PyCapsule_Destruct_JuliaAsPython);

  PyObject *args = PyTuple_New(0);
  PyObject *pyjv = PyObject_CallObject(JVType, args);
  PyObject_SetAttr(pyjv, name_slot, capsule);

  Py_DecRef(args);
  return pyjv;
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

static PyObject *setup_jv(PyObject *self, PyObject *arg)
{
  // check arg type
  if (!PyType_Check(arg))
  {
    PyErr_SetString(PyExc_TypeError, "setup_jv: arg must be a type");
    return NULL;
  }

  if (JVType == NULL)
  {
    Py_IncRef(arg);
    JVType = arg;
  }

  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef example_methods[] = {
    {"jl_square", square_wrapper, METH_VARARGS, "Square function"},
    {"jl_eval", jl_eval_wrapper, METH_VARARGS, "eval julia function and return a python capsule"},
    {"setup_jv", setup_jv, METH_O, "setup JV class"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef example_module = {PyModuleDef_HEAD_INIT, "_tyjuliacall_jnumpy",
                                            NULL, -1, example_methods};

/* name here must match extension name, with PyInit_ prefix */
DLLEXPORT PyObject *init_PyModule(void)
{
  name_slot = PyUnicode_FromString("__jlslot__");
  JuliaCallError = PyErr_NewException("_tyjuliacall_jnumpy.error", NULL, NULL);
  return PyModule_Create(&example_module);
}