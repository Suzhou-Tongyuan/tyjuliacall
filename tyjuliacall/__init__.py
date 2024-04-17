from tyjuliasetup import *
from tyjuliasetup import _load_pyjulia_core

setup()

JV = _load_pyjulia_core().JV
sys.meta_path.insert(0, JuliaFinder())  # type: ignore


def _setup_help():
    import pydoc

    class Helper(object):
        def __init__(self):
            if isinstance(__builtins__, dict):
                self.help = __builtins__['help']
            else:
                self.help = __builtins__.help

        def __call__(self, obj):
            if isinstance(obj, JV):
                # get Julia documentation
                jdoc_func = JuliaEvaluator["jdoc_func = Base.string ∘ Base.Docs.doc"]
                return pydoc.pager(jdoc_func(obj))
            else:
                return self.help(obj)

    if isinstance(__builtins__, dict):
        __builtins__['help'] = Helper()
    else:
        __builtins__.help = Helper()


_setup_help()
del _setup_help
