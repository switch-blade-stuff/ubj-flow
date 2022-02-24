//
// Created by switchblade on 2022-02-24.
//

#include "read.h"

#include "error_handling.h"
#include "token.h"
#include "bswap.h"

/* Store all relevant data in a single structure for single pointer access. */
typedef struct
{
	jmp_buf *panic_buf;
	ubjf_read_event_info read_event_info;
	ubjf_parse_event_info parse_event_info;
	ubjf_error error;
	size_t total_nodes;
} ubjf_parse_ctx;

typedef struct
{
	int64_t size;
	ubjf_type type;
} ubjf_container_ctx;

typedef struct
{
	int64_t size;
	ubjf_type type;
} ubjf_value_ctx;

static const ubjf_type ubjf_token_type_map[UBJF_TOKEN_MAX] = {
		[UBJF_TOKEN_NULL]         = UBJF_NULL,
		[UBJF_TOKEN_NOOP]         = UBJF_NOOP,
		[UBJF_TOKEN_TRUE]         = UBJF_BOOL,
		[UBJF_TOKEN_FALSE]        = UBJF_BOOL,
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
#define IS_VALID_TYPE_TOKEN(token) (ubjf_token_type_map[token] != 0)

static void ubjf_read_node_recursive(ubjf_parse_ctx *ctx);
int ubjf_read_next(ubjf_read_state *state, ubjf_error *error, size_t *nodes)
{
	ubjf_parse_ctx ctx = {
			.panic_buf      = NULL,
			.error          = UBJF_NO_ERROR,
			.total_nodes    = 0,
	};

#ifdef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		ctx.error = UBJF_MAKE_PARAM_ERROR(0);
	else
#endif
	{
		ctx.read_event_info = state->read_event_info;
		ctx.parse_event_info = state->parse_event_info;

		/* Do the actual reading. */
		ubjf_read_node_recursive(&ctx);
	}

	UBJF_SET_OPTIONAL(error, ctx.error);
	UBJF_SET_OPTIONAL(nodes, ctx.total_nodes);

	if (!ctx.error)
		return 0;
	else if (ctx.error == UBJF_EOF)
		return -1;
	else
		return 1;
}

static void ubjf_read_node_recursive(ubjf_parse_ctx *ctx)
{
	GUARD_ERROR(ctx->panic_buf, ctx->error)
	{

	} else RETHROW_ERROR(ctx->panic_buf, ctx->error);

	END_GUARD(ctx->panic_buf);
}
