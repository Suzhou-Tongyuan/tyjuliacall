from __future__ import annotations
import typing

__jl_invoke__: typing.Callable[[JV, tuple, dict], typing.Any]
__jl_getattr__: typing.Callable[[JV, str], typing.Any]
__jl_setattr__: typing.Callable[[JV, str, typing.Any], typing.Any]
__jl_getitem__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_setitem__: typing.Callable[[JV, typing.Any, typing.Any], typing.Any]
__jl_add__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_sub__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_mul__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_matmul__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_truediv__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_floordiv__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_mod__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_pow__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_lshift__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_rshift__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_bitor__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_bitxor__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_bitand__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_eq__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_ne__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_lt__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_le__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_gt__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_ge__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_contains__: typing.Callable[[JV, typing.Any], typing.Any]
__jl_invert__: typing.Callable[[JV], typing.Any]
__jl_bool__: typing.Callable[[JV], bool]
__jl_pos__: typing.Callable[[JV], typing.Any]
__jl_neg__: typing.Callable[[JV], typing.Any]
__jl_abs__: typing.Callable[[JV], typing.Any]
__jl_hash__: typing.Callable[[JV], typing.Any]
__jl_repr__: typing.Callable[[JV], str]
_jl_repr_pretty_: typing.Callable[[JV], str]


class JV:
    __slots__ = ["__jlslot__"]

    def __call__(self, *args, **kwargs):
        return __jl_invoke__(self, args, kwargs)

    def __getattr__(self, name: str):
        return __jl_getattr__(self, name)

    def __setattr__(self, name: str, value: typing.Any):
        return __jl_setattr__(self, name, value)

    def __getitem__(self, key: typing.Any):
        return __jl_getitem__(self, key)

    def __setitem__(self, key: typing.Any, value: typing.Any):
        return __jl_setitem__(self, key, value)

    def __add__(self, other: typing.Any):
        return __jl_add__(self, other)

    def __sub__(self, other: typing.Any):
        return __jl_sub__(self, other)

    def __mul__(self, other: typing.Any):
        return __jl_mul__(self, other)

    def __matmul__(self, other: typing.Any):
        return __jl_matmul__(self, other)

    def __truediv__(self, other: typing.Any):
        return __jl_truediv__(self, other)

    def __floordiv__(self, other: typing.Any):
        return __jl_floordiv__(self, other)

    def __mod__(self, other: typing.Any):
        return __jl_mod__(self, other)

    def __pow__(self, other: typing.Any):
        return __jl_pow__(self, other)

    def __lshift__(self, other: typing.Any):
        return __jl_lshift__(self, other)

    def __rshift__(self, other: typing.Any):
        return __jl_rshift__(self, other)

    def __or__(self, other: typing.Any):
        return __jl_bitor__(self, other)

    def __xor__(self, other: typing.Any):
        return __jl_bitxor__(self, other)

    def __and__(self, other: typing.Any):
        return __jl_bitand__(self, other)

    def __eq__(self, other: typing.Any):
        return __jl_eq__(self, other)

    def __ne__(self, other: typing.Any):
        return __jl_ne__(self, other)

    def __lt__(self, other: typing.Any):
        return __jl_lt__(self, other)

    def __le__(self, other: typing.Any):
        return __jl_le__(self, other)

    def __gt__(self, other: typing.Any):
        return __jl_gt__(self, other)

    def __ge__(self, other: typing.Any):
        return __jl_ge__(self, other)

    def __contains__(self, other: typing.Any):
        return __jl_contains__(self, other)

    def __invert__(self):
        return __jl_invert__(self)

    def __bool__(self):
        return __jl_bool__(self)

    def __pos__(self):
        return __jl_pos__(self)

    def __neg__(self):
        return __jl_neg__(self)

    def __abs__(self):
        return __jl_abs__(self)

    def __hash__(self):
        return __jl_hash__(self)

    def __repr__(self):
        return __jl_repr__(self)

    def _repr_pretty_(self, p, cycle):
        p.text(_jl_repr_pretty_(self) if not cycle else "...")

    def __iter__(self):
        global _jl_iterate
        try:
            jl_iterate = _jl_iterate  # type: ignore
        except NameError:
            from _tyjuliacall_jnumpy import Base  # type: ignore

            jl_iterate = _jl_iterate = Base.iterate

        pair = jl_iterate(self)
        while pair is not None:
            element, state = pair
            yield element
            pair = jl_iterate(self, state)
