//
// Created by switchblade on 2022-02-24.
//

#pragma once

#include "error.h"

#include <setjmp.h>

#define GUARD_ERROR(buf, error) \
    jmp_buf frame_buf;          \
    jmp_buf *last_frame = buf;  \
    buf = &frame_buf;           \
    if ((error = setjmp(frame_buf)) == UBJF_NO_ERROR)
#define END_GUARD(buf) buf = last_frame

#define THROW_ERROR(buf, error) longjmp(*buf, error)
#define RETHROW_ERROR(buf, error) \
    do {                          \
        END_GUARD(buf);           \
        THROW_ERROR(buf, error);  \
    } while (0)
