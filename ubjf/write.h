//
// Created by switchblade on 2022-02-23.
//

#pragma once

#include <stdio.h>

#include "detail/define.h"
#include "detail/error.h"
#include "detail/value.h"
#include "detail/config.h"

typedef struct
{
	/** Type of the container. Must be either `UBJF_ARRAY` or `UBJF_OBJECT`. */
	ubjf_type container_type;
	/** Optional fixed length for container's value. To make container dynamically-size, set to `-1`. */
	int64_t length;
	/** Optional fixed type for container's value. To make container dynamically-typed, set to `UBJF_NO_TYPE`.
	 * @note If a fixed type is set, fixed length must be provided as well.
	 * @note For fixed-type containers, `UBJF_BOOL` will be ignored. Use `UBJF_TRUE_TYPE` and `UBJF_FALSE_TYPE` instead. */
	ubjf_type value_type;
} ubjf_container_info;

typedef size_t (*ubjf_write_func)(const void *src, size_t n, void *udata);

typedef struct
{
	/** User data passed to write event. */
	void *udata;
	/** Function used to write output. */
	ubjf_write_func write;
} ubjf_write_event_info;

typedef struct
{
	/** Events used for writing. */
	ubjf_write_event_info write_event_info;
	/** UBJson syntax to use for writing. */
	ubjf_syntax syntax;
} ubjf_write_state_info;

typedef struct
{
	/** Syntax selector. */
	ubjf_syntax syntax;

	/** Stack of started container writes. */
	ubjf_container_info *container_stack;
	/** Pointer to the top of the container stack. */
	ubjf_container_info *current_container;
	/** End of the stack. */
	ubjf_container_info *stack_end;

	/** Events used for writing. */
	ubjf_write_event_info write_event_info;
} ubjf_write_state;

/** Initializes an instance of `ubjf_read_state` for write events.
 * @param[out] state State to initialize.
 * @param[in] init_info Data used to initialize the state.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
UBJF_EXTERN ubjf_error ubjf_init_write(ubjf_write_state *state, ubjf_write_state_info init_info);
/** Destroys a write state initialized with `ubjf_init_write`.
 * @param[in] state State to destroy. */
UBJF_EXTERN void ubjf_destroy_write(ubjf_write_state *state);
/** Initializes an instance of `ubjf_read_state` for file writing.
 * @param[out] state State to initialize.
 * @param[in] init_info Data used to initialize the state.
 * @param[in] file File to write to.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
UBJF_EXTERN ubjf_error ubjf_init_file_write(ubjf_write_state *UBJF_RESTRICT state, ubjf_write_state_info init_info,
                                            FILE *UBJF_RESTRICT file);
/** Destroys a write state initialized with `ubjf_init_file_write`.
 * @param[in] state State to destroy. */
UBJF_EXTERN void ubjf_destroy_file_write(ubjf_write_state *state);
/** Initializes an instance of `ubjf_read_state` for buffer writing.
 * @param[out] state State to initialize.
 * @param[in] init_info Data used to initialize the state.
 * @param[in] buffer Buffer to write to. Must be non-NULL.
 * @param[in] buffer_size Size of the output buffer. Must be non-0.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
UBJF_EXTERN ubjf_error ubjf_init_buffer_write(ubjf_write_state *UBJF_RESTRICT state, ubjf_write_state_info init_info,
                                              void *UBJF_RESTRICT buffer, size_t buffer_size);
/** Destroys a write state initialized with `ubjf_init_buffer_write`.
 * @param[in] state State to destroy. */
UBJF_EXTERN void ubjf_destroy_buffer_write(ubjf_write_state *state);

/** Writes a value to the target state.
 * @param[in] state State to use for writing.
 * @param[in] value Value to write.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
UBJF_EXTERN ubjf_error ubjf_write_value(ubjf_write_state *state, ubjf_value value);

/** Writes a NOOP to the target state.
 * @param[in] state State to use for writing.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
static inline ubjf_error ubjf_write_noop(ubjf_write_state *state)
{
	ubjf_value value = {.type = UBJF_NOOP};
	return ubjf_write_value(state, value);
}
/** Writes a NULL to the target state.
 * @param[in] state State to use for writing.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
static inline ubjf_error ubjf_write_null(ubjf_write_state *state)
{
	ubjf_value value = {.type = UBJF_NULL};
	return ubjf_write_value(state, value);
}
/** Writes a boolean to the target state.
 * @param[in] state State to use for writing.
 * @param[in] b Value to write.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
static inline ubjf_error ubjf_write_bool(ubjf_write_state *state, bool b)
{
	ubjf_value value = {.type = UBJF_BOOL, .boolean = b};
	return ubjf_write_value(state, value);
}
/** Writes a character to the target state.
 * @param[in] state State to use for writing.
 * @param[in] c Value to write.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
static inline ubjf_error ubjf_write_char(ubjf_write_state *state, char c)
{
	ubjf_value value = {.type = UBJF_CHAR, .character = c};
	return ubjf_write_value(state, value);
}
/** Writes a int8 to the target state.
 * @param[in] state State to use for writing.
 * @param[in] i Value to write.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
static inline ubjf_error ubjf_write_int8(ubjf_write_state *state, int8_t i)
{
	ubjf_value value = {.type = UBJF_INT8, .integer = i};
	return ubjf_write_value(state, value);
}
/** Writes a uint8 to the target state.
 * @param[in] state State to use for writing.
 * @param[in] i Value to write.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
static inline ubjf_error ubjf_write_uint8(ubjf_write_state *state, uint8_t i)
{
	ubjf_value value = {.type = UBJF_UINT8, .integer = i};
	return ubjf_write_value(state, value);
}
/** Writes a int16 to the target state.
 * @param[in] state State to use for writing.
 * @param[in] i Value to write.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
static inline ubjf_error ubjf_write_int16(ubjf_write_state *state, int16_t i)
{
	ubjf_value value = {.type = UBJF_INT16, .integer = i};
	return ubjf_write_value(state, value);
}
/** Writes a int32 to the target state.
 * @param[in] state State to use for writing.
 * @param[in] i Value to write.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
static inline ubjf_error ubjf_write_int32(ubjf_write_state *state, int32_t i)
{
	ubjf_value value = {.type = UBJF_INT32, .integer = i};
	return ubjf_write_value(state, value);
}
/** Writes a int64 to the target state.
 * @param[in] state State to use for writing.
 * @param[in] i Value to write.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
static inline ubjf_error ubjf_write_int64(ubjf_write_state *state, int64_t i)
{
	ubjf_value value = {.type = UBJF_INT64, .integer = i};
	return ubjf_write_value(state, value);
}
/** Writes a float32 to the target state.
 * @param[in] state State to use for writing.
 * @param[in] i Value to write.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
static inline ubjf_error ubjf_write_float32(ubjf_write_state *state, float i)
{
	ubjf_value value = {.type = UBJF_INT32, .float32 = i};
	return ubjf_write_value(state, value);
}
/** Writes a float64 to the target state.
 * @param[in] state State to use for writing.
 * @param[in] i Value to write.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
static inline ubjf_error ubjf_write_float64(ubjf_write_state *state, double i)
{
	ubjf_value value = {.type = UBJF_INT64, .float64 = i};
	return ubjf_write_value(state, value);
}
/** Writes a high-precision number to the target state.
 * @param[in] state State to use for writing.
 * @param[in] h Value to write.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
static inline ubjf_error ubjf_write_highp(ubjf_write_state *state, ubjf_string h)
{
	ubjf_value value = {.type = UBJF_HIGHP, .highp = h};
	return ubjf_write_value(state, value);
}
/** Writes a string to the target state.
 * @param[in] state State to use for writing.
 * @param[in] str String to write.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
static inline ubjf_error ubjf_write_string(ubjf_write_state *state, ubjf_string str)
{
	ubjf_value value = {.type = UBJF_STRING, .string = str};
	return ubjf_write_value(state, value);
}

/** Starts a container write for the target state.
 * @param[in] state State to use for writing.
 * @param[in] info Container info specifying details for this container.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code.
 * @note If a container info is not valid, call to this function will fail with error being set to `UBJF_ERROR_PARAM` with index 1. */
UBJF_EXTERN ubjf_error ubjf_start_container(ubjf_write_state *state, ubjf_container_info info);
/** Writes a key for object container's key-value pair.
 * @param[in] state State to use for writing.
 * @param[in] key String containing the key.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
UBJF_EXTERN ubjf_error ubjf_write_object_key(ubjf_write_state *state, ubjf_string key);
/** Ends a previously started container write.
 * @param[in] state State to use for writing.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code.
 * @note If a container write was not started, call to this function will fail with error being set to `UBJF_ERROR_PARAM` with index 0. */
UBJF_EXTERN ubjf_error ubjf_end_container(ubjf_write_state *state);

/** Writes a dynamic-typed array to the target state.
 * @param[in] state State to use for writing.
 * @param[in] data Data of the array.
 * @param[in] n Size of the array.
 * @param[in] value_type Optional fixed type for the array.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code.
 * @note If a fixed type is specified, every value within the source array must be of the same type. */
UBJF_EXTERN ubjf_error ubjf_write_array(ubjf_write_state *UBJF_RESTRICT state, const ubjf_value *UBJF_RESTRICT data,
                                        int64_t n, ubjf_type value_type);
