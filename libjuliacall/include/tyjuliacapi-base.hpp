// This file is generated. Do not modify it directly.
#pragma GCC diagnostic ignored "-Wconversion-null"
#include "tyjuliacapi-types.hpp"


void (*_fptr_JLFreeFromMe)(JV ref);
void JLFreeFromMe(JV ref) {
    return _fptr_JLFreeFromMe(ref);
}

ErrorCode (*_fptr_JLEval)(/* out */JV* out, JV module, SList<uint8_t> code);
ErrorCode JLEval(/* out */JV* out, JV module, SList<uint8_t> code) {
    return _fptr_JLEval(out, module, code);
}

ErrorCode JLEval(/* out */JV* out, JV module, const char* code) {
    char* code_copy = const_cast<char*>(code);
    return _fptr_JLEval(out, module, SList_adapt(reinterpret_cast<uint8_t*>(code_copy), strlen(code_copy)));
}

ErrorCode (*_fptr_FetchJLErrorSize)(/* out */int64_t* size);
ErrorCode FetchJLErrorSize(/* out */int64_t* size) {
    return _fptr_FetchJLErrorSize(size);
}

ErrorCode (*_fptr_FetchJLError)(/* out */JSym* out, SList<uint8_t> msgBuffer);
ErrorCode FetchJLError(/* out */JSym* out, SList<uint8_t> msgBuffer) {
    return _fptr_FetchJLError(out, msgBuffer);
}

JV (*_fptr_JSymToJV)(JSym sym);
JV JSymToJV(JSym sym) {
    return _fptr_JSymToJV(sym);
}

JV (*_fptr_JLTypeOf)(JV value);
JV JLTypeOf(JV value) {
    return _fptr_JLTypeOf(value);
}

int64_t (*_fptr_JLTypeOfAsTypeSlot)(JV value);
int64_t JLTypeOfAsTypeSlot(JV value) {
    return _fptr_JLTypeOfAsTypeSlot(value);
}

bool8_t (*_fptr_JLIsInstanceWithTypeSlot)(JV value, int64_t slot);
bool8_t JLIsInstanceWithTypeSlot(JV value, int64_t slot) {
    return _fptr_JLIsInstanceWithTypeSlot(value, slot);
}

ErrorCode (*_fptr_JLCall)(/* out */JV* out, JV func, SList<JV> args, SList<STuple<JSym,JV>> kwargs);
ErrorCode JLCall(/* out */JV* out, JV func, SList<JV> args, SList<STuple<JSym,JV>> kwargs) {
    return _fptr_JLCall(out, func, args, kwargs);
}

ErrorCode (*_fptr_JLDotCall)(/* out */JV* out, JV func, SList<JV> args, SList<STuple<JSym,JV>> kwargs);
ErrorCode JLDotCall(/* out */JV* out, JV func, SList<JV> args, SList<STuple<JSym,JV>> kwargs) {
    return _fptr_JLDotCall(out, func, args, kwargs);
}

ErrorCode (*_fptr_JLCompare)(/* out */bool8_t* out, Compare cmp, JV a, JV b);
ErrorCode JLCompare(/* out */bool8_t* out, Compare cmp, JV a, JV b) {
    return _fptr_JLCompare(out, cmp, a, b);
}

ErrorCode (*_fptr_JLGetProperty)(/* out */JV* out, JV self, JSym property);
ErrorCode JLGetProperty(/* out */JV* out, JV self, JSym property) {
    return _fptr_JLGetProperty(out, self, property);
}

ErrorCode (*_fptr_JLSetProperty)(JV self, JSym property, JV value);
ErrorCode JLSetProperty(JV self, JSym property, JV value) {
    return _fptr_JLSetProperty(self, property, value);
}

ErrorCode (*_fptr_JLHasProperty)(/* out */bool8_t* out, JV self, JSym property);
ErrorCode JLHasProperty(/* out */bool8_t* out, JV self, JSym property) {
    return _fptr_JLHasProperty(out, self, property);
}

ErrorCode (*_fptr_JLGetIndex)(/* out */JV* out, JV self, SList<JV> index);
ErrorCode JLGetIndex(/* out */JV* out, JV self, SList<JV> index) {
    return _fptr_JLGetIndex(out, self, index);
}

ErrorCode (*_fptr_JLGetIndexI)(/* out */JV* out, JV self, int64_t index);
ErrorCode JLGetIndexI(/* out */JV* out, JV self, int64_t index) {
    return _fptr_JLGetIndexI(out, self, index);
}

ErrorCode (*_fptr_JLSetIndex)(JV self, SList<JV> index, JV value);
ErrorCode JLSetIndex(JV self, SList<JV> index, JV value) {
    return _fptr_JLSetIndex(self, index, value);
}

ErrorCode (*_fptr_JLSetIndexI)(JV self, int64_t index, JV value);
ErrorCode JLSetIndexI(JV self, int64_t index, JV value) {
    return _fptr_JLSetIndexI(self, index, value);
}

ErrorCode (*_fptr_JLGetSymbol)(/* out */JSym* out, JV value, bool8_t doCast);
ErrorCode JLGetSymbol(/* out */JSym* out, JV value, bool8_t doCast) {
    return _fptr_JLGetSymbol(out, value, doCast);
}

ErrorCode (*_fptr_JLGetBool)(/* out */bool8_t* out, JV value, bool8_t doCast);
ErrorCode JLGetBool(/* out */bool8_t* out, JV value, bool8_t doCast) {
    return _fptr_JLGetBool(out, value, doCast);
}

ErrorCode (*_fptr_JLGetUInt8)(/* out */uint8_t* out, JV value, bool8_t doCast);
ErrorCode JLGetUInt8(/* out */uint8_t* out, JV value, bool8_t doCast) {
    return _fptr_JLGetUInt8(out, value, doCast);
}

ErrorCode (*_fptr_JLGetUInt32)(/* out */uint32_t* out, JV value, bool8_t doCast);
ErrorCode JLGetUInt32(/* out */uint32_t* out, JV value, bool8_t doCast) {
    return _fptr_JLGetUInt32(out, value, doCast);
}

ErrorCode (*_fptr_JLGetUInt64)(/* out */uint64_t* out, JV value, bool8_t doCast);
ErrorCode JLGetUInt64(/* out */uint64_t* out, JV value, bool8_t doCast) {
    return _fptr_JLGetUInt64(out, value, doCast);
}

ErrorCode (*_fptr_JLGetInt32)(/* out */int32_t* out, JV value, bool8_t doCast);
ErrorCode JLGetInt32(/* out */int32_t* out, JV value, bool8_t doCast) {
    return _fptr_JLGetInt32(out, value, doCast);
}

ErrorCode (*_fptr_JLGetInt64)(/* out */int64_t* out, JV value, bool8_t doCast);
ErrorCode JLGetInt64(/* out */int64_t* out, JV value, bool8_t doCast) {
    return _fptr_JLGetInt64(out, value, doCast);
}

ErrorCode (*_fptr_JLGetSingle)(/* out */float* out, JV value, bool8_t doCast);
ErrorCode JLGetSingle(/* out */float* out, JV value, bool8_t doCast) {
    return _fptr_JLGetSingle(out, value, doCast);
}

ErrorCode (*_fptr_JLGetDouble)(/* out */double* out, JV value, bool8_t doCast);
ErrorCode JLGetDouble(/* out */double* out, JV value, bool8_t doCast) {
    return _fptr_JLGetDouble(out, value, doCast);
}

ErrorCode (*_fptr_JLGetComplexF64)(/* out */complex_t* out, JV value, bool8_t doCast);
ErrorCode JLGetComplexF64(/* out */complex_t* out, JV value, bool8_t doCast) {
    return _fptr_JLGetComplexF64(out, value, doCast);
}

ErrorCode (*_fptr_JLGetUTF8String)(SList<uint8_t> out, JV value);
ErrorCode JLGetUTF8String(SList<uint8_t> out, JV value) {
    return _fptr_JLGetUTF8String(out, value);
}

ErrorCode (*_fptr_JLGetArrayPointer)(/* out */uint8_t** out, /* out */int64_t* len, JV value);
ErrorCode JLGetArrayPointer(/* out */uint8_t** out, /* out */int64_t* len, JV value) {
    return _fptr_JLGetArrayPointer(out, len, value);
}

ErrorCode (*_fptr_JSymFromString)(/* out */JSym* out, SList<uint8_t> value);
ErrorCode JSymFromString(/* out */JSym* out, SList<uint8_t> value) {
    return _fptr_JSymFromString(out, value);
}

ErrorCode JSymFromString(/* out */JSym* out, const char *value) {
    char* value_copy = const_cast<char*>(value);
    return _fptr_JSymFromString(out, SList_adapt(reinterpret_cast<uint8_t*>(value_copy), strlen(value_copy)));
}

ErrorCode (*_fptr_ToJLInt64)(/* out */JV* out, int64_t value);
ErrorCode ToJLInt64(/* out */JV* out, int64_t value) {
    return _fptr_ToJLInt64(out, value);
}

ErrorCode (*_fptr_ToJLUInt64)(/* out */JV* out, uint64_t value);
ErrorCode ToJLUInt64(/* out */JV* out, uint64_t value) {
    return _fptr_ToJLUInt64(out, value);
}

ErrorCode (*_fptr_ToJLUInt32)(/* out */JV* out, uint32_t value);
ErrorCode ToJLUInt32(/* out */JV* out, uint32_t value) {
    return _fptr_ToJLUInt32(out, value);
}

ErrorCode (*_fptr_ToJLUInt8)(/* out */JV* out, uint8_t value);
ErrorCode ToJLUInt8(/* out */JV* out, uint8_t value) {
    return _fptr_ToJLUInt8(out, value);
}

ErrorCode (*_fptr_ToJLString)(/* out */JV* out, SList<uint8_t> value);
ErrorCode ToJLString(/* out */JV* out, SList<uint8_t> value) {
    return _fptr_ToJLString(out, value);
}

ErrorCode (*_fptr_ToJLBool)(/* out */JV* out, bool8_t value);
ErrorCode ToJLBool(/* out */JV* out, bool8_t value) {
    return _fptr_ToJLBool(out, value);
}

ErrorCode (*_fptr_ToJLFloat64)(/* out */JV* out, double value);
ErrorCode ToJLFloat64(/* out */JV* out, double value) {
    return _fptr_ToJLFloat64(out, value);
}

ErrorCode (*_fptr_ToJLComplexF64)(/* out */JV* out, complex_t value);
ErrorCode ToJLComplexF64(/* out */JV* out, complex_t value) {
    return _fptr_ToJLComplexF64(out, value);
}

ErrorCode (*_fptr_JLStrVecWriteEltWithUTF8)(JV self, int64_t i, SList<uint8_t> value);
ErrorCode JLStrVecWriteEltWithUTF8(JV self, int64_t i, SList<uint8_t> value) {
    return _fptr_JLStrVecWriteEltWithUTF8(self, i, value);
}

ErrorCode (*_fptr_JLStrVecGetEltNBytes)(/* out */int64_t* out, JV self, int64_t i);
ErrorCode JLStrVecGetEltNBytes(/* out */int64_t* out, JV self, int64_t i) {
    return _fptr_JLStrVecGetEltNBytes(out, self, i);
}

ErrorCode (*_fptr_JLStrVecReadEltWithUTF8)(JV self, int64_t i, SList<uint8_t> value);
ErrorCode JLStrVecReadEltWithUTF8(JV self, int64_t i, SList<uint8_t> value) {
    return _fptr_JLStrVecReadEltWithUTF8(self, i, value);
}

ErrorCode (*_fptr_JLTypeToIdent)(/* out */int64_t* out, JV jv);
ErrorCode JLTypeToIdent(/* out */int64_t* out, JV jv) {
    return _fptr_JLTypeToIdent(out, jv);
}

ErrorCode (*_fptr_JLTypeFromIdent)(/* out */JV* out, int64_t slot);
ErrorCode JLTypeFromIdent(/* out */JV* out, int64_t slot) {
    return _fptr_JLTypeFromIdent(out, slot);
}

ErrorCode (*_fptr_JLNew_F64Array)(/* out */JV* out, SList<int64_t> dims);
ErrorCode JLNew_F64Array(/* out */JV* out, SList<int64_t> dims) {
    return _fptr_JLNew_F64Array(out, dims);
}

ErrorCode (*_fptr_JLNew_U64Array)(/* out */JV* out, SList<int64_t> dims);
ErrorCode JLNew_U64Array(/* out */JV* out, SList<int64_t> dims) {
    return _fptr_JLNew_U64Array(out, dims);
}

ErrorCode (*_fptr_JLNew_U32Array)(/* out */JV* out, SList<int64_t> dims);
ErrorCode JLNew_U32Array(/* out */JV* out, SList<int64_t> dims) {
    return _fptr_JLNew_U32Array(out, dims);
}

ErrorCode (*_fptr_JLNew_U8Array)(/* out */JV* out, SList<int64_t> dims);
ErrorCode JLNew_U8Array(/* out */JV* out, SList<int64_t> dims) {
    return _fptr_JLNew_U8Array(out, dims);
}

ErrorCode (*_fptr_JLNew_I64Array)(/* out */JV* out, SList<int64_t> dims);
ErrorCode JLNew_I64Array(/* out */JV* out, SList<int64_t> dims) {
    return _fptr_JLNew_I64Array(out, dims);
}

ErrorCode (*_fptr_JLNew_BoolArray)(/* out */JV* out, SList<int64_t> dims);
ErrorCode JLNew_BoolArray(/* out */JV* out, SList<int64_t> dims) {
    return _fptr_JLNew_BoolArray(out, dims);
}

ErrorCode (*_fptr_JLNew_ComplexF64Array)(/* out */JV* out, SList<int64_t> dims);
ErrorCode JLNew_ComplexF64Array(/* out */JV* out, SList<int64_t> dims) {
    return _fptr_JLNew_ComplexF64Array(out, dims);
}

ErrorCode (*_fptr_JLNew_StringVector)(/* out */JV* out, int64_t length);
ErrorCode JLNew_StringVector(/* out */JV* out, int64_t length) {
    return _fptr_JLNew_StringVector(out, length);
}

ErrorCode (*_fptr_JLArray_Size)(/* out */int64_t* out, JV self, int64_t i);
ErrorCode JLArray_Size(/* out */int64_t* out, JV self, int64_t i) {
    return _fptr_JLArray_Size(out, self, i);
}

ErrorCode (*_fptr_JLArray_Rank)(/* out */int64_t* out, JV self);
ErrorCode JLArray_Rank(/* out */int64_t* out, JV self) {
    return _fptr_JLArray_Rank(out, self);
}

void (*_fptr_JLError_EnableBackTraceMsg)(bool8_t status);
void JLError_EnableBackTraceMsg(bool8_t status) {
    return _fptr_JLError_EnableBackTraceMsg(status);
}

uint8_t (*_fptr_JLError_HasBackTraceMsg)();
uint8_t JLError_HasBackTraceMsg() {
    return _fptr_JLError_HasBackTraceMsg();
}

ErrorCode (*_fptr_JLError_FetchMsgSize)(/* out */int64_t* size);
ErrorCode JLError_FetchMsgSize(/* out */int64_t* size) {
    return _fptr_JLError_FetchMsgSize(size);
}

ErrorCode (*_fptr_JLError_FetchMsgStr)(/* out */JSym* out, SList<uint8_t> msgBuffer);
ErrorCode JLError_FetchMsgStr(/* out */JSym* out, SList<uint8_t> msgBuffer) {
    return _fptr_JLError_FetchMsgStr(out, msgBuffer);
}

typedef void (*_get_capi_t)(const char* name, void** funcref,  bool8_t* status_ref);
DLLEXPORT bool8_t library_init(_get_capi_t get_capi) {
    bool8_t status;
    get_capi("JLFreeFromMe", (void**)&_fptr_JLFreeFromMe, &status);
    if (!status) {
        printf("Failed to load JLFreeFromMe\n");
        return false;
    }
    get_capi("JLEval", (void**)&_fptr_JLEval, &status);
    if (!status) {
        printf("Failed to load JLEval\n");
        return false;
    }
    get_capi("FetchJLErrorSize", (void**)&_fptr_FetchJLErrorSize, &status);
    if (!status) {
        printf("Failed to load FetchJLErrorSize\n");
        return false;
    }
    get_capi("FetchJLError", (void**)&_fptr_FetchJLError, &status);
    if (!status) {
        printf("Failed to load FetchJLError\n");
        return false;
    }
    get_capi("JSymToJV", (void**)&_fptr_JSymToJV, &status);
    if (!status) {
        printf("Failed to load JSymToJV\n");
        return false;
    }
    get_capi("JLTypeOf", (void**)&_fptr_JLTypeOf, &status);
    if (!status) {
        printf("Failed to load JLTypeOf\n");
        return false;
    }
    get_capi("JLTypeOfAsTypeSlot", (void**)&_fptr_JLTypeOfAsTypeSlot, &status);
    if (!status) {
        printf("Failed to load JLTypeOfAsTypeSlot\n");
        return false;
    }
    get_capi("JLIsInstanceWithTypeSlot", (void**)&_fptr_JLIsInstanceWithTypeSlot, &status);
    if (!status) {
        printf("Failed to load JLIsInstanceWithTypeSlot\n");
        return false;
    }
    get_capi("JLCall", (void**)&_fptr_JLCall, &status);
    if (!status) {
        printf("Failed to load JLCall\n");
        return false;
    }
    get_capi("JLDotCall", (void**)&_fptr_JLDotCall, &status);
    if (!status) {
        printf("Failed to load JLDotCall\n");
        return false;
    }
    get_capi("JLCompare", (void**)&_fptr_JLCompare, &status);
    if (!status) {
        printf("Failed to load JLCompare\n");
        return false;
    }
    get_capi("JLGetProperty", (void**)&_fptr_JLGetProperty, &status);
    if (!status) {
        printf("Failed to load JLGetProperty\n");
        return false;
    }
    get_capi("JLSetProperty", (void**)&_fptr_JLSetProperty, &status);
    if (!status) {
        printf("Failed to load JLSetProperty\n");
        return false;
    }
    get_capi("JLHasProperty", (void**)&_fptr_JLHasProperty, &status);
    if (!status) {
        printf("Failed to load JLHasProperty\n");
        return false;
    }
    get_capi("JLGetIndex", (void**)&_fptr_JLGetIndex, &status);
    if (!status) {
        printf("Failed to load JLGetIndex\n");
        return false;
    }
    get_capi("JLGetIndexI", (void**)&_fptr_JLGetIndexI, &status);
    if (!status) {
        printf("Failed to load JLGetIndexI\n");
        return false;
    }
    get_capi("JLSetIndex", (void**)&_fptr_JLSetIndex, &status);
    if (!status) {
        printf("Failed to load JLSetIndex\n");
        return false;
    }
    get_capi("JLSetIndexI", (void**)&_fptr_JLSetIndexI, &status);
    if (!status) {
        printf("Failed to load JLSetIndexI\n");
        return false;
    }
    get_capi("JLGetSymbol", (void**)&_fptr_JLGetSymbol, &status);
    if (!status) {
        printf("Failed to load JLGetSymbol\n");
        return false;
    }
    get_capi("JLGetBool", (void**)&_fptr_JLGetBool, &status);
    if (!status) {
        printf("Failed to load JLGetBool\n");
        return false;
    }
    get_capi("JLGetUInt8", (void**)&_fptr_JLGetUInt8, &status);
    if (!status) {
        printf("Failed to load JLGetUInt8\n");
        return false;
    }
    get_capi("JLGetUInt32", (void**)&_fptr_JLGetUInt32, &status);
    if (!status) {
        printf("Failed to load JLGetUInt32\n");
        return false;
    }
    get_capi("JLGetUInt64", (void**)&_fptr_JLGetUInt64, &status);
    if (!status) {
        printf("Failed to load JLGetUInt64\n");
        return false;
    }
    get_capi("JLGetInt32", (void**)&_fptr_JLGetInt32, &status);
    if (!status) {
        printf("Failed to load JLGetInt32\n");
        return false;
    }
    get_capi("JLGetInt64", (void**)&_fptr_JLGetInt64, &status);
    if (!status) {
        printf("Failed to load JLGetInt64\n");
        return false;
    }
    get_capi("JLGetSingle", (void**)&_fptr_JLGetSingle, &status);
    if (!status) {
        printf("Failed to load JLGetSingle\n");
        return false;
    }
    get_capi("JLGetDouble", (void**)&_fptr_JLGetDouble, &status);
    if (!status) {
        printf("Failed to load JLGetDouble\n");
        return false;
    }
    get_capi("JLGetComplexF64", (void**)&_fptr_JLGetComplexF64, &status);
    if (!status) {
        printf("Failed to load JLGetComplexF64\n");
        return false;
    }
    get_capi("JLGetUTF8String", (void**)&_fptr_JLGetUTF8String, &status);
    if (!status) {
        printf("Failed to load JLGetUTF8String\n");
        return false;
    }
    get_capi("JLGetArrayPointer", (void**)&_fptr_JLGetArrayPointer, &status);
    if (!status) {
        printf("Failed to load JLGetArrayPointer\n");
        return false;
    }
    get_capi("JSymFromString", (void**)&_fptr_JSymFromString, &status);
    if (!status) {
        printf("Failed to load JSymFromString\n");
        return false;
    }
    get_capi("ToJLInt64", (void**)&_fptr_ToJLInt64, &status);
    if (!status) {
        printf("Failed to load ToJLInt64\n");
        return false;
    }
    get_capi("ToJLUInt64", (void**)&_fptr_ToJLUInt64, &status);
    if (!status) {
        printf("Failed to load ToJLUInt64\n");
        return false;
    }
    get_capi("ToJLUInt32", (void**)&_fptr_ToJLUInt32, &status);
    if (!status) {
        printf("Failed to load ToJLUInt32\n");
        return false;
    }
    get_capi("ToJLUInt8", (void**)&_fptr_ToJLUInt8, &status);
    if (!status) {
        printf("Failed to load ToJLUInt8\n");
        return false;
    }
    get_capi("ToJLString", (void**)&_fptr_ToJLString, &status);
    if (!status) {
        printf("Failed to load ToJLString\n");
        return false;
    }
    get_capi("ToJLBool", (void**)&_fptr_ToJLBool, &status);
    if (!status) {
        printf("Failed to load ToJLBool\n");
        return false;
    }
    get_capi("ToJLFloat64", (void**)&_fptr_ToJLFloat64, &status);
    if (!status) {
        printf("Failed to load ToJLFloat64\n");
        return false;
    }
    get_capi("ToJLComplexF64", (void**)&_fptr_ToJLComplexF64, &status);
    if (!status) {
        printf("Failed to load ToJLComplexF64\n");
        return false;
    }
    get_capi("JLStrVecWriteEltWithUTF8", (void**)&_fptr_JLStrVecWriteEltWithUTF8, &status);
    if (!status) {
        printf("Failed to load JLStrVecWriteEltWithUTF8\n");
        return false;
    }
    get_capi("JLStrVecGetEltNBytes", (void**)&_fptr_JLStrVecGetEltNBytes, &status);
    if (!status) {
        printf("Failed to load JLStrVecGetEltNBytes\n");
        return false;
    }
    get_capi("JLStrVecReadEltWithUTF8", (void**)&_fptr_JLStrVecReadEltWithUTF8, &status);
    if (!status) {
        printf("Failed to load JLStrVecReadEltWithUTF8\n");
        return false;
    }
    get_capi("JLTypeToIdent", (void**)&_fptr_JLTypeToIdent, &status);
    if (!status) {
        printf("Failed to load JLTypeToIdent\n");
        return false;
    }
    get_capi("JLTypeFromIdent", (void**)&_fptr_JLTypeFromIdent, &status);
    if (!status) {
        printf("Failed to load JLTypeFromIdent\n");
        return false;
    }
    get_capi("JLNew_F64Array", (void**)&_fptr_JLNew_F64Array, &status);
    if (!status) {
        printf("Failed to load JLNew_F64Array\n");
        return false;
    }
    get_capi("JLNew_U64Array", (void**)&_fptr_JLNew_U64Array, &status);
    if (!status) {
        printf("Failed to load JLNew_U64Array\n");
        return false;
    }
    get_capi("JLNew_U32Array", (void**)&_fptr_JLNew_U32Array, &status);
    if (!status) {
        printf("Failed to load JLNew_U32Array\n");
        return false;
    }
    get_capi("JLNew_U8Array", (void**)&_fptr_JLNew_U8Array, &status);
    if (!status) {
        printf("Failed to load JLNew_U8Array\n");
        return false;
    }
    get_capi("JLNew_I64Array", (void**)&_fptr_JLNew_I64Array, &status);
    if (!status) {
        printf("Failed to load JLNew_I64Array\n");
        return false;
    }
    get_capi("JLNew_BoolArray", (void**)&_fptr_JLNew_BoolArray, &status);
    if (!status) {
        printf("Failed to load JLNew_BoolArray\n");
        return false;
    }
    get_capi("JLNew_ComplexF64Array", (void**)&_fptr_JLNew_ComplexF64Array, &status);
    if (!status) {
        printf("Failed to load JLNew_ComplexF64Array\n");
        return false;
    }
    get_capi("JLNew_StringVector", (void**)&_fptr_JLNew_StringVector, &status);
    if (!status) {
        printf("Failed to load JLNew_StringVector\n");
        return false;
    }
    get_capi("JLArray_Size", (void**)&_fptr_JLArray_Size, &status);
    if (!status) {
        printf("Failed to load JLArray_Size\n");
        return false;
    }
    get_capi("JLArray_Rank", (void**)&_fptr_JLArray_Rank, &status);
    if (!status) {
        printf("Failed to load JLArray_Rank\n");
        return false;
    }
    get_capi("JLError_EnableBackTraceMsg", (void**)&_fptr_JLError_EnableBackTraceMsg, &status);
    if (!status) {
        printf("Failed to load JLError_EnableBackTraceMsg\n");
        return false;
    }
    get_capi("JLError_HasBackTraceMsg", (void**)&_fptr_JLError_HasBackTraceMsg, &status);
    if (!status) {
        printf("Failed to load JLError_HasBackTraceMsg\n");
        return false;
    }
    get_capi("JLError_FetchMsgSize", (void**)&_fptr_JLError_FetchMsgSize, &status);
    if (!status) {
        printf("Failed to load JLError_FetchMsgSize\n");
        return false;
    }
    get_capi("JLError_FetchMsgStr", (void**)&_fptr_JLError_FetchMsgStr, &status);
    if (!status) {
        printf("Failed to load JLError_FetchMsgStr\n");
        return false;
    }
    return true;
}
