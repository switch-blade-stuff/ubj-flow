//
// Created by switchblade on 2022-02-24.
//

#pragma once

#define UBJF_VALUE_TYPE_MASK 0b1000
#define UBJF_VALUE_TYPE(n) (UBJF_VALUE_TYPE_MASK | (n))
#define UBJF_INTEGER_TYPE_MASK 0b10000
#define UBJF_INTEGER_TYPE(n) UBJF_VALUE_TYPE(UBJF_INTEGER_TYPE_MASK | (n))
#define UBJF_FLOAT_TYPE_MASK 0b100000
#define UBJF_FLOAT_TYPE(n) UBJF_VALUE_TYPE(UBJF_FLOAT_TYPE_MASK | (n))

#define UBJF_CONTAINER_TYPE_MASK 0b1000000
#define UBJF_CONTAINER_TYPE(n) (UBJF_CONTAINER_TYPE_MASK | (n))

typedef enum
{
	UBJF_NO_TYPE = 0,
	UBJF_NULL = UBJF_VALUE_TYPE(1),
	UBJF_NOOP = UBJF_VALUE_TYPE(2),
	UBJF_CHAR = UBJF_VALUE_TYPE(3),
	UBJF_HIGHP = UBJF_VALUE_TYPE(4),
	UBJF_STRING = UBJF_VALUE_TYPE(5),

#ifndef UBJF_NO_SPEC12
	UBJF_BOOL_TYPE_MASK = UBJF_VALUE_TYPE(6),
	UBJF_FALSE_TYPE = UBJF_BOOL_TYPE_MASK,
	UBJF_TRUE_TYPE = UBJF_BOOL_TYPE_MASK | 1,
#endif
	UBJF_BOOL = UBJF_VALUE_TYPE(8),

	UBJF_INT8 = UBJF_INTEGER_TYPE(1),
	UBJF_UINT8 = UBJF_INTEGER_TYPE(2),
	UBJF_INT16 = UBJF_INTEGER_TYPE(3),
	UBJF_INT32 = UBJF_INTEGER_TYPE(4),
	UBJF_INT64 = UBJF_INTEGER_TYPE(5),

	UBJF_FLOAT32 = UBJF_FLOAT_TYPE(1),
	UBJF_FLOAT64 = UBJF_FLOAT_TYPE(2),

	UBJF_ARRAY = UBJF_CONTAINER_TYPE(1),
	UBJF_OBJECT = UBJF_CONTAINER_TYPE(2),
} ubjf_type;