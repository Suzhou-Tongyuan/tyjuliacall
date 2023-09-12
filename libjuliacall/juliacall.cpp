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

DLLEXPORT int init_libjuliacall(void* lpfnJlEvalString, void* lpfnJlExceptionOccurred, void* lpfnJLCApiGetter)
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

static JV f_jl_square;

static double jl_square(float x) {
  JV jv_x;
  ToJLFloat64(&jv_x, x);

  if (f_jl_square == 0)
  {
    const char* _square_code = "square(x) = x^2";
    char *square_code = const_cast<char*>(_square_code);
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

static PyObject *square_wrapper(PyObject *self, PyObject *args) {
  double input, result;
  if (!PyArg_ParseTuple(args, "d", &input)) {
    return NULL;
  }
  result = jl_square(input);
  return PyFloat_FromDouble(result);
}

static PyMethodDef example_methods[] = {
    {"square", square_wrapper, METH_VARARGS, "Square function"},
    {NULL, NULL, 0, NULL}};

static struct PyModuleDef example_module = {PyModuleDef_HEAD_INIT, "example",
                                             NULL, -1, example_methods};

/* name here must match extension name, with PyInit_ prefix */
DLLEXPORT PyObject* init_PyModule(void) {
  PyModule_Create(&example_module);
  return PyModule_Create(&example_module);
}