//
// Created by switchblade on 2022-03-03.
//

#ifndef UBJF_NO_SPEC_12

#include "../parser.h"

static const ubjf_type ubjf_s12_token_type_map[UBJF_TOKEN_MAX] = {
		[UBJF_TOKEN_NULL]         = UBJF_NULL,
		[UBJF_TOKEN_NOOP]         = UBJF_NOOP,
		[UBJF_TOKEN_FALSE]        = UBJF_FALSE_TYPE,
		[UBJF_TOKEN_TRUE]         = UBJF_TRUE_TYPE,
		[UBJF_TOKEN_INT8]         = UBJF_INT8,
		[UBJF_TOKEN_UINT8]        = UBJF_UINT8,
		[UBJF_TOKEN_INT16]        = UBJF_INT16,
		[UBJF_TOKEN_INT32]        = UBJF_INT32,
		[UBJF_TOKEN_INT64]        = UBJF_INT64,
		[UBJF_TOKEN_FLOAT32]      = UBJF_FLOAT32,
		[UBJF_TOKEN_FLOAT64]      = UBJF_FLOAT64,
		[UBJF_TOKEN_HIGHP]        = UBJF_HIGHP,
		[UBJF_TOKEN_CHAR]         = UBJF_CHAR,
		[UBJF_TOKEN_STRING]       = UBJF_STRING,
		[UBJF_TOKEN_ARRAY_START]  = UBJF_ARRAY,
		[UBJF_TOKEN_OBJECT_START] = UBJF_OBJECT,
};

static ubjf_token ubjf_s12_read_token(ubjf_parse_ctx *ctx)
{
	ubjf_token token = 0;
	ubjf_guarded_read(ctx, &token, 1);
	return token;
}
static ubjf_token ubjf_s12_peek_token(ubjf_parse_ctx *ctx) { return (ubjf_token) ubjf_guarded_peek(ctx); }


static void ubjf_s12_parse_node(ubjf_parse_ctx *ctx, ubjf_type type);
static void ubjf_s12_read_node_recursive(ubjf_parse_ctx *ctx);

static void ubjf_s12_parse_integer(ubjf_parse_ctx *UBJF_RESTRICT ctx, ubjf_value *UBJF_RESTRICT value)
{
	switch (value->type)
	{
		case UBJF_INT8: /* Covered by uint8_t since both are the same length. */
		case UBJF_UINT8:
		{
			uint8_t temp;
			ubjf_guarded_read(ctx, &temp, sizeof(temp));
			value->integer = temp;
			break;
		}
		case UBJF_INT16:
		{
			int16_t temp;
			ubjf_guarded_read(ctx, &temp, sizeof(temp));
			value->integer = FIX_ENDIANNESS_16_BE(temp);
			break;
		}
		case UBJF_INT32:
		{
			int32_t temp;
			ubjf_guarded_read(ctx, &temp, sizeof(temp));
			value->integer = FIX_ENDIANNESS_32_BE(temp);
			break;
		}
		case UBJF_INT64:
		{
			int64_t temp;
			ubjf_guarded_read(ctx, &temp, sizeof(temp));
			value->integer = FIX_ENDIANNESS_64_BE(temp);
			break;
		}
	}
}
static void ubjf_s12_parse_float(ubjf_parse_ctx *UBJF_RESTRICT ctx, ubjf_value *UBJF_RESTRICT value)
{
	switch (value->type)
	{
		case UBJF_FLOAT32:
		{
			float temp;
			ubjf_guarded_read(ctx, &temp, sizeof(temp));
			value->floating = FIX_ENDIANNESS_32_BE(temp);
			break;
		}
		case UBJF_FLOAT64:
		{
			double temp;
			ubjf_guarded_read(ctx, &temp, sizeof(temp));
			value->floating = FIX_ENDIANNESS_64_BE(temp);
			break;
		}
	}
}
static int64_t ubjf_s12_parse_length(ubjf_parse_ctx *ctx)
{
	ubjf_value length = {.type = ubjf_s12_token_type_map[ubjf_s12_read_token(ctx)]};
	if (UBJF_UNLIKELY(!(length.type & UBJF_INTEGER_TYPE_MASK)))
		THROW_ERROR(ctx->panic_buf, UBJF_ERROR_BAD_DATA);

	/* Read the length and make sure it is within i64 range.
	 * Since all integer values are stored in a union,
	 * using int64 will cover all possible integer sizes. */
	ubjf_s12_parse_integer(ctx, &length);
	if (length.integer < 0)
		THROW_ERROR(ctx->panic_buf, UBJF_ERROR_BAD_DATA);

	return length.integer;
}
static bool ubjf_s12_parse_string(ubjf_parse_ctx *UBJF_RESTRICT ctx, ubjf_value *UBJF_RESTRICT value)
{
	int64_t length = ubjf_s12_parse_length(ctx);
	char *str = ubjf_invoke_string_alloc(ctx, length + 1); /* Add 1 for null terminator. */

	/* String alloc may return NULL. In that case, ignore the string. */
	if (UBJF_UNLIKELY(!str))
	{
		ubjf_guarded_bump(ctx, length);
		return false;
	}

	/* Guard for string read. */
	GUARD_ERROR(ctx->panic_buf, ctx->error)
	{
		ubjf_guarded_read(ctx, str, length);
		END_GUARD(ctx->panic_buf);
	} else RETHROW_ERROR(ctx->panic_buf, ctx->error);

	/* If reached here, we have read a string now. */
	str[length] = '\0';
	value->string.data = str;
	value->string.size = length;
	return true;
}
static void ubjf_s12_skip_highp(ubjf_parse_ctx *ctx)
{
	int64_t length = ubjf_s12_parse_length(ctx);
	ubjf_guarded_bump(ctx, length);
}
static void ubjf_s12_parse_value(ubjf_parse_ctx *ctx, ubjf_type type)
{
	ubjf_value value = {.type = type};
	if ((type & UBJF_BOOL_TYPE_MASK) == UBJF_BOOL_TYPE_MASK)
	{
		value.boolean = type & 1;   /* Extract the value flag. */
		value.type = UBJF_BOOL;     /* Change to bool type. */
	} else if (type & UBJF_INTEGER_TYPE_MASK)
		ubjf_s12_parse_integer(ctx, &value);
	else if (type & UBJF_FLOAT_TYPE_MASK)
		ubjf_s12_parse_float(ctx, &value);
	else if (type == UBJF_TOKEN_STRING)
	{
		parse_string:
		if (UBJF_UNLIKELY(!ubjf_s12_parse_string(ctx, &value)))
			return;
	} else if (type == UBJF_TOKEN_HIGHP)
	{
		if (ctx->highp_mode == UBJF_HIGHP_SKIP)
		{
			ubjf_s12_skip_highp(ctx);
			return;
		} else if (ctx->highp_mode == UBJF_HIGHP_AS_STRING)
			goto parse_string;
		else
			THROW_ERROR(ctx->panic_buf, UBJF_ERROR_HIGHP);
	}

	ubjf_invoke_on_value(ctx, value);
}

static void ubjf_s12_parse_sized_container_next(ubjf_parse_ctx *ctx, ubjf_type value_type)
{
	/* If the object is weakly-typed, do a full read, otherwise parse with type. */
	if (value_type != UBJF_NO_TYPE)
		ubjf_s12_parse_node(ctx, value_type);
	else
		ubjf_s12_read_node_recursive(ctx);
}
static void ubjf_s12_parse_array(ubjf_parse_ctx *ctx, int64_t length, ubjf_type value_type)
{
	if (length >= 0)
		while (length-- > 0)
		{
			ubjf_s12_parse_sized_container_next(ctx, value_type);
		}
	else
		for (;;)
		{
			ubjf_token token = ubjf_s12_read_token(ctx);
			if (token == UBJF_TOKEN_ARRAY_END)
				break;

			/* Expect that it is a value token. */
			ubjf_type type = ubjf_s12_token_type_map[token];
			if (UBJF_UNLIKELY(type == UBJF_NO_TYPE))
				THROW_ERROR(ctx->panic_buf, UBJF_ERROR_BAD_DATA);

			ubjf_s12_parse_node(ctx, type);
		}
}
static void ubjf_s12_parse_object(ubjf_parse_ctx *ctx, int64_t length, ubjf_type value_type)
{
	if (length >= 0)
		while (length-- > 0)
		{
			ubjf_s12_parse_value(ctx, UBJF_STRING);
			ubjf_s12_parse_sized_container_next(ctx, value_type);
		}
	else
		for (;;)
		{
			if (ubjf_guarded_peek(ctx) == UBJF_TOKEN_OBJECT_END)
			{
				ubjf_guarded_bump(ctx, 1);
				break;
			}

			/* Parse the string key and read the value node. */
			ubjf_s12_parse_value(ctx, UBJF_STRING);
			ubjf_s12_read_node_recursive(ctx);
		}
}
static void ubjf_s12_parse_container_preface(ubjf_parse_ctx *UBJF_RESTRICT ctx,
                                             int64_t *UBJF_RESTRICT out_length,
                                             ubjf_type *UBJF_RESTRICT out_value_type)
{
	switch (ubjf_s12_peek_token(ctx))
	{
		case UBJF_TOKEN_CONTAINER_TYPE:
		{
			/* If explicit type is specified, read the type, then read the length. */
			ubjf_guarded_bump(ctx, 1);

			*out_value_type = ubjf_s12_token_type_map[ubjf_s12_read_token(ctx)];
			if (UBJF_UNLIKELY(*out_value_type == UBJF_NO_TYPE))
				THROW_ERROR(ctx->panic_buf, UBJF_ERROR_BAD_DATA);

			/* For strongly typed containers, length always follows the type. */
			if (UBJF_UNLIKELY(ubjf_s12_peek_token(ctx) != UBJF_TOKEN_CONTAINER_LENGTH))
				THROW_ERROR(ctx->panic_buf, UBJF_ERROR_BAD_DATA);
		}
		case UBJF_TOKEN_CONTAINER_LENGTH:
		{
			ubjf_guarded_bump(ctx, 1);
			*out_length = ubjf_s12_parse_length(ctx);
		}
		default:
			break;
	}
}
static void ubjf_s12_parse_container(ubjf_parse_ctx *ctx, ubjf_type type)
{
	int64_t length = -1; /* -1 means no fixed size. */
	ubjf_type value_type = UBJF_NO_TYPE;
	ubjf_s12_parse_container_preface(ctx, &length, &value_type);

	ubjf_invoke_on_container_begin(ctx, type, length, value_type);

	/* Guard for container parse. */
	GUARD_ERROR(ctx->panic_buf, ctx->error)
	{
		if (type == UBJF_ARRAY)
			ubjf_s12_parse_array(ctx, length, value_type);
		else
			ubjf_s12_parse_object(ctx, length, value_type);
	} else RETHROW_ERROR(ctx->panic_buf, ctx->error);
	END_GUARD(ctx->panic_buf);
	ubjf_invoke_on_container_end(ctx);
}

static void ubjf_s12_parse_node(ubjf_parse_ctx *ctx, ubjf_type type)
{
	if (type & UBJF_VALUE_TYPE_MASK)
		ubjf_s12_parse_value(ctx, type);
	else if (type & UBJF_CONTAINER_TYPE_MASK)
		ubjf_s12_parse_container(ctx, type);

	ctx->total_nodes++;
}
static void ubjf_s12_read_node_recursive(ubjf_parse_ctx *ctx)
{
	ubjf_type type = ubjf_s12_token_type_map[ubjf_s12_read_token(ctx)];
	if (UBJF_UNLIKELY(type == UBJF_NO_TYPE))
		THROW_ERROR(ctx->panic_buf, UBJF_ERROR_BAD_DATA);

	ubjf_s12_parse_node(ctx, type);
}

ubjf_error ubjf_s12_read_next(ubjf_read_state *UBJF_RESTRICT state, size_t *UBJF_RESTRICT nodes)
{
#ifdef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		return UBJF_MAKE_PARAM_ERROR(0);
#endif

	ubjf_parse_ctx ctx = {
			.panic_buf      = NULL,
			.error          = UBJF_NO_ERROR,
			.read_info      = state->read_event_info,
			.parse_info     = state->parse_event_info,
			.highp_mode     = state->highp_mode,
			.total_nodes    = 0,
	};

	/* Do the actual reading. */
	GUARD_ERROR(ctx.panic_buf, ctx.error)
	{
		ubjf_s12_read_node_recursive(&ctx);
		/* No need to end the guard, since ubjf_s12_read_next is always top-level. */
	}

	UBJF_SET_OPTIONAL(nodes, ctx.total_nodes);
	return ctx.error;
}
#endif