//
// Created by switchblade on 2022-02-23.
//

#pragma once

#include "define.h"
#include "ubjf_error.h"
#include "ubjf_type.h"

typedef size_t (*ubjf_write_func)(const void *src, size_t size, size_t n, void *udata);

typedef struct
{
	/** Function used to request data write. */
	ubjf_write_func write;
	/** Work buffer. */
	void *buffer;
	/** Work buffer size. */
	size_t buffer_size;
	/** User data passed to parse_events. */
	void *udata;
} ubjf_write_state;