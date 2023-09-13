#ifndef TYJULIACAPI_H
#define TYJULIACAPI_H

#include "tyjuliacapi-base.hpp"

#define USE_STACKTRACE_MSG_LOCALLY _use_stacktrace_msg_t _localvar ## __LINE__ ## __FILE__ ## __FUNCTION__;

/**
 * use RAII to enable stacktrace message and assure
 * it will be restored when the scope ends
*/

class _use_stacktrace_msg_t
{
    bool8_t _is_enabled;
public:
    _use_stacktrace_msg_t()
    {
        _is_enabled = JLError_HasBackTraceMsg();
        JLError_EnableBackTraceMsg(true);
    }
    ~_use_stacktrace_msg_t()
    {
        JLError_EnableBackTraceMsg(_is_enabled);
    }
};

#endif