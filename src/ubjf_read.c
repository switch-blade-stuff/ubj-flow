//
// Created by switchblade on 2022-02-23.
//

#include "ubjf_read.h"

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

void ubjf_init_file_read(ubjf_read_state *state, FILE *file, const ubjf_read_parse_callback_info *callback_info,
                         ubjf_error *out_error)
{
	ubjf_error error = UBJF_NO_ERROR;
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		error = UBJF_MAKE_PARAM_ERROR(0);
	else if (UBJF_UNLIKELY(!file))
		error = UBJF_MAKE_PARAM_ERROR(1);
	else if (UBJF_UNLIKELY(!callback_info))
		error = UBJF_MAKE_PARAM_ERROR(2);
	else
#endif
	{
		state->read = (ubjf_read_func) fread;
		state->callbacks = *callback_info;
	}

	UBJF_SET_OPTIONAL(out_error, error);
}
void ubjf_destroy_file_read(ubjf_read_state *state)
{
}

struct buffer_read_data
{
	const void *buffer;
	size_t size;
};
static size_t buffer_read(void *dest, size_t size, size_t n, struct buffer_read_data *udata)
{
	size_t bytes = size * n;
	if (UBJF_UNLIKELY(bytes > udata->size))
		bytes = udata->size;
	memcpy(dest, udata->buffer, bytes);

	udata->buffer += bytes;
	udata->size -= bytes;
	return bytes;
}

void ubjf_init_buffer_read(ubjf_read_state *state, const void *buffer, size_t buffer_size,
                           const ubjf_read_parse_callback_info *callback_info, ubjf_error *out_error)
{
	ubjf_error error = UBJF_NO_ERROR;
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		error = UBJF_MAKE_PARAM_ERROR(0);
	else if (UBJF_UNLIKELY(!buffer))
		error = UBJF_MAKE_PARAM_ERROR(1);
	else if (UBJF_UNLIKELY(!buffer_size))
		error = UBJF_MAKE_PARAM_ERROR(2);
	else if (UBJF_UNLIKELY(!callback_info))
		error = UBJF_MAKE_PARAM_ERROR(3);
	else
#endif
	{
		struct buffer_read_data *buffer_data = UBJF_MALLOC(sizeof(struct buffer_read_data));
		if (UBJF_UNLIKELY(!buffer_data))
			error = UBJF_ERROR_ALLOC;
		else
		{
			buffer_data->buffer = buffer;
			buffer_data->size = buffer_size;
			state->read_udata = buffer_data;
			state->read = (ubjf_read_func) buffer_read;
			state->callbacks = *callback_info;
		}
	}

	UBJF_SET_OPTIONAL(out_error, error);
}
void ubjf_destroy_buffer_read(ubjf_read_state *state)
{
	if (UBJF_LIKELY(state))
		UBJF_FREE(state->read_udata);
}

void ubjf_destroy_value(ubjf_value value)
{
	if (value.type == UBJF_STRING || value.type == UBJF_HIGHP)
		UBJF_FREE((void *) value.string);
}

static size_t do_read(ubjf_read_state *state, void *dest, size_t size, size_t n)
{
	if (UBJF_LIKELY(state->read))
		return state->read(dest, size, n, state->read_udata);
	else
		return 0;
}

static const int16_t token_table[255] = {
		/* Type tokens. */
		['N'] = UBJF_NOOP,
		['Z'] = UBJF_NULL,
		['T'] = UBJF_TRUE,
		['F'] = UBJF_FALSE,
		['i'] = UBJF_INT8,
		['U'] = UBJF_UINT8,
		['I'] = UBJF_INT16,
		['l'] = UBJF_INT32,
		['L'] = UBJF_INT64,
		['d'] = UBJF_FLOAT32,
		['D'] = UBJF_FLOAT64,
		['H'] = UBJF_HIGHP,
		['C'] = UBJF_CHAR,
		['S'] = UBJF_STRING,
		['['] = UBJF_ARRAY,
		['{'] = UBJF_OBJECT,

		/* Extra tokens. */
		[']'] = UBJF_ARRAY_END,
		['}'] = UBJF_OBJECT_END,
		['#'] = UBJF_CONTAINER_SIZE,
		['$'] = UBJF_STRONG_CONTAINER,
};

#define INT8_SIZE 1
#define UINT8_SIZE 1
#define INT16_SIZE 2
#define INT32_SIZE 4
#define INT64_SIZE 8
#define FLOAT32_SIZE 4
#define FLOAT64_SIZE 8
#define CHAR_SIZE 1

#ifndef UBJF_BIG_ENDIAN

#include "bswap.h"

#define FIX_ENDIANNESS_16(value) bswap_16(value)
#define FIX_ENDIANNESS_32(value) bswap_32(value)
#define FIX_ENDIANNESS_64(value) bswap_64(value)
#else
#define FIX_ENDIANNESS_16(...)
#define FIX_ENDIANNESS_32(...)
#define FIX_ENDIANNESS_64(...)
#endif

#define THROW_ERROR(state, error) longjmp(*(jmp_buf *) state->panic, error)

#define CHECKED_INVOKE(state, func, ...)                                            \
    do {                                                                            \
        ubjf_error inner_result;                                                    \
        if ((inner_result = state->callbacks.func(__VA_ARGS__)) != UBJF_NO_ERROR)   \
            THROW_ERROR(state, inner_result);                                       \
    } while (0)

#define START_FRAME(state)                      \
    jmp_buf panic_buf;                          \
    void *old_panic = state->panic;             \
    state->panic = &panic_buf;                  \
    ubjf_error inner_error = setjmp(panic_buf); \
    if (inner_error == UBJF_NO_ERROR) {

#define END_FRAME(state, error) \
    }                           \
    state->panic = old_panic;   \
    *error = inner_error;

#define END_FRAME_RETHROW(state)        \
    }                                   \
    state->panic = old_panic;           \
    THROW_ERROR(state, inner_error);


static void parse_string(const char **out_str, ubjf_read_state *state, size_t *bytes);
static void parse_value(ubjf_type type, ubjf_read_state *state, size_t *bytes);
static void parse_array(ubjf_read_state *state, ubjf_error *error, size_t *bytes, size_t *nodes);
static void parse_object(ubjf_read_state *state, ubjf_error *error, size_t *bytes, size_t *nodes);

static char peek_token(ubjf_read_state *state, size_t *bytes)
{
	char c = 0;
	if (UBJF_LIKELY(!do_read(state, &c, 1, 1)))
		THROW_ERROR(state, UBJF_EOF);
	*bytes += 1;
	return c;
}
static void read_bytes(void *buffer, size_t n, ubjf_read_state *state, size_t *bytes)
{
	size_t read;
	if (UBJF_UNLIKELY((read = do_read(state, buffer, 1, n)) != n))
		THROW_ERROR(state, UBJF_EOF);
	*bytes += read;
}

static ubjf_value read_value(ubjf_type type, ubjf_read_state *state, size_t *bytes)
{
	ubjf_value value = {.type = type};
	switch (type)
	{
		case UBJF_TRUE:
			value.boolean = true;
			break;
		case UBJF_FALSE:
			value.boolean = false;
			break;
		case UBJF_INT8:
			read_bytes(&value.int8, INT8_SIZE, state, bytes);
			break;
		case UBJF_UINT8:
			read_bytes(&value.uint8, UINT8_SIZE, state, bytes);
			break;
		case UBJF_INT16:
			read_bytes(&value.int16, INT16_SIZE, state, bytes);
			FIX_ENDIANNESS_16(value.int16);
			break;
		case UBJF_INT32:
			read_bytes(&value.int32, INT32_SIZE, state, bytes);
			FIX_ENDIANNESS_32(value.int32);
			break;
		case UBJF_INT64:
			read_bytes(&value.int64, INT64_SIZE, state, bytes);
			FIX_ENDIANNESS_64(value.int64);
			break;
		case UBJF_FLOAT32:
			read_bytes(&value.float32, FLOAT32_SIZE, state, bytes);
			FIX_ENDIANNESS_32(value.float32);
			break;
		case UBJF_FLOAT64:
			read_bytes(&value.float64, FLOAT64_SIZE, state, bytes);
			FIX_ENDIANNESS_64(value.float64);
			break;
		case UBJF_CHAR:
			read_bytes(&value.character, CHAR_SIZE, state, bytes);
			break;
		case UBJF_HIGHP:
		case UBJF_STRING:
			parse_string(&value.string, state, bytes);
		default:
			break;
	}
	return value;
}
static int64_t read_length(ubjf_read_state *state, size_t *bytes)
{
	ubjf_value value;
	char token = peek_token(state, bytes);
	ubjf_type type = token_table[token];
	switch (type)
	{
		case UBJF_INT8:
			read_bytes(&value.int8, INT8_SIZE, state, bytes);
			return value.int8;
		case UBJF_UINT8:
			read_bytes(&value.uint8, UINT8_SIZE, state, bytes);
			return value.uint8;
		case UBJF_INT16:
			read_bytes(&value.int16, INT16_SIZE, state, bytes);
			FIX_ENDIANNESS_16(value.int16);
			return value.int16;
		case UBJF_INT32:
			read_bytes(&value.int32, INT32_SIZE, state, bytes);
			FIX_ENDIANNESS_32(value.int32);
			return value.int32;
		case UBJF_INT64:
			read_bytes(&value.int64, INT64_SIZE, state, bytes);
			FIX_ENDIANNESS_64(value.int64);
			return value.int64;
		default:
			THROW_ERROR(state, UBJF_ERROR_BAD_DATA);
	}
}

static void parse_string(const char **out_str, ubjf_read_state *state, size_t *bytes)
{
	int64_t length = read_length(state, bytes);
	char *str;
	if (state->callbacks.on_string_alloc)
		str = state->callbacks.on_string_alloc(length + 1, state->callbacks.udata);
	else
		str = UBJF_MALLOC(length + 1);

	if (UBJF_UNLIKELY(!str))
		THROW_ERROR(state, UBJF_ERROR_ALLOC);

	if (UBJF_LIKELY(length > 0))
		read_bytes(str, length, state, bytes);
	str[length] = '\0';
	*out_str = str;
}
static void parse_value(ubjf_type type, ubjf_read_state *state, size_t *bytes)
{
	ubjf_value value = read_value(type, state, bytes);
	if (state->callbacks.on_value)
		CHECKED_INVOKE(state, on_value, value, state->callbacks.udata);
}
static void parse_array(ubjf_read_state *state, ubjf_error *error, size_t *bytes, size_t *nodes)
{
	/* TODO: Implement this */
}
static void parse_object(ubjf_read_state *state, ubjf_error *error, size_t *bytes, size_t *nodes)
{
	/* TODO: Implement this */
}
static void parse_container(ubjf_type type, ubjf_read_state *state, ubjf_error *error, size_t *bytes, size_t *nodes)
{
	START_FRAME(state)
		if (!state->callbacks.on_container_begin)
			CHECKED_INVOKE(state, on_container_begin, type, state->callbacks.udata);

		if (type == UBJF_ARRAY)
			parse_array(state, error, bytes, nodes);
		else if (type == UBJF_OBJECT)
			parse_object(state, error, bytes, nodes);

		if (state->callbacks.on_container_end)
			state->callbacks.on_container_end(state->callbacks.udata);
	} else
	{
		if (state->callbacks.on_container_error)
			state->callbacks.on_container_error(state->callbacks.udata);
	END_FRAME_RETHROW(state)
}

static void parse_node(ubjf_read_state *state, ubjf_error *error, size_t *bytes, size_t *nodes)
{
	START_FRAME(state)
		ubjf_type type = token_table[peek_token(state, bytes)];
		if (type == UBJF_NOOP)
		{
			if (state->callbacks.on_noop)
				CHECKED_INVOKE(state, on_noop, state->callbacks.udata);
		} else if (type & UBJF_VALUE_TYPE_MASK)
			parse_value(type, state, bytes);
		else if (type & UBJF_CONTAINER_TYPE_MASK)
			parse_container(type, state, error, bytes, nodes);
		else
			THROW_ERROR(state, UBJF_ERROR_BAD_DATA);

		++*nodes;
	END_FRAME(state, error)
}

size_t ubjf_read_next(ubjf_read_state *state, ubjf_error *error, size_t *bytes)
{
	ubjf_error temp_error = UBJF_NO_ERROR;
	size_t temp_nodes = 0;
	size_t temp_bytes = 0;

#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
	{
		temp_error = UBJF_MAKE_PARAM_ERROR(0);
		goto finally;
	}
#endif

	parse_node(state, &temp_error, &temp_bytes, &temp_nodes);

	finally:
	UBJF_SET_OPTIONAL(error, temp_error);
	UBJF_SET_OPTIONAL(bytes, temp_bytes);
	return temp_error == UBJF_EOF ? -1 : temp_nodes;
}