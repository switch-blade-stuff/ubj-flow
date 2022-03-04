//
// Created by switchblade on 2022-03-03.
//

#pragma once

#include "../../read.h"

#include <stdlib.h>

#include "../error_handling.h"
#include "../token.h"
#include "../bswap.h"

typedef struct
{
	jmp_buf *panic_buf;
	ubjf_error error;
	ubjf_read_event_info read_info;
	ubjf_parse_event_info parse_info;
	ubjf_highp_mode highp_mode;
	size_t total_nodes;
} ubjf_parse_ctx;

static inline void ubjf_guarded_read(ubjf_parse_ctx *UBJF_RESTRICT ctx, void *UBJF_RESTRICT dest, size_t n)
{
	if (UBJF_UNLIKELY(!(ctx->read_info.read && ctx->read_info.read(dest, n, ctx->read_info.udata) == n)))
		THROW_ERROR(ctx->panic_buf, UBJF_EOF);
}
static inline char ubjf_guarded_peek(ubjf_parse_ctx *ctx)
{
	int result;
	if (UBJF_UNLIKELY(!(ctx->read_info.peek && (result = ctx->read_info.peek(ctx->read_info.udata)) != EOF)))
		THROW_ERROR(ctx->panic_buf, UBJF_EOF);
	return (char) result;
}
static inline void ubjf_guarded_bump(ubjf_parse_ctx *ctx, size_t n)
{
	if (UBJF_UNLIKELY(!(ctx->read_info.bump && ctx->read_info.bump(n, ctx->read_info.udata))))
		THROW_ERROR(ctx->panic_buf, UBJF_EOF);
}