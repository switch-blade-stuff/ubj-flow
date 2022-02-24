//
// Created by switchblade on 2022-02-23.
//

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "define.h"

#define UBJF_BOOL_TYPE_MASK 0b1000
#define UBJF_INTEGER_TYPE_MASK 0b10000
#define UBJF_FLOAT_TYPE_MASK 0b100000
#define UBJF_NUMERIC_TYPE_MASK UBJF_INTEGER_TYPE_MASK | UBJF_FLOAT_TYPE_MASK
#define UBJF_VALUE_TYPE_MASK 0b1000000
#define UBJF_CONTAINER_TYPE_MASK 0b10000000

#define UBJF_VALUE_TYPE(value) UBJF_VALUE_TYPE_MASK | (value)
#define UBJF_BOOL_TYPE(value) UBJF_VALUE_TYPE(UBJF_BOOL_TYPE_MASK | (value))
#define UBJF_INTEGER_TYPE(value) UBJF_VALUE_TYPE(UBJF_INTEGER_TYPE_MASK | (value))
#define UBJF_FLOAT_TYPE(value) UBJF_VALUE_TYPE(UBJF_FLOAT_TYPE_MASK | (value))
#define UBJF_CONTAINER_TYPE(value) UBJF_CONTAINER_TYPE_MASK | (value)

/** @brief Enum encoding all types defined by UBJSON. */
typedef enum
{
	UBJF_INVALID = 0,
	UBJF_NULL = UBJF_VALUE_TYPE(0),
	UBJF_NOOP = UBJF_VALUE_TYPE(1),
	UBJF_CHAR = UBJF_VALUE_TYPE(2),
	UBJF_STRING = UBJF_VALUE_TYPE(3),
	UBJF_HIGHP = UBJF_VALUE_TYPE(4),

	UBJF_FALSE = UBJF_BOOL_TYPE(false),
	UBJF_TRUE = UBJF_BOOL_TYPE(true),

	UBJF_INT8 = UBJF_INTEGER_TYPE(1),
	UBJF_UINT8 = UBJF_INTEGER_TYPE(2),
	UBJF_INT16 = UBJF_INTEGER_TYPE(3),
	UBJF_INT32 = UBJF_INTEGER_TYPE(4),
	UBJF_INT64 = UBJF_INTEGER_TYPE(5),

	UBJF_FLOAT64 = UBJF_FLOAT_TYPE(2),
	UBJF_FLOAT32 = UBJF_FLOAT_TYPE(1),

	UBJF_ARRAY = UBJF_CONTAINER_TYPE(1),
	UBJF_OBJECT = UBJF_CONTAINER_TYPE(2),
} ubjf_type;

/** Returns string containing name of the specified type.
 * @param[in] type Type to get name for.
 * @return Pointer to the name string. */
UBJF_EXTERN const char *ubjf_get_type_name(ubjf_type type);