//
// Created by switchblade on 2022-02-25.
//

#include <stdlib.h>
#include <string.h>

#include "write.h"

ubjf_error ubjf_init_write(ubjf_write_state *state, ubjf_write_event_info write_info)
{
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		return UBJF_MAKE_PARAM_ERROR(0);
#endif

	state->container_stack = state->current_container = state->stack_end = NULL;
	state->write_event_info = write_info;
	return UBJF_NO_ERROR;
}
void ubjf_destroy_write(ubjf_write_state *state)
{
	if (UBJF_LIKELY(state && state->container_stack))
		UBJF_FREE(state->container_stack);
}
void ubjf_destroy_file_write(ubjf_write_state *state) { ubjf_destroy_write(state); }

struct buffer_write_data
{
	void *write_pos;
	void *buffer_end;
};
static size_t ubjf_buffer_write(const void *src, size_t n, struct buffer_write_data *udata)
{
	size_t left = udata->buffer_end - udata->write_pos;
	if (UBJF_UNLIKELY(left < n))
		n = left;

	memcpy(udata->write_pos, src, n);
	udata->write_pos += n;

	return n;
}
ubjf_error ubjf_init_buffer_write(ubjf_write_state *state, void *buffer, size_t buffer_size)
{
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		return UBJF_MAKE_PARAM_ERROR(0);
	else if (UBJF_UNLIKELY(!buffer))
		return UBJF_MAKE_PARAM_ERROR(1);
	else if (UBJF_UNLIKELY(!buffer_size))
		return UBJF_MAKE_PARAM_ERROR(2);
#endif

	struct buffer_write_data *buffer_data = UBJF_MALLOC(sizeof(struct buffer_write_data));
	if (UBJF_UNLIKELY(!buffer_data))
		return UBJF_ERROR_ALLOC;

	buffer_data->write_pos = buffer;
	buffer_data->buffer_end = buffer + buffer_size;

	ubjf_write_event_info info = {
			.write = (ubjf_write_func) ubjf_buffer_write,
			.udata = buffer_data,
	};
	return ubjf_init_write(state, info);
}
void ubjf_destroy_buffer_write(ubjf_write_state *state)
{
	if (UBJF_LIKELY(state))
		UBJF_FREE(state->write_event_info.udata);
	ubjf_destroy_write(state);
}