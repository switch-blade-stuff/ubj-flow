//
// Created by switchblade on 2022-02-23.
//

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "ubjf.h"

void test_error_msg();
void test_parse_error();
void test_parse_noop_array();
void test_bool_parse();
void test_parse_bool_array();
void test_string_parse();
void test_parse_string_array();

int main()
{
	test_error_msg();
	test_parse_error();
	//test_bool_parse();
	//test_string_parse();
	test_parse_noop_array();
	test_parse_bool_array();
	//test_parse_string_array();
}

void test_error_msg()
{
	assert(ubjf_make_error_msg(UBJF_NO_ERROR) == NULL);
	assert(strcmp(ubjf_make_error_msg(UBJF_ERROR_ALLOC), "Memory allocation error") == 0);

	const char *msg = ubjf_make_error_msg(UBJF_MAKE_PARAM_ERROR(1));
	assert(strcmp(msg, "Unexpected argument [001]") == 0);
	ubjf_free_error_message(msg);

	msg = ubjf_make_error_msg(UBJF_MAKE_PARAM_ERROR(20));
	assert(strcmp(msg, "Unexpected argument [020]") == 0);
	ubjf_free_error_message(msg);

	msg = ubjf_make_error_msg(UBJF_MAKE_PARAM_ERROR(255));
	assert(strcmp(msg, "Unexpected argument [255]") == 0);
	ubjf_free_error_message(msg);

	msg = ubjf_make_error_msg(UBJF_MAKE_PARAM_ERROR(256));
	assert(strcmp(msg, "Unexpected argument [000]") == 0);
	ubjf_free_error_message(msg);
}

void test_parse_error()
{
	ubjf_read_state state;
	ubjf_error error;

	{
		ubjf_init_buffer_read(&state, NULL, 0, NULL, &error);
		assert(error & UBJF_PARAM_ERROR_MASK);
		assert(UBJF_PARAM_ERROR_GET_INDEX(error) == 1);
		ubjf_destroy_buffer_read(&state);
	}

	{
		const char bad_data[8] = "bad_data";
		ubjf_parse_event_info event_info = {};
		size_t nodes = 0;

		ubjf_init_buffer_read(&state, bad_data, sizeof(bad_data), &event_info, &error);
		assert(error == UBJF_NO_ERROR);

		assert(ubjf_read_next(&state, &error, &nodes) == 1);
		assert(error == UBJF_ERROR_BAD_DATA);

		ubjf_destroy_buffer_read(&state);
	}
}

void test_int_parse()
{
	const char data[] = "";
}
void test_string_parse();

typedef struct
{
	bool started;
	bool ended;
	size_t array_size;
	ubjf_type value_type;
	size_t noop_count;
} test_parse_noop_ctx;

static ubjf_error test_parse_noop_array_on_value(ubjf_value value, test_parse_noop_ctx *ctx)
{
	assert(value.type == UBJF_NOOP);
	ctx->noop_count++;
	return UBJF_NO_ERROR;
}
static ubjf_error test_parse_noop_array_on_container_begin(ubjf_type container_type, int64_t fixed_size,
                                                           ubjf_type value_type, test_parse_noop_ctx *ctx)
{
	ctx->started = true;
	assert(container_type == UBJF_ARRAY);
	assert((ctx->array_size = fixed_size) != 0);
	assert((ctx->value_type = value_type) == UBJF_NOOP);
	return UBJF_NO_ERROR;
}
static ubjf_error test_parse_noop_array_on_container_end(test_parse_noop_ctx *ctx)
{
	ctx->ended = true;
	return UBJF_NO_ERROR;
}

void test_parse_noop_array()
{
	const char data[] = "[$N#i\x0f";

	ubjf_read_state state;
	ubjf_error error;
	size_t nodes_read;

	test_parse_noop_ctx ctx = {};
	ubjf_parse_event_info event_info = {
			.udata = &ctx,
			.on_value = (ubjf_on_value_func) test_parse_noop_array_on_value,
			.on_container_begin = (ubjf_on_container_begin_func) test_parse_noop_array_on_container_begin,
			.on_container_end = (ubjf_on_container_end_func) test_parse_noop_array_on_container_end,
	};

	ubjf_init_buffer_read(&state, data, sizeof(data), &event_info, &error);
	assert(error == UBJF_NO_ERROR);

	ubjf_read_next(&state, &error, &nodes_read);
	assert(error == UBJF_NO_ERROR);
	assert(ctx.started);
	assert(ctx.ended);
	assert(ctx.value_type == UBJF_NOOP);
	assert(ctx.noop_count == 0xf);
	assert(ctx.array_size == ctx.noop_count);
	assert(nodes_read == ctx.noop_count + 1);

	ubjf_destroy_buffer_read(&state);
}

typedef struct
{
	bool started;
	bool ended;
	size_t array_size;
	size_t bool_count;
} test_parse_bool_ctx;

static ubjf_error test_parse_bool_array_on_value(ubjf_value value, test_parse_bool_ctx *ctx)
{
	assert(value.type == UBJF_BOOL);
	ctx->bool_count++;
	return UBJF_NO_ERROR;
}
static ubjf_error test_parse_bool_array_on_container_begin(ubjf_type container_type, int64_t fixed_size,
                                                           ubjf_type value_type, test_parse_bool_ctx *ctx)
{
	ctx->started = true;
	assert(container_type == UBJF_ARRAY);
	assert((ctx->array_size = fixed_size) != 0);
	assert(value_type == UBJF_BAD_TYPE);
	return UBJF_NO_ERROR;
}
static ubjf_error test_parse_bool_array_on_container_end(test_parse_bool_ctx *ctx)
{
	ctx->ended = true;
	return UBJF_NO_ERROR;
}

void test_parse_bool_array()
{
	const char data[] = "[#i\x0fTTTTTTTTTTTTTTF";

	ubjf_read_state state;
	ubjf_error error;
	size_t nodes_read;

	test_parse_bool_ctx ctx = {};
	ubjf_parse_event_info event_info = {
			.udata = &ctx,
			.on_value = (ubjf_on_value_func) test_parse_bool_array_on_value,
			.on_container_begin = (ubjf_on_container_begin_func) test_parse_bool_array_on_container_begin,
			.on_container_end = (ubjf_on_container_end_func) test_parse_bool_array_on_container_end,
	};

	ubjf_init_buffer_read(&state, data, sizeof(data), &event_info, &error);
	assert(error == UBJF_NO_ERROR);

	ubjf_read_next(&state, &error, &nodes_read);
	assert(error == UBJF_NO_ERROR);
	assert(ctx.started);
	assert(ctx.ended);
	assert(ctx.bool_count == 0xf);
	assert(ctx.array_size == ctx.bool_count);
	assert(nodes_read == ctx.bool_count + 1);

	ubjf_destroy_buffer_read(&state);
}

void test_parse_string_array();