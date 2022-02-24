//
// Created by switchblade on 2022-02-23.
//

#include "ubjf_read.h"

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>

void ubjf_init_read(ubjf_read_state *state,
                    const ubjf_read_event_info *read_info,
                    const ubjf_parse_event_info *parse_info,
                    ubjf_error *out_error)
{
	ubjf_error error = UBJF_NO_ERROR;
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		error = UBJF_MAKE_PARAM_ERROR(0);
	else if (UBJF_UNLIKELY(!read_info))
		error = UBJF_MAKE_PARAM_ERROR(1);
	else if (UBJF_UNLIKELY(!parse_info))
		error = UBJF_MAKE_PARAM_ERROR(2);
	else
#endif
	{
		state->read_events = *read_info;
		state->parse_events = *parse_info;
	}

	UBJF_SET_OPTIONAL(out_error, error);
}
static size_t file_read(void *dest, size_t n, FILE *file) { return fread(dest, 1, n, file); }
static size_t file_bump(size_t n, FILE *file) { return fseek(file, n, SEEK_CUR) ? 0 : n; }
static int file_peek(FILE *file) { return ungetc(getc(file), file); }

void ubjf_init_file_read(ubjf_read_state *state, FILE *file, const ubjf_parse_event_info *parse_info,
                         ubjf_error *out_error)
{
	ubjf_error error = UBJF_NO_ERROR;
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		error = UBJF_MAKE_PARAM_ERROR(0);
	else if (UBJF_UNLIKELY(!file))
		error = UBJF_MAKE_PARAM_ERROR(1);
	else if (UBJF_UNLIKELY(!parse_info))
		error = UBJF_MAKE_PARAM_ERROR(2);
	else
#endif
	{
		state->read_events.read = (ubjf_read_func) fread;
		state->read_events.bump = (ubjf_bump_func) file_bump;
		state->read_events.peek = (ubjf_peek_func) file_peek;
		state->parse_events = *parse_info;
	}

	UBJF_SET_OPTIONAL(out_error, error);
}
void ubjf_destroy_file_read(ubjf_read_state *) {}

struct buffer_read_data
{
	const void *buffer;
	size_t size;
};
static size_t buffer_read(void *dest, size_t n, struct buffer_read_data *udata)
{
	if (UBJF_UNLIKELY(n > udata->size))
		n = udata->size;
	memcpy(dest, udata->buffer, n);

	udata->buffer += n;
	udata->size -= n;
	return n;
}
static size_t buffer_bump(size_t n, struct buffer_read_data *udata)
{
	if (UBJF_UNLIKELY(n > udata->size))
		n = udata->size;
	udata->buffer += n;
	udata->size -= n;
	return n;
}
static int buffer_peek(struct buffer_read_data *udata)
{
	return udata->size ? *(const char *) udata->buffer : EOF;
}

void ubjf_init_buffer_read(ubjf_read_state *state, const void *buffer, size_t buffer_size,
                           const ubjf_parse_event_info *parse_info, ubjf_error *out_error)
{
	ubjf_error error = UBJF_NO_ERROR;
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		error = UBJF_MAKE_PARAM_ERROR(0);
	else if (UBJF_UNLIKELY(!buffer))
		error = UBJF_MAKE_PARAM_ERROR(1);
	else if (UBJF_UNLIKELY(!buffer_size))
		error = UBJF_MAKE_PARAM_ERROR(2);
	else if (UBJF_UNLIKELY(!parse_info))
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
			state->read_events.udata = buffer_data;
			state->read_events.read = (ubjf_read_func) buffer_read;
			state->read_events.bump = (ubjf_bump_func) buffer_bump;
			state->read_events.peek = (ubjf_peek_func) buffer_peek;
			state->parse_events = *parse_info;
		}
	}

	UBJF_SET_OPTIONAL(out_error, error);
}
void ubjf_destroy_buffer_read(ubjf_read_state *state)
{
	if (UBJF_LIKELY(state))
		UBJF_FREE(state->read_events.udata);
}

void ubjf_destroy_value(ubjf_value value)
{
	if (value.type == UBJF_STRING || value.type == UBJF_HIGHP)
		UBJF_FREE((void *) value.string);
}

static const int16_t token_table[0xff] = {
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
        if ((inner_result = state->func(__VA_ARGS__)) != UBJF_NO_ERROR)             \
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

static void read_bytes(ubjf_read_state *state, size_t *bytes, void *buffer, size_t n)
{
	size_t read;
	if (UBJF_UNLIKELY(!state->read_events.read ||
	                  (read = state->read_events.read(buffer, n, state->read_events.udata)) != n))
		THROW_ERROR(state, UBJF_EOF);
	*bytes += read;
}
static int16_t read_token(ubjf_read_state *state, size_t *bytes)
{
	char c;
	read_bytes(state, bytes, &c, 1);
	return token_table[c];
}
static void require_token(ubjf_read_state *state, size_t *bytes, int16_t token)
{
	if (UBJF_UNLIKELY(read_token(state, bytes) != token))
		THROW_ERROR(state, UBJF_ERROR_BAD_DATA);
}

static int16_t peek_token(ubjf_read_state *state)
{
	int c;
	if (UBJF_UNLIKELY(!state->read_events.peek || (c = state->read_events.peek(state->read_events.udata)) == EOF))
		THROW_ERROR(state, UBJF_EOF);
	return token_table[c & 0xff];
}
static void consume_bytes(ubjf_read_state *state, size_t *bytes, size_t n)
{
	size_t read;
	if (UBJF_UNLIKELY(!state->read_events.bump ||
	                  (read = state->read_events.bump(n, state->read_events.udata)) != n))
		THROW_ERROR(state, UBJF_EOF);
	*bytes += read;
}

static int64_t read_length(ubjf_read_state *state, size_t *bytes)
{
	switch (read_token(state, bytes))
	{
		case UBJF_INT8:
		{
			int8_t value = 0;
			read_bytes(state, bytes, &value, INT8_SIZE);
			return value;
		}
		case UBJF_UINT8:
		{
			uint8_t value = 0;
			read_bytes(state, bytes, &value, UINT8_SIZE);
			return value;
		}
		case UBJF_INT16:
		{
			int16_t value = 0;
			read_bytes(state, bytes, &value, INT16_SIZE);
			FIX_ENDIANNESS_16(value);
			return value;
		}
		case UBJF_INT32:
		{
			int32_t value = 0;
			read_bytes(state, bytes, &value, INT32_SIZE);
			FIX_ENDIANNESS_32(value);
			return value;
		}
		case UBJF_INT64:
		{
			int64_t value = 0;
			read_bytes(state, bytes, &value, INT64_SIZE);
			FIX_ENDIANNESS_64(value);
			return value;
		}
		default:
			THROW_ERROR(state, UBJF_ERROR_BAD_DATA);
	}
}

static void parse_string(ubjf_read_state *state, size_t *bytes, ubjf_value *value)
{
	int64_t length = read_length(state, bytes);
	char *str;
	if (state->parse_events.on_string_alloc)
		str = state->parse_events.on_string_alloc(length + 1, state->parse_events.udata);
	else
		str = UBJF_MALLOC(length + 1);

	if (UBJF_UNLIKELY(!str))
		THROW_ERROR(state, UBJF_ERROR_ALLOC);

	if (UBJF_LIKELY(length > 0))
		read_bytes(state, bytes, str, length);
	str[length] = '\0';
	value->string = str;
}

static void parse_integer(ubjf_read_state *state, size_t *bytes, ubjf_value *value)
{
	switch (value->type)
	{
		case UBJF_INT8:
			read_bytes(state, bytes, &value->int8, INT8_SIZE);
			break;
		case UBJF_UINT8:
			read_bytes(state, bytes, &value->uint8, UINT8_SIZE);
			break;
		case UBJF_INT16:
			read_bytes(state, bytes, &value->int16, INT16_SIZE);
			FIX_ENDIANNESS_16(value->int16);
			break;
		case UBJF_INT32:
			read_bytes(state, bytes, &value->int32, INT32_SIZE);
			FIX_ENDIANNESS_32(value->int32);
			break;
		case UBJF_INT64:
			read_bytes(state, bytes, &value->int64, INT64_SIZE);
			FIX_ENDIANNESS_64(value->int64);
			break;
		default:
			THROW_ERROR(state, UBJF_ERROR_BAD_DATA);
	}
}
static void parse_float(ubjf_read_state *state, size_t *bytes, ubjf_value *value)
{
	if (value->type == UBJF_FLOAT32)
	{
		read_bytes(state, bytes, &value->float32, FLOAT32_SIZE);
		FIX_ENDIANNESS_32(value->float32);
	} else if (value->type == UBJF_FLOAT64)
	{
		read_bytes(state, bytes, &value->float64, FLOAT64_SIZE);
		FIX_ENDIANNESS_64(value->float64);
	} else
		THROW_ERROR(state, UBJF_ERROR_BAD_DATA);
}
static void parse_value(ubjf_read_state *state, size_t *bytes, ubjf_type type)
{

	ubjf_value value = {.type = type};
	if (type & UBJF_BOOL_TYPE_MASK)
		value.boolean = type & 1;
	else if (type & UBJF_INTEGER_TYPE_MASK)
		parse_integer(state, bytes, &value);
	else if (type & UBJF_FLOAT_TYPE_MASK)
		parse_float(state, bytes, &value);
	else if (type == UBJF_HIGHP || type == UBJF_STRING)
		parse_string(state, bytes, &value);
	else
		THROW_ERROR(state, UBJF_ERROR_BAD_DATA);
	if (state->parse_events.on_value)
		CHECKED_INVOKE(state, parse_events.on_value, value, state->parse_events.udata);
}

struct container_context
{
	int64_t size;           /* If fixed size, will be non 0. */
	ubjf_type type;   /* If strongly typed, will be non 0. */
};

static void parse_node_recursive(ubjf_read_state *state, ubjf_error *error, size_t *bytes, size_t *nodes,
                                 ubjf_type type);

static void parse_array(ubjf_read_state *state, ubjf_error *error, size_t *bytes, size_t *nodes,
                        struct container_context *context)
{
	if (context->size) /* Fixed-size. */
		while (context->size--)
		{
			ubjf_type type = context->type;
			if (!type)
				type = read_token(state, bytes);
			parse_node_recursive(state, error, bytes, nodes, type);
		}
	else /* Fully dynamic. */
		for (;;)
		{
			int16_t token = peek_token(state);
			if (token == UBJF_ARRAY_END)
			{
				consume_bytes(state, bytes, 1);
				break;
			}

			parse_node_recursive(state, error, bytes, nodes, token);
		}
}

static void accept_node(size_t *nodes) { ++*nodes; }
static void parse_key_node(ubjf_read_state *state, size_t *bytes, size_t *nodes)
{
	ubjf_value value = {.type = UBJF_STRING};
	parse_string(state, bytes, &value);
	if (state->parse_events.on_value)
		CHECKED_INVOKE(state, parse_events.on_value, value, state->parse_events.udata);
	accept_node(nodes);
}
static void parse_object(ubjf_read_state *state, ubjf_error *error, size_t *bytes, size_t *nodes,
                         struct container_context *context)
{
	if (context->size) /* Fixed-size. */
		while (context->size--)
		{
			parse_key_node(state, bytes, nodes);
			ubjf_type type = context->type;
			if (!type)
				type = read_token(state, bytes);
			parse_node_recursive(state, error, bytes, nodes, type);
		}
	else /* Fully dynamic. */
		for (;;)
		{
			int16_t token = peek_token(state);
			if (token == UBJF_OBJECT_END)
			{
				consume_bytes(state, bytes, 1);
				break;
			}

			parse_key_node(state, bytes, nodes);
			parse_node_recursive(state, error, bytes, nodes, token);
		}
}
static void parse_container(ubjf_read_state *state, ubjf_error *error, size_t *bytes, size_t *nodes, ubjf_type type)
{
	if (type != UBJF_OBJECT && type != UBJF_ARRAY)
		THROW_ERROR(state, UBJF_ERROR_BAD_DATA);

	START_FRAME(state)
		struct container_context context = {};
		{
			int16_t token = peek_token(state);
			if (token == UBJF_STRONG_CONTAINER)
			{
				consume_bytes(state, bytes, 1);
				context.type = read_token(state, bytes);
				require_token(state, bytes, UBJF_CONTAINER_SIZE);
				context.size = read_length(state, bytes);
			} else if (token == UBJF_CONTAINER_SIZE)
			{
				consume_bytes(state, bytes, 1);
				context.size = read_length(state, bytes);
			}
		}

		if (state->parse_events.on_container_begin)
			CHECKED_INVOKE(state, parse_events.on_container_begin,
			               type, context.size, context.type,
			               state->parse_events.udata);

		if (type == UBJF_ARRAY)
			parse_array(state, error, bytes, nodes, &context);
		else
			parse_object(state, error, bytes, nodes, &context);

		if (state->parse_events.on_container_end)
			state->parse_events.on_container_end(state->parse_events.udata);
	} else
	{
		if (state->parse_events.on_container_error)
			state->parse_events.on_container_error(state->parse_events.udata);
	END_FRAME_RETHROW(state)
}

static void parse_node_recursive(ubjf_read_state *state, ubjf_error *error, size_t *bytes, size_t *nodes,
                                 ubjf_type type)
{
	if (type == UBJF_NOOP)
	{
		if (state->parse_events.on_noop)
			CHECKED_INVOKE(state, parse_events.on_noop, state->parse_events.udata);
	} else if (type & UBJF_VALUE_TYPE_MASK)
		parse_value(state, bytes, type);
	else if (type & UBJF_CONTAINER_TYPE_MASK)
		parse_container(state, error, bytes, nodes, type);
	else
		THROW_ERROR(state, UBJF_ERROR_BAD_DATA);
	accept_node(nodes);
}

static void read_node_impl(ubjf_read_state *state, ubjf_error *error, size_t *bytes, size_t *nodes)
{
	START_FRAME(state)
		parse_node_recursive(state, error, bytes, nodes, read_token(state, bytes));
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

	read_node_impl(state, &temp_error, &temp_bytes, &temp_nodes);

	finally:
	UBJF_SET_OPTIONAL(error, temp_error);
	UBJF_SET_OPTIONAL(bytes, temp_bytes);
	return temp_error == UBJF_EOF ? -1 : temp_nodes;
}