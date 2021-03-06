//
// Created by switchblade on 2022-03-03.
//

#ifndef UBJF_NO_SPEC_12

#include "../emitter.h"

static const ubjf_token ubjf_type_token_map[] = {
		[UBJF_NULL]         = UBJF_TOKEN_NULL,
		[UBJF_NOOP]         = UBJF_TOKEN_NOOP,
		[UBJF_FALSE_TYPE]   = UBJF_TOKEN_FALSE,
		[UBJF_TRUE_TYPE]    = UBJF_TOKEN_TRUE,
		[UBJF_INT8]         = UBJF_TOKEN_INT8,
		[UBJF_UINT8]        = UBJF_TOKEN_UINT8,
		[UBJF_INT16]        = UBJF_TOKEN_INT16,
		[UBJF_INT32]        = UBJF_TOKEN_INT32,
		[UBJF_INT64]        = UBJF_TOKEN_INT64,
		[UBJF_FLOAT32]      = UBJF_TOKEN_FLOAT32,
		[UBJF_FLOAT64]      = UBJF_TOKEN_FLOAT64,
		[UBJF_HIGHP]        = UBJF_TOKEN_HIGHP,
		[UBJF_CHAR]         = UBJF_TOKEN_CHAR,
		[UBJF_STRING]       = UBJF_TOKEN_STRING,
		[UBJF_ARRAY]        = UBJF_TOKEN_ARRAY_START,
		[UBJF_OBJECT]       = UBJF_TOKEN_OBJECT_START,
};

static void ubjf_s12_emit_token(ubjf_emit_ctx *ctx, ubjf_token token) { ubjf_guarded_write(ctx, &token, 1); }
static void ubjf_s12_emit_type_token(ubjf_emit_ctx *ctx, ubjf_value value)
{
	/* Store boolean value within the type. */
	if (value.type == UBJF_BOOL)
		value.type = UBJF_BOOL_TYPE_MASK | value.boolean;

	ubjf_token token = ubjf_type_token_map[value.type];
	if (UBJF_UNLIKELY(!token))
		THROW_ERROR(ctx->panic_buf, UBJF_ERROR_BAD_TYPE);

	ubjf_s12_emit_token(ctx, token);
}
static void ubjf_s12_emit_integer(ubjf_emit_ctx *ctx, ubjf_value value)
{
	switch (value.type)
	{
		case UBJF_INT8:
		case UBJF_UINT8:
		{
			uint8_t temp = value.integer;
			ubjf_guarded_write(ctx, &temp, sizeof(temp));
			break;
		}
		case UBJF_INT16:
		{
			int16_t temp = FIX_ENDIANNESS_16_BE((int16_t) value.integer);
			ubjf_guarded_write(ctx, &temp, sizeof(temp));
			break;
		}
		case UBJF_INT32:
		{
			int32_t temp = FIX_ENDIANNESS_32_BE((int32_t) value.integer);
			ubjf_guarded_write(ctx, &temp, sizeof(temp));
			break;
		}
		case UBJF_INT64:
		{
			int64_t temp = FIX_ENDIANNESS_64_BE(value.integer);
			ubjf_guarded_write(ctx, &temp, sizeof(temp));
			break;
		}
		default:
			break;
	}
}
static void ubjf_s12_emit_float(ubjf_emit_ctx *ctx, ubjf_value value)
{
	switch (value.type)
	{
		case UBJF_FLOAT32:
		{
			float temp = FIX_ENDIANNESS_32_BE((float) value.floating);
			ubjf_guarded_write(ctx, &temp, sizeof(temp));
			break;
		}
		case UBJF_FLOAT64:
		{
			double temp = FIX_ENDIANNESS_64_BE(value.floating);
			ubjf_guarded_write(ctx, &temp, sizeof(temp));
			break;
		}
		default:
			break;
	}
}
static void ubjf_s12_emit_length(ubjf_emit_ctx *ctx, int64_t length)
{
	ubjf_value value = {.integer = length};
	if (length > INT32_MAX)
		value.type = UBJF_INT64;
	else if (length > INT16_MAX)
		value.type = UBJF_INT32;
	else if (length > UINT8_MAX)
		value.type = UBJF_INT16;
	else if (length > INT8_MAX)
		value.type = UBJF_UINT8;
	else
		value.type = UBJF_INT8;

	ubjf_s12_emit_type_token(ctx, value);
	ubjf_s12_emit_integer(ctx, value);
}
static void ubjf_s12_emit_string(ubjf_emit_ctx *ctx, ubjf_string str)
{
	ubjf_s12_emit_length(ctx, str.size);
	ubjf_guarded_write(ctx, str.data, str.size);
}

ubjf_error ubjf_s12_write_value(ubjf_write_state *state, ubjf_value value)
{
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		return UBJF_MAKE_PARAM_ERROR(0);
#endif

	ubjf_emit_ctx ctx = {
			.panic_buf   = NULL,
			.write_info  = state->write_event_info,
			.error       = UBJF_NO_ERROR,
	};

	GUARD_ERROR(ctx.panic_buf, ctx.error)
	{
		/* Don't emit type token if we are within a fixed-type container. */
		if (!state->current_container || state->current_container->value_type == UBJF_NO_TYPE)
			ubjf_s12_emit_type_token(&ctx, value);
		else if (UBJF_UNLIKELY(state->current_container->value_type != value.type))
			THROW_ERROR(ctx.panic_buf, UBJF_ERROR_BAD_TYPE); /* Make sure value type matches the fixed type. */

		if (value.type & UBJF_INTEGER_TYPE_MASK)
			ubjf_s12_emit_integer(&ctx, value);
		else if (value.type & UBJF_FLOAT_TYPE_MASK)
			ubjf_s12_emit_float(&ctx, value);
		else if (value.type == UBJF_STRING || value.type == UBJF_HIGHP)
			ubjf_s12_emit_string(&ctx, value.string);

		/* No need to end the guard, since ubjf_s12_read_next is always top-level. */
	}

	return ctx.error;
}

static ubjf_error ubjf_s12_push_info(ubjf_write_state *state, ubjf_container_info info)
{
	state->current_container = state->current_container ? ++state->current_container : state->container_stack;
	if (state->current_container == state->stack_end)
	{
		size_t old_size = state->stack_end - state->container_stack;
		size_t new_size = old_size ? old_size * 2 : 4;

		state->container_stack = UBJF_REALLOC((void *) state->container_stack, new_size);
		if (UBJF_UNLIKELY(!state->container_stack))
			return UBJF_ERROR_ALLOC;
		state->current_container = state->container_stack + old_size;
		state->stack_end = state->container_stack + new_size;
	}

	*state->current_container = info;
	return UBJF_NO_ERROR;
}
static void ubjf_s12_pop_info(ubjf_write_state *state)
{
	if (state->current_container-- == state->container_stack)
		state->current_container = NULL;
}

static bool ubjf_s12_validate_info(ubjf_container_info info)
{
	return (info.container_type == UBJF_ARRAY || info.container_type == UBJF_OBJECT) &&
	       (info.container_type == UBJF_NO_TYPE || info.length >= 0);
}
ubjf_error ubjf_s12_start_container(ubjf_write_state *state, ubjf_container_info info)
{
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		return UBJF_MAKE_PARAM_ERROR(0);
	else if (UBJF_UNLIKELY(!ubjf_s12_validate_info(info)))
		return UBJF_MAKE_PARAM_ERROR(1);
#endif

	{
		ubjf_error result = ubjf_s12_push_info(state, info);
		if (UBJF_UNLIKELY(result != UBJF_NO_ERROR))
			return result;
	}

	ubjf_emit_ctx ctx = {
			.panic_buf  = NULL,
			.write_info = state->write_event_info,
			.error      = UBJF_NO_ERROR,
	};

	GUARD_ERROR(ctx.panic_buf, ctx.error)
	{
		ubjf_s12_emit_token(&ctx, ubjf_type_token_map[info.container_type]);

		if (info.container_type != UBJF_NO_TYPE && info.container_type != UBJF_BOOL) /* Ignore regular bool type. */
		{
			ubjf_s12_emit_token(&ctx, UBJF_TOKEN_CONTAINER_TYPE);
			ubjf_s12_emit_token(&ctx, ubjf_type_token_map[info.value_type]);
		}
		if (info.length >= 0)
		{
			ubjf_s12_emit_token(&ctx, UBJF_TOKEN_CONTAINER_LENGTH);
			ubjf_s12_emit_length(&ctx, info.length);
		}

		/* Elements are to be written by the user. */
	}

	return ctx.error;
}
ubjf_error ubjf_s12_write_object_key(ubjf_write_state *state, ubjf_string key)
{
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		return UBJF_MAKE_PARAM_ERROR(0);
#endif

	ubjf_emit_ctx ctx = {
			.panic_buf  = NULL,
			.write_info = state->write_event_info,
			.error      = UBJF_NO_ERROR,
	};

	GUARD_ERROR(ctx.panic_buf, ctx.error)
	{
		ubjf_s12_emit_string(&ctx, key);
	}

	return ctx.error;
}
ubjf_error ubjf_s12_end_container(ubjf_write_state *state)
{
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!(state && state->current_container)))
		return UBJF_MAKE_PARAM_ERROR(0);
#endif

	ubjf_container_info container = *state->current_container;
	if (container.length >= 0)
	{
		ubjf_emit_ctx ctx = {
				.panic_buf  = NULL,
				.write_info = state->write_event_info,
				.error      = UBJF_NO_ERROR,
		};

		GUARD_ERROR(ctx.panic_buf, ctx.error)
		{
			/* Don't emit close token if we are within a fixed-size container. */
			if (container.container_type == UBJF_ARRAY)
				ubjf_s12_emit_token(&ctx, UBJF_TOKEN_ARRAY_END);
			else
				ubjf_s12_emit_token(&ctx, UBJF_TOKEN_OBJECT_END);
		}
	}

	ubjf_s12_pop_info(state);
	return UBJF_NO_ERROR;
}
#endif