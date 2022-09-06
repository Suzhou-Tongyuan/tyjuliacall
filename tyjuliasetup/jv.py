from __future__ import annotations
import typing

class JV:
    __slots__ = ['__jlslot__']
    __invoke__: typing.Callable[[JV, tuple, dict], typing.Any]

    def __call__(self, *args, **kwargs):
        return self.__invoke__(
            self, args, kwargs
        )

    # def __getattr__(self, attr: str):
    #     return self.__jl_getattr__(self, attr)

    # def __setattr__(self, _name: str, _value) -> None:
    #     return self.__jl_setattr__(self, _name, _value)

    # def __getitem__(self, item):
    #     return self.__jl_getitem__(self, item)

    # def __setitem__(self, item, value):
    #     return self.__jl_setitem__(self, item, value)

    # def __add__(self, other):
    #     return self.__jl_add__(self, other)

    # def __sub__(self, other):
    #     return self.__jl_sub__(self, other)

    # def __mul__(self, other):
    #     return self.__jl_mul__(self, other)

    # def __matmul__(self, other):
    #     return self.__jl_matmul__(self, other)

    # def __truediv__(self, other):
    #     return self.__jl_div__(self, other)

    # def __floordiv__(self, other):
    #     return self.__jl_floordiv__(self, other)

    # def __mod__(self, other):
    #     return self.__jl_mod__(self, other)

    # def __pow__(self, other):
    #     return self.__jl_pow__(self, other)

    # def __lshift__(self, other):
    #     return self.__jl_lshift__(self, other)

    # def __rshift__(self, other):
    #     return self.__jl_rshift__(self, other)

    # def __or__(self, other):
    #     return self.__jl_bitor__(self, other)

    # def __and__(self, other):
    #     return self.__jl_bitand__(self, other)

    # def __eq__(self, other):
    #     return self.__jl_eq__(self, other)

    # def __ne__(self, other):
    #     return self.__jl_ne__(self, other)

    # def __lt__(self, other):
    #     return self.__jl_lt__(self, other)

    # def __le__(self, other):
    #     return self.__jl_le__(self, other)

    # def __gt__(self, other):
    #     return self.__jl_gt__(self, other)

    # def __ge__(self, other):
    #     return self.__jl_ge__(self, other)

    # def __contains__(self, item):
    #     return self.__jl_contains__(self, item)

    # def __invert__(self):
    #     return self.__jl_invert__(self)

    # def __bool__(self):
    #     return self.__jl_bool__(self)

    # def __pos__(self):
    #     return self.__jl_pos__(self)

    # def __neg__(self):
    #     return self.__jl_neg__(self)

    # def __hash__(self):
    #     return self.__jl_hash__(self)

    # def __repr__(self):
    #     return self.__jl_repr__(self)
