function pyjl_mul(self, other)
    return self .* other
end

function pyjl_hash(self)
    return hash(self) % Int64
end

function pyjl_contains(self, other)
    return (other in self)
end

function jl_evaluate(s::String)
    Base.eval(Main, Base.Meta.parseall(s))
end

function pyjl_bool(self)
    if self isa Number
        return py_cast(Py, o != 0)
    end
    if (self isa AbstractArray || self isa AbstractDict ||
        self isa AbstractSet || self isa AbstractString)
        return py_cast(Py, !isempty(o))
    end
    # return `true` is the default semantics of a Python object
    return py_cast(Py, true)
end

function pyjl_display(self)::String
    old_stdout = stdout
    rd, wr = redirect_stdout()
    try
        show(wr, "text/plain", self)
    finally
        try
            close(wr)
        catch
        end
        redirect_stdout(old_stdout)
    end
    read(rd, String)
end

function pyjl_frist_iter(self)
    return iterate(self)
end

# unary operator
for (pyfname, op) in [
    (:_pyjl_pos, :+),
    (:_pyjl_neg, :-),
    (:_pyjl_len, :length),
    (:_pyjl_invert, :~),
    (:_pyjl_abs, :abs),
    (:_pyjl_hash, :pyjl_hash),
    (:_pyjl_bool, :pyjl_bool),
    (:_pyjl_repr_pretty, :pyjl_display),
    (:_pyjl_first_iter, :iterate),
]
    @eval begin
        function $pyfname(self_::C.Ptr{PyObject}, ::C.Ptr{PyObject})
            if PyJuliaValue_IsNull(self_)
                py_seterror!(G_PyBuiltin.TypeError, "Julia object is NULL")
                return Py_NULLPTR
            end
            self = PyJuliaValue_GetValue(self_)
            try
                ans = $op(self)
                out = unsafe_unwrap(auto_box(ans))
                PyAPI.Py_IncRef(out)
                return out
            catch e
                handle_except(e)
            end
        end
    end
end

# binary operator
for (pyfname, op) in [
    (:_pyjl_add, :+),
    (:_pyjl_sub, :-),
    (:_pyjl_matmul, :*),
    (:_pyjl_mul, :pyjl_mul),
    (:_pyjl_truediv, :/),
    (:_pyjl_floordiv, :÷),
    (:_pyjl_mod, :%),
    (:_pyjl_lshift, :<<),
    (:_pyjl_rshift, :>>),
    (:_pyjl_and, :&),
    (:_pyjl_xor, :⊻),
    (:_pyjl_or, :|),
    (:_pyjl_eq, :(==)),
    (:_pyjl_ne,:(!=)),
    (:_pyjl_le, :≤),
    (:_pyjl_lt, :<),
    (:_pyjl_ge, :≥),
    (:_pyjl_gt, :>),
    (:_pyjl_pow, :^),
    (:_pyjl_contains, :pyjl_contains),
    (:_pyjl_next_iter, :iterate),
]
    @eval begin
        function $pyfname(self_::C.Ptr{PyObject}, other_::C.Ptr{PyObject})
            if PyJuliaValue_IsNull(self_)
                py_seterror!(G_PyBuiltin.TypeError, "Julia object is NULL")
                return Py_NULLPTR
            end
            self = PyJuliaValue_GetValue(self_)
            other = auto_unbox(Py(NewReference(), other_))
            try
                ans = $op(self, other)
                out = unsafe_unwrap(auto_box(ans))
                PyAPI.Py_IncRef(out)
                return out
            catch e
                handle_except(e)
            end
        end
    end
end
