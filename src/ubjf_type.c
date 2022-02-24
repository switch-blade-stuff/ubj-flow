//
// Created by switchblade on 2022-02-23.
//

#include "ubjf_type.h"

const char *ubjf_get_type_name(ubjf_type type)
{
	switch (type)
	{
		case UBJF_NULL:
			return "NULL";
		case UBJF_NOOP:
			return "NOP";
		case UBJF_CHAR:
			return "CHAR";
		case UBJF_STRING:
			return "STRING";
		case UBJF_HIGHP:
			return "HIGHP";
		case UBJF_FALSE:
			return "FALSE";
		case UBJF_TRUE:
			return "TRUE";
		case UBJF_INT8:
			return "INT8";
		case UBJF_UINT8:
			return "UINT8";
		case UBJF_INT16:
			return "INT16";
		case UBJF_INT32:
			return "INT32";
		case UBJF_INT64:
			return "INT64";
		case UBJF_FLOAT64:
			return "FLOAT64";
		case UBJF_FLOAT32:
			return "FLOAT32";
		case UBJF_ARRAY:
			return "ARRAY";
		case UBJF_OBJECT:
			return "OBJECT";
		default:
			return 0;
	}
}