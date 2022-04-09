//
// Created by switchblade on 2022-02-23.
//

#pragma once

#include "define.h"

typedef enum
{
    UBJF_NO_ERROR = 0,
    /** Failed to allocate memory. */
    UBJF_ERROR_ALLOC,
    /** Failed to write to output. */
    UBJF_ERROR_BAD_WRITE,
    /** Invalid input, failed to parse. */
    UBJF_ERROR_BAD_DATA,
    /** Invalid type for this operation. */
    UBJF_ERROR_BAD_TYPE,
    /** High precision number support is not enabled. */
    UBJF_ERROR_HIGHP,

    /** Passed parameter is invalid. Index of the bad parameter is stored as bottom 16 bits via `UBJF_MAKE_PARAM_ERROR`
     * and retrieved via `UBJF_PARAM_ERROR_GET_INDEX`. Use `UBJF_IS_PARAM_ERROR` to check if an error is parameter error. */
    UBJF_ERROR_PARAM = 1 << 16,

    /** User event error. Optional data can be stored as bottom 16 bits via `UBJF_MAKE_EVENT_ERROR`
     * and retrieved via `UBJF_EVENT_ERROR_DET_DATA`. Use `UBJF_IS_EVENT_ERROR` to check if an error is event error. */
    UBJF_ERROR_EVENT = 1 << 17,

    /** Failed to read from input. */
    UBJF_EOF = -1,
} ubjf_error;

#define UBJF_PARAM_ERROR_MASK ((int) 0xffff)
#define UBJF_MAKE_PARAM_ERROR(index) (((index) & UBJF_PARAM_ERROR_MASK) | UBJF_ERROR_PARAM)
#define UBJF_IS_PARAM_ERROR(error) ((error) & UBJF_ERROR_PARAM)
#define UBJF_PARAM_ERROR_GET_INDEX(value) ((value) & UBJF_PARAM_ERROR_MASK)

#define UBJF_EVENT_ERROR_MASK ((int) 0xffff)
#define UBJF_MAKE_EVENT_ERROR(index) (((index) & UBJF_EVENT_ERROR_MASK) | UBJF_ERROR_EVENT)
#define UBJF_IS_EVENT_ERROR(index) ((error) & UBJF_ERROR_EVENT)
#define UBJF_EVENT_ERROR_DET_DATA(index) ((index) & (UBJF_EVENT_ERROR_MASK))
