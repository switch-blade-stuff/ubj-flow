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
	/** Failed to read from input. */
	UBJF_EOF,
	/** Failed to write to output. */
	UBJF_ERROR_BAD_WRITE,
	/** Invalid input, failed to parse. */
	UBJF_ERROR_BAD_DATA,
	/** Unknown error. */
	UBJF_ERROR_UNKNOWN,
	/** Passed parameter is invalid. */
	UBJF_ERROR_PARAM = 1 << 8, /* Index of the bad parameter stored as bottom 8 bits. */
} ubjf_error;

#define UBJF_CHECK_ERROR(error, value) (error) & (value)

#define UBJF_PARAM_ERROR_MASK 0xff
#define UBJF_MAKE_PARAM_ERROR(index) (((index) & UBJF_PARAM_ERROR_MASK) | UBJF_ERROR_PARAM)
#define UBJF_PARAM_ERROR_GET_INDEX(value) ((value) & UBJF_PARAM_ERROR_MASK)

/** Creates a formatted error message string.
 * @param[in] err Error to create message for.
 * @return Pointer to the allocated message string.
 * @note String will need to be de-allocated via `ubjf_free_error_message`. */
UBJF_EXTERN const char *ubjf_make_error_msg(ubjf_error err);
/** De-allocates error message obtained from `ubjf_make_error_msg`.
 * @param[in] msg Pointer to message string. */
UBJF_EXTERN void ubjf_free_error_message(const char *msg);
