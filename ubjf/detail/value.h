//
// Created by switchblade on 2022-02-24.
//

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "type.h"

typedef struct
{
	/** Size of the string (excluding the null terminator). */
	int64_t size;
	/** Data of the string. */
	const char *data;
} ubjf_string;

typedef struct
{
	union
	{
		bool boolean;
		char character;
		int64_t integer;
		double floating;
		ubjf_string highp;
		ubjf_string string;
	};
	ubjf_type type;
} ubjf_value;