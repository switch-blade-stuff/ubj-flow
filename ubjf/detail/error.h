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
	/** Unknown error. */
	UBJF_ERROR_UNKNOWN,

	/** Passed parameter is invalid. Use `UBJF_IS_PARAM_ERROR` to check if an error is parameter error. */
	UBJF_ERROR_PARAM = 1 << 8, /* Index of the bad parameter stored as bottom 8 bits. */

	/** Failed to read from input. */
	UBJF_EOF = -1,
} ubjf_error;

#define UBJF_CHECK_ERROR(error, value) (error) & (value)

#define UBJF_PARAM_ERROR_MASK 0xff
#define UBJF_MAKE_PARAM_ERROR(index) (((index) & UBJF_PARAM_ERROR_MASK) | UBJF_ERROR_PARAM)
#define UBJF_PARAM_ERROR_GET_INDEX(value) ((value) & UBJF_PARAM_ERROR_MASK)
#define UBJF_IS_PARAM_ERROR(error) (((error) & (~(int) 0xff)) == UBJF_ERROR_PARAM)

/** Creates a formatted error message string.
 * @param[in] err Error to create message for.
 * @param[out] str String to write the message to.
 * @param[in] len Length of the output string.
 * @return Amount of characters written to the output string (excluding the null terminator).
 * If `str` is NULL or `len` is 0, returns the required length of the formatted string. */
UBJF_EXTERN size_t ubjf_make_error_msg(ubjf_error err, char *str, size_t len);
