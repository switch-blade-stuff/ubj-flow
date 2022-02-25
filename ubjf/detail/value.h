//
// Created by switchblade on 2022-02-24.
//

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "type.h"

typedef struct
{
	size_t size;
	const char *data;
} ubjf_string;

typedef struct
{
	union
	{
		bool boolean;
		char character;
		int8_t int8;
		uint8_t uint8;
		int16_t int16;
		int32_t int32;
		int64_t int64;
		float float32;
		double float64;
		ubjf_string highp;
		ubjf_string string;
	};
	ubjf_type type;
} ubjf_value;