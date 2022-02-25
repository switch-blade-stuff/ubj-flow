//
// Created by switchblade on 2022-02-23.
//

#pragma once

#include <stdio.h>

#include "detail/define.h"
#include "detail/error.h"
#include "detail/value.h"

typedef size_t (*ubjf_read_func)(void *dest, size_t n, void *udata);
typedef int (*ubjf_peek_func)(void *udata);
typedef size_t (*ubjf_bump_func)(size_t n, void *udata);

typedef struct
{
	/** User data passed to read events. */
	void *udata;
	/** Function used to read input. */
	ubjf_read_func read;
	/** Function used to advance input without reading. */
	ubjf_bump_func bump;
	/** Function used to read input without advancing. */
	ubjf_peek_func peek;
} ubjf_read_event_info;

typedef ubjf_error (*ubjf_on_value_func)(ubjf_value value, void *udata);

/** Function called to request string allocation for string & high-precision number parse.
 * @note Size of the string (n) includes the null terminator. */
typedef char *(*ubjf_on_string_alloc_func)(size_t n, void *udata);

typedef ubjf_error (*ubjf_on_container_begin_func)(ubjf_type container_type, int64_t fixed_size,
                                                   ubjf_type value_type, void *udata);
typedef ubjf_error (*ubjf_on_container_end_func)(void *udata);

typedef struct
{
	/** User data passed to parse parse_info. */
	void *udata;
	/** Function called when a value is parsed. */
	ubjf_on_value_func on_value;
	/** Function called when a string buffer (for a string or a high-precision number value) is allocated.
	 * @note After a string is parsed successfully, `on_value` is called.
	 * @note Subsequent `on_value` will not be called if an error occurs during string parsing. */
	ubjf_on_string_alloc_func on_string_alloc;
	/** Function called when a container parse is started. */
	ubjf_on_container_begin_func on_container_begin;
	/** Function called when a container parse is finished.
	 * @note `on_container_end` will not be called if an error occurs during container parsing. */
	ubjf_on_container_end_func on_container_end;
} ubjf_parse_event_info;

typedef struct
{
	/** Events used for reading. */
	ubjf_read_event_info read_event_info;
	/** Events used for parsing. */
	ubjf_parse_event_info parse_event_info;
} ubjf_read_state;

/** Initializes an instance of `ubjf_read_state` for custom events.
 * @param[out] state State to initialize.
 * @param[in] read_info Event info used for reading.
 * @param[in] parse_info Event info for parsing.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
UBJF_EXTERN ubjf_error ubjf_init_read(ubjf_read_state *state, ubjf_read_event_info read_info,
                                      ubjf_parse_event_info parse_info);
/** Initializes an instance of `ubjf_read_state` for file reading.
 * @param[out] state State to initialize.
 * @param[in] file File to use for reading. Must be stored externally.
 * @param[in] parse_info Event info used for parsing.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code.*/
UBJF_EXTERN ubjf_error ubjf_init_file_read(ubjf_read_state *state, FILE *file, ubjf_parse_event_info parse_info);
/** De-initializes state previously initialized via `ubjf_init_file_read`.
 * @param[in] state State to destroy. */
UBJF_EXTERN void ubjf_destroy_file_read(ubjf_read_state *state);

/** Initializes an instance of `ubjf_read_state` for buffer reading.
 * @param[out] state State to initialize.
 * @param[in] buffer Buffer containing UBJSON data to use for parsing. Must be non-NULL.
 * @param[in] buffer_size Size of the data buffer in bytes. Must be non-0.
 * @param[in] parse_info Event info used for parsing.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
UBJF_EXTERN ubjf_error ubjf_init_buffer_read(ubjf_read_state *state, const void *buffer, size_t buffer_size,
                                             ubjf_parse_event_info parse_info);
/** De-initializes state previously initialized via `ubjf_init_buffer_read`.
 * @param[in] state State to destroy. */
UBJF_EXTERN void ubjf_destroy_buffer_read(ubjf_read_state *state);

/** Reads in next UBJSON node and invokes appropriate parse_info.
 * @param[in] state State to use for reading.
 * @param[out] nodes Pointer to the value set to the amount of nodes processed. Optional.
 * @return On success, returns `UBJF_NO_ERROR`. On error, returns the error code. */
UBJF_EXTERN ubjf_error ubjf_read_next(ubjf_read_state *state, size_t *nodes);
