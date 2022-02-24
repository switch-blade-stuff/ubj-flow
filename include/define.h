//
// Created by switchblade on 2022-02-23.
//

#pragma once

#include "ubjf_export.h"

#ifdef __cplusplus
#define UBJF_EXTERN extern "C" UBJF_API
#else
#define UBJF_EXTERN extern UBJF_API
#endif

#if !defined(UBJF_MALLOC) && !defined(UBJF_REALLOC) && !defined(UBJF_FREE)
#define UBJF_MALLOC malloc
#define UBJF_REALLOC realloc
#define UBJF_FREE free
#endif

#define UBJF_SET_OPTIONAL(out_ptr, value) do { if (out_ptr) *(out_ptr) = (value); } while (0)

#ifdef __GNUC__
#define UBJF_LIKELY(x)       __builtin_expect(!!(x),1)
#define UBJF_UNLIKELY(x)     __builtin_expect(!!(x),0)
#else
#define UBJF_LIKELY(x)
#define UBJF_UNLIKELY(x)
#endif
