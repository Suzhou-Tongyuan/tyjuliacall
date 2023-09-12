#include <stdint.h>
#include <stdio.h>

#ifndef DLLEXPORT
#ifdef MSVC
#define DLLEXPORT extern "C" __declspec(dllexport)
#else
#define DLLEXPORT extern "C"
#endif
#endif

enum struct ErrorCode : uint8_t
{
    ok = 0,
    error = 1,
};

enum struct Compare : uint8_t
{
    SEQ = 0, // === 运算符
    SNE = 1, // !==
    // 以下是常见的6种逻辑运算符
    EQ = 2,
    NE = 3,
    LT = 4,
    LE = 5,
    GT = 6,
    GE = 7
};

typedef uint8_t bool8_t;

struct complex_t
{
    double re;
    double im;
};

typedef int64_t JV;
typedef int64_t JSym;

template <typename T>
struct SList
{
    int64_t len;
    T *data;
};

template <typename T>
SList<T> SList_adapt(T *data, int64_t len)
{
    SList<T> res;
    res.len = len;
    res.data = data;
    return res;
}

template <typename L, typename R>
struct STuple
{
    L l;
    R r;
};

template <typename L, typename R>
STuple<L, R> STuple_adapt(L l, R r)
{
    STuple<L, R> res;
    res.l = l;
    res.r = r;
    return res;
}

SList<STuple<JSym, JV>> emptyKwArgs()
{
    // return only value
    SList<STuple<JSym, JV>> res;
    res.len = 0;
    res.data = nullptr;
    return res;
}
