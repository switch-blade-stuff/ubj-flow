//
// Created by switchblade on 2022-02-23.
//

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "define.h"
#include "ubjf_error.h"
#include "type.h"

typedef size_t (*ubjf_read_func)(void *dest, size_t n, void *udata);
typedef int (*ubjf_peek_func)(void *udata);
typedef size_t (*ubjf_bump_func)(size_t n, void *udata);

typedef struct
{
	/** User data passed to read event. */
	void *udata;
	/** Function used to read input. */
	ubjf_read_func read;
	/** Function used to advance input without reading. */
	ubjf_bump_func bump;
	/** Function used to read input without advancing. */
	ubjf_peek_func peek;
} ubjf_read_event_info;

typedef struct
{
	ubjf_type type;
	union
	{
		bool boolean;
		char character;
		int8_t int8;
		uint8_t uint8;
		int16_t int16;
		int32_t int32;
		int64_t int64;
		float float32;
		double float64;
		const char *highp;
		const char *string;
	};
} ubjf_value;

typedef ubjf_error (*ubjf_on_noop_func)(void *udata);
typedef ubjf_error (*ubjf_on_value_func)(ubjf_value value, void *udata);
typedef char *(*ubjf_on_string_alloc_func)(size_t length, void *udata);

typedef ubjf_error (*ubjf_on_container_begin_func)(ubjf_type container_type,
                                                   int64_t fixed_size,
                                                   ubjf_type value_type,
                                                   void *udata);
typedef void (*ubjf_on_container_end_func)(void *udata);

typedef struct
{
	/** User data passed to parse parse_event_info. */
	void *udata;
	/** Function called when a noop is parsed. */
	ubjf_on_noop_func on_noop;
	/** Function called when a value is parsed. */
	ubjf_on_value_func on_value;
	/** Function called when a string is allocated. */
	ubjf_on_string_alloc_func on_string_alloc;
	/** Function called when a container parse is started. */
	ubjf_on_container_begin_func on_container_begin;
	/** Function called when a container parse is finished. */
	ubjf_on_container_end_func on_container_end;
	/** Function called when a container parse cannot be completed due to an error. */
	ubjf_on_container_end_func on_container_error;
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
 * @param[in] read_info Event info used for reading. Will be copied into the state.
 * @param[in] parse_info Event info for parsing. Will be copied into the state.
 * @param[out] error Pointer to the value set in case of an error. Optional. */
UBJF_EXTERN void ubjf_init_read(ubjf_read_state *state, const ubjf_read_event_info *read_info,
                                const ubjf_parse_event_info *parse_info, ubjf_error *error);
/** Initializes an instance of `ubjf_read_state` for file reading.
 * @param[out] state State to initialize.
 * @param[in] file File to use for reading. Must be stored externally.
 * @param[in] parse_info Event info used for parsing. Will be copied into the state.
 * @param[out] error Pointer to the value set in case of an error. Optional. */
UBJF_EXTERN void ubjf_init_file_read(ubjf_read_state *state, FILE *file, const ubjf_parse_event_info *parse_info,
                                     ubjf_error *error);
/** De-initializes state previously initialized via `ubjf_init_file_read`.
 * @param[in] state State to destroy. */
UBJF_EXTERN void ubjf_destroy_file_read(ubjf_read_state *state);

/** Initializes an instance of `ubjf_read_state` for buffer reading.
 * @param[out] state State to initialize.
 * @param[in] buffer Buffer to UBJSON data to use for parsing.
 * @param[in] buffer_size Size of the data buffer in bytes.
 * @param[in] parse_info Event info used for parsing. Will be copied into the state.
 * @param[out] error Pointer to the value set in case of an error. Optional. */
UBJF_EXTERN void ubjf_init_buffer_read(ubjf_read_state *state, const void *buffer, size_t buffer_size,
                                       const ubjf_parse_event_info *parse_info, ubjf_error *error);
/** De-initializes state previously initialized via `ubjf_init_buffer_read`.
 * @param[in] state State to destroy. */
UBJF_EXTERN void ubjf_destroy_buffer_read(ubjf_read_state *state);

/** Reads in next UBJSON node and invokes appropriate parse_event_info.
 * @param[in] state State to use for reading.
 * @param[out] error Pointer to the value set in case of an error. Optional.
 * @param[out] bytes Pointer to the value set to the amount of bytes read. Optional.
 * @param[out] nodes Pointer to the value set to the amount of nodes processed. Optional.
 * @return On success, returns 0. On EOF, returns `-1` and sets error to `UBJF_ERROR_EOF`.
 * In case of any other error, returns 1. */
UBJF_EXTERN int ubjf_read_next(ubjf_read_state *state, ubjf_error *error, size_t *bytes, size_t *nodes);
