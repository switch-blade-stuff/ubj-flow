//
// Created by switchblade on 2022-02-23.
//

#include "error.h"

#include <string.h>
#include <stdint.h>

#define BUFFER_SIZE 64

static size_t ubjf_format_param_msg(char *buff, uint8_t idx)
{
	const char template[] = "Unexpected argument [   ]";

	if (buff)
	{
		strcpy(buff, template);
		for (size_t dest = 24; dest > 21; idx /= 10)
			buff[--dest] = (char) ('0' + idx % 10);
	}

	return sizeof(template) - 1;
}
static size_t ubjf_format_simple_msg(char *UBJF_RESTRICT buff, const char *UBJF_RESTRICT error_msg, size_t len)
{
	if (buff)
		strncpy(buff, error_msg, len);

	return len;
}

size_t ubjf_make_error_msg(ubjf_error err, char *str, size_t len)
{
	char buffer[BUFFER_SIZE];
	char *buffer_ptr = (str && len) ? buffer : NULL;
	size_t msg_length;

	if (UBJF_IS_PARAM_ERROR(err))
		msg_length = ubjf_format_param_msg(buffer_ptr, UBJF_PARAM_ERROR_GET_INDEX(err));
	else
	{
		const char *msg = NULL;
		switch (err)
		{
			case UBJF_ERROR_ALLOC:
			{
				msg = "Memory allocation error";
				msg_length = 23;
				break;
			}
			case UBJF_EOF:
			{
				msg = "End of input";
				msg_length = 12;
				break;
			}
			case UBJF_ERROR_BAD_WRITE:
			{
				msg = "Failed to write to output";
				msg_length = 25;
				break;
			}
			case UBJF_ERROR_BAD_DATA:
			{
				msg = "Failed to parse input (invalid data)";
				msg_length = 36;
				break;
			}
			case UBJF_ERROR_BAD_TYPE:
			{
				msg = "Invalid type";
				msg_length = 12;
				break;
			}
			case UBJF_ERROR_UNKNOWN:
			{
				msg = "Unknown error";
				msg_length = 13;
				break;
			}
			case UBJF_ERROR_HIGHP:
			{
				msg = "High precision numbers are disabled";
				msg_length = 35;
				break;
			}
			default:
				msg_length = 0;
				break;
		}

		msg_length = ubjf_format_simple_msg(buffer_ptr, msg, msg_length);
	}

	if (buffer_ptr)
	{
		len = msg_length >= len ? len - 1 : msg_length;
		strncpy(str, buffer_ptr, len);
		str[len] = '\0';
		return len;
	} else
		return msg_length;
}