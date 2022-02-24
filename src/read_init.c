//
// Created by switchblade on 2022-02-23.
//

#include "ubjf_read.h"

#include <stdlib.h>
#include <string.h>

void ubjf_init_read(ubjf_read_state *state,
                    const ubjf_read_event_info *read_info,
                    const ubjf_parse_event_info *parse_info,
                    ubjf_error *out_error)
{
	ubjf_error error = UBJF_NO_ERROR;
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		error = UBJF_MAKE_PARAM_ERROR(0);
	else if (UBJF_UNLIKELY(!read_info))
		error = UBJF_MAKE_PARAM_ERROR(1);
	else if (UBJF_UNLIKELY(!parse_info))
		error = UBJF_MAKE_PARAM_ERROR(2);
	else
#endif
	{
		state->read_event_info = *read_info;
		state->parse_event_info = *parse_info;
	}

	UBJF_SET_OPTIONAL(out_error, error);
}

static size_t file_read(void *dest, size_t n, FILE *file) { return fread(dest, 1, n, file); }
static size_t file_bump(size_t n, FILE *file) { return fseek(file, n, SEEK_CUR) ? 0 : n; }
static int file_peek(FILE *file) { return ungetc(getc(file), file); }
void ubjf_init_file_read(ubjf_read_state *state, FILE *file, const ubjf_parse_event_info *parse_info,
                         ubjf_error *out_error)
{
	ubjf_error error = UBJF_NO_ERROR;
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		error = UBJF_MAKE_PARAM_ERROR(0);
	else if (UBJF_UNLIKELY(!file))
		error = UBJF_MAKE_PARAM_ERROR(1);
	else if (UBJF_UNLIKELY(!parse_info))
		error = UBJF_MAKE_PARAM_ERROR(2);
	else
#endif
	{
		state->read_event_info.read = (ubjf_read_func) fread;
		state->read_event_info.bump = (ubjf_bump_func) file_bump;
		state->read_event_info.peek = (ubjf_peek_func) file_peek;
		state->read_event_info.udata = file;
		state->parse_event_info = *parse_info;
	}

	UBJF_SET_OPTIONAL(out_error, error);
}
void ubjf_destroy_file_read(ubjf_read_state *state) {}

struct buffer_read_data
{
	const void *buffer;
	size_t size;
};
static size_t buffer_read(void *dest, size_t n, struct buffer_read_data *udata)
{
	if (UBJF_UNLIKELY(n > udata->size))
		n = udata->size;
	memcpy(dest, udata->buffer, n);

	udata->buffer += n;
	udata->size -= n;
	return n;
}
static size_t buffer_bump(size_t n, struct buffer_read_data *udata)
{
	if (UBJF_UNLIKELY(n > udata->size))
		n = udata->size;
	udata->buffer += n;
	udata->size -= n;
	return n;
}
static int buffer_peek(struct buffer_read_data *udata)
{
	return udata->size ? *(const char *) udata->buffer : EOF;
}
void ubjf_init_buffer_read(ubjf_read_state *state, const void *buffer, size_t buffer_size,
                           const ubjf_parse_event_info *parse_info, ubjf_error *out_error)
{
	ubjf_error error = UBJF_NO_ERROR;
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		error = UBJF_MAKE_PARAM_ERROR(0);
	else if (UBJF_UNLIKELY(!buffer))
		error = UBJF_MAKE_PARAM_ERROR(1);
	else if (UBJF_UNLIKELY(!buffer_size))
		error = UBJF_MAKE_PARAM_ERROR(2);
	else if (UBJF_UNLIKELY(!parse_info))
		error = UBJF_MAKE_PARAM_ERROR(3);
	else
#endif
	{
		struct buffer_read_data *buffer_data = UBJF_MALLOC(sizeof(struct buffer_read_data));
		if (UBJF_UNLIKELY(!buffer_data))
			error = UBJF_ERROR_ALLOC;
		else
		{
			buffer_data->buffer = buffer;
			buffer_data->size = buffer_size;
			state->read_event_info.udata = buffer_data;
			state->read_event_info.read = (ubjf_read_func) buffer_read;
			state->read_event_info.bump = (ubjf_bump_func) buffer_bump;
			state->read_event_info.peek = (ubjf_peek_func) buffer_peek;
			state->parse_event_info = *parse_info;
		}
	}

	UBJF_SET_OPTIONAL(out_error, error);
}
void ubjf_destroy_buffer_read(ubjf_read_state *state)
{
	if (UBJF_LIKELY(state))
		UBJF_FREE(state->read_event_info.udata);
}

void ubjf_destroy_value(ubjf_value value)
{
	if (value.type == UBJF_STRING || value.type == UBJF_HIGHP)
		UBJF_FREE((void *) value.string);
}
