//
// Created by switchblade on 2022-02-23.
//

#include "detail/error.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* Need to store this externally. No allocations after allocation error. */
static const char *alloc_error_msg = "Memory allocation error";

static const char *format_param_error(uint8_t idx)
{
	const char template[] = "Unexpected argument [   ]";

	char *msg = UBJF_MALLOC(sizeof(template));
	strcpy(msg, template);

	for (size_t i = 0, dest = 24; i < 3; ++i, idx /= 10)
		msg[--dest] = (char) ('0' + idx % 10);

	return msg;
}

const char *ubjf_make_error_msg(ubjf_error err)
{
	const char *msg = NULL;
	if (err == UBJF_ERROR_ALLOC)
		msg = alloc_error_msg;
	else if (err == UBJF_EOF)
		msg = strcpy(UBJF_MALLOC(13), "End of input");
	else if (err == UBJF_ERROR_BAD_WRITE)
		msg = strcpy(UBJF_MALLOC(26), "Failed to write to output");
	else if (err == UBJF_ERROR_BAD_DATA)
		msg = strcpy(UBJF_MALLOC(16), "Failed to parse");
	else if (err == UBJF_ERROR_UNKNOWN)
		msg = strcpy(UBJF_MALLOC(14), "Unknown error");
	else if (err & UBJF_ERROR_PARAM)
		msg = format_param_error(UBJF_PARAM_ERROR_GET_INDEX(err));
	return msg;
}
void ubjf_free_error_message(const char *msg)
{
	if (msg && msg != alloc_error_msg)
		UBJF_FREE((void *) msg);
}