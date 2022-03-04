//
// Created by switchblade on 2022-03-03.
//

#pragma once

#include "../../write.h"

#include <stdlib.h>

#include "../error_handling.h"
#include "../token.h"
#include "../bswap.h"

typedef struct
{
	jmp_buf *panic_buf;
	ubjf_write_event_info write_info;
	ubjf_error error;
} ubjf_emit_ctx;

static inline void ubjf_guarded_write(ubjf_emit_ctx *restrict ctx, const void *restrict src, size_t n)
{
	if (UBJF_UNLIKELY(!(ctx->write_info.write && ctx->write_info.write(src, n, ctx->write_info.udata) == n)))
		THROW_ERROR(ctx->panic_buf, UBJF_ERROR_BAD_WRITE);
}
