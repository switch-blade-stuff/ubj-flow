//
// Created by switchblade on 2022-02-23.
//

#include "../read.h"

#include <stdlib.h>
#include <string.h>

ubjf_error ubjf_init_read(ubjf_read_state *state, ubjf_read_state_info init_info)
{
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		return UBJF_MAKE_PARAM_ERROR(0);
#endif

	state->read_event_info = init_info.read_event_info;
	state->parse_event_info = init_info.parse_event_info;
	state->highp_mode = init_info.highp_mode;
	state->syntax = init_info.syntax;

	return UBJF_NO_ERROR;
}
void ubjf_destroy_read(ubjf_read_state *state) {}

static size_t ubjf_file_read(void *UBJF_RESTRICT dest, size_t n, FILE *UBJF_RESTRICT file)
{
	return fread(dest, 1, n, file);
}
static size_t ubjf_file_bump(size_t n, FILE *file) { return fseek(file, n, SEEK_CUR) ? 0 : n; }
static int ubjf_file_peek(FILE *file) { return ungetc(getc(file), file); }
ubjf_error ubjf_init_file_read(ubjf_read_state *state, ubjf_read_state_info init_info, FILE *file)
{
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		return UBJF_MAKE_PARAM_ERROR(0);
	else if (UBJF_UNLIKELY(!file))
		return UBJF_MAKE_PARAM_ERROR(2);
#endif

	ubjf_read_event_info read_info = {
			.udata = file,
			.read = (ubjf_read_func) ubjf_file_read,
			.bump = (ubjf_bump_func) ubjf_file_bump,
			.peek = (ubjf_peek_func) ubjf_file_peek,
	};
	init_info.read_event_info = read_info;

	return ubjf_init_read(state, init_info);
}
void ubjf_destroy_file_read(ubjf_read_state *state) { ubjf_destroy_read(state); }

struct buffer_read_data
{
	const char *buffer;
	size_t size;
};
static size_t ubjf_buffer_read(void *UBJF_RESTRICT dest, size_t n, struct buffer_read_data *UBJF_RESTRICT udata)
{
	if (UBJF_UNLIKELY(n > udata->size))
		n = udata->size;
	memcpy(dest, udata->buffer, n);

	udata->buffer += n;
	udata->size -= n;
	return n;
}
static size_t ubjf_buffer_bump(size_t n, struct buffer_read_data *UBJF_RESTRICT udata)
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
ubjf_error ubjf_init_buffer_read(ubjf_read_state *UBJF_RESTRICT state, ubjf_read_state_info init_info,
                                 const void *UBJF_RESTRICT buffer, size_t buffer_size)
{
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		return UBJF_MAKE_PARAM_ERROR(0);
	else if (UBJF_UNLIKELY(!buffer))
		return UBJF_MAKE_PARAM_ERROR(2);
	else if (UBJF_UNLIKELY(!buffer_size))
		return UBJF_MAKE_PARAM_ERROR(3);
#endif

	struct buffer_read_data *buffer_data = UBJF_MALLOC(sizeof(struct buffer_read_data));
	if (UBJF_UNLIKELY(!buffer_data))
		return UBJF_ERROR_ALLOC;

	buffer_data->buffer = buffer;
	buffer_data->size = buffer_size;

	ubjf_read_event_info read_info = {
			.udata = buffer_data,
			.read = (ubjf_read_func) ubjf_buffer_read,
			.bump = (ubjf_bump_func) ubjf_buffer_bump,
			.peek = (ubjf_peek_func) ubjf_buffer_peek,
	};
	init_info.read_event_info = read_info;

	return ubjf_init_read(state, init_info);
}
void ubjf_destroy_buffer_read(ubjf_read_state *state)
{
	if (UBJF_LIKELY(state))
		UBJF_FREE(state->read_event_info.udata);
	ubjf_destroy_read(state);
}
