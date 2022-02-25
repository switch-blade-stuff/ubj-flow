//
// Created by switchblade on 2022-02-23.
//

#include "read.h"

#include <stdlib.h>
#include <string.h>

ubjf_error ubjf_init_read(ubjf_read_state *state, ubjf_read_event_info read_info, ubjf_parse_event_info parse_info)
{
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		return UBJF_MAKE_PARAM_ERROR(0);
#endif

	state->read_event_info = read_info;
	state->parse_event_info = parse_info;

	return UBJF_NO_ERROR;
}

static size_t ubjf_file_read(void *dest, size_t n, FILE *file) { return fread(dest, 1, n, file); }
static size_t ubjf_file_bump(size_t n, FILE *file) { return fseek(file, n, SEEK_CUR) ? 0 : n; }
static int ubjf_file_peek(FILE *file) { return ungetc(getc(file), file); }
ubjf_error ubjf_init_file_read(ubjf_read_state *state, FILE *file, ubjf_parse_event_info parse_info)
{
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		return UBJF_MAKE_PARAM_ERROR(0);
	else if (UBJF_UNLIKELY(!file))
		return UBJF_MAKE_PARAM_ERROR(1);
#endif

	state->read_event_info.read = (ubjf_read_func) ubjf_file_read;
	state->read_event_info.bump = (ubjf_bump_func) ubjf_file_bump;
	state->read_event_info.peek = (ubjf_peek_func) ubjf_file_peek;
	state->read_event_info.udata = file;
	state->parse_event_info = parse_info;

	return UBJF_NO_ERROR;
}
void ubjf_destroy_file_read(ubjf_read_state *state) {}

struct buffer_read_data
{
	const void *buffer;
	size_t size;
};
static size_t ubjf_buffer_read(void *dest, size_t n, struct buffer_read_data *udata)
{
	if (UBJF_UNLIKELY(n > udata->size))
		n = udata->size;
	memcpy(dest, udata->buffer, n);

	udata->buffer += n;
	udata->size -= n;
	return n;
}
static size_t ubjf_buffer_bump(size_t n, struct buffer_read_data *udata)
{
	if (UBJF_UNLIKELY(n > udata->size))
		n = udata->size;
	udata->buffer += n;
	udata->size -= n;
	return n;
}
static int ubjf_buffer_peek(struct buffer_read_data *udata)
{
	return udata->size ? *(const char *) udata->buffer : EOF;
}
ubjf_error ubjf_init_buffer_read(ubjf_read_state *state, const void *buffer, size_t buffer_size,
                                 ubjf_parse_event_info parse_info)
{
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		return UBJF_MAKE_PARAM_ERROR(0);
	else if (UBJF_UNLIKELY(!buffer))
		return UBJF_MAKE_PARAM_ERROR(1);
	else if (UBJF_UNLIKELY(!buffer_size))
		return UBJF_MAKE_PARAM_ERROR(2);
#endif

	struct buffer_read_data *buffer_data = UBJF_MALLOC(sizeof(struct buffer_read_data));
	if (UBJF_UNLIKELY(!buffer_data))
		return UBJF_ERROR_ALLOC;

	buffer_data->buffer = buffer;
	buffer_data->size = buffer_size;
	state->read_event_info.udata = buffer_data;
	state->read_event_info.read = (ubjf_read_func) ubjf_buffer_read;
	state->read_event_info.bump = (ubjf_bump_func) ubjf_buffer_bump;
	state->read_event_info.peek = (ubjf_peek_func) ubjf_buffer_peek;
	state->parse_event_info = parse_info;

	return UBJF_NO_ERROR;
}
void ubjf_destroy_buffer_read(ubjf_read_state *state)
{
	if (UBJF_LIKELY(state))
		UBJF_FREE(state->read_event_info.udata);
}
