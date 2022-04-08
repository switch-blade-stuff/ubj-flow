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

void test_write_string();
void test_write_bool_array();
void test_write_int_array();

int main()
{
	int i = sizeof(ubjf_container_info);

	test_error_msg();
	test_parse_error();
	//test_bool_parse();
	//test_string_parse();
	test_parse_noop_array();
	test_parse_bool_array();
	//test_parse_string_array();

	test_write_string();
	test_write_bool_array();
	test_write_int_array();
}

void test_error_msg()
{
	assert(ubjf_make_error_msg(UBJF_NO_ERROR, NULL, 0) == 0);

	char msg[64];
	assert(ubjf_make_error_msg(UBJF_ERROR_ALLOC, msg, sizeof(msg)) != 0);
	assert(strcmp(msg, "Memory allocation error") == 0);

	assert(ubjf_make_error_msg(UBJF_MAKE_PARAM_ERROR(1), msg, sizeof(msg)) != 0);
	assert(strcmp(msg, "Unexpected argument [001]") == 0);

	assert(ubjf_make_error_msg(UBJF_MAKE_PARAM_ERROR(20), msg, sizeof(msg)) != 0);
	assert(strcmp(msg, "Unexpected argument [020]") == 0);

	assert(ubjf_make_error_msg(UBJF_MAKE_PARAM_ERROR(255), msg, sizeof(msg)) != 0);
	assert(strcmp(msg, "Unexpected argument [255]") == 0);

	assert(ubjf_make_error_msg(UBJF_MAKE_PARAM_ERROR(256), msg, sizeof(msg)) != 0);
	assert(strcmp(msg, "Unexpected argument [000]") == 0);
}

void test_parse_error()
{
	ubjf_read_state state;
	ubjf_read_state_info info = {};

	{
		ubjf_error error = ubjf_init_buffer_read(&state, info, NULL, 0);
		assert(UBJF_IS_PARAM_ERROR(error));
		assert(UBJF_PARAM_ERROR_GET_INDEX(error) == 2);
		//ubjf_destroy_buffer_read(&state);
	}

	{
		const char bad_data[8] = "bad_data";
		size_t nodes = 0;

		assert(ubjf_init_buffer_read(&state, info, bad_data, sizeof(bad_data)) == UBJF_NO_ERROR);
		assert(ubjf_read_next(&state, &nodes) == UBJF_ERROR_BAD_DATA);

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
	size_t nodes_read;

	test_parse_noop_ctx ctx = {};
	ubjf_read_state_info info = {
			.parse_event_info = {
					.udata = &ctx,
					.on_value = (ubjf_on_value_func) test_parse_noop_array_on_value,
					.on_container_begin = (ubjf_on_container_begin_func) test_parse_noop_array_on_container_begin,
					.on_container_end = (ubjf_on_container_end_func) test_parse_noop_array_on_container_end,
			},
	};

	assert(ubjf_init_buffer_read(&state, info, data, sizeof(data)) == UBJF_NO_ERROR);

	assert(ubjf_read_next(&state, &nodes_read) == UBJF_NO_ERROR);
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
	assert(value_type == UBJF_NO_TYPE);
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
	ubjf_read_state_info info = {
			.parse_event_info = {
					.udata = &ctx,
					.on_value = (ubjf_on_value_func) test_parse_bool_array_on_value,
					.on_container_begin = (ubjf_on_container_begin_func) test_parse_bool_array_on_container_begin,
					.on_container_end = (ubjf_on_container_end_func) test_parse_bool_array_on_container_end,
			},
	};

	assert(ubjf_init_buffer_read(&state, info, data, sizeof(data)) == UBJF_NO_ERROR);

	assert(ubjf_read_next(&state, &nodes_read) == UBJF_NO_ERROR);
	assert(ctx.started);
	assert(ctx.ended);
	assert(ctx.bool_count == 0xf);
	assert(ctx.array_size == ctx.bool_count);
	assert(nodes_read == ctx.bool_count + 1);

	ubjf_destroy_buffer_read(&state);
}

void test_parse_string_array();

void test_write_string()
{
	const char expected[15] = "Si\x0cHello, world";
	char buffer[sizeof(expected)] = {0};

	ubjf_write_state_info info = {};
	ubjf_write_state state;
	assert(ubjf_init_buffer_write(&state, info, buffer, sizeof(buffer)) == UBJF_NO_ERROR);

	ubjf_string str = {
			.size = 12,
			.data = "Hello, world",
	};
	assert(ubjf_write_string(&state, str) == UBJF_NO_ERROR);
	assert(strncmp(buffer, expected, 12) == 0);

	ubjf_destroy_buffer_write(&state);
}

void test_write_bool_array()
{
	const char expected[6] = "[$T#U\xff";
	char buffer[sizeof(expected)] = {0};

	ubjf_write_state state;
	{
		ubjf_write_state_info info = {};
		assert(ubjf_init_buffer_write(&state, info, buffer, sizeof(buffer)) == UBJF_NO_ERROR);
	}

	ubjf_container_info info = {
			.container_type = UBJF_ARRAY,
			.length = 0xff,
			.value_type = UBJF_TRUE_TYPE,
	};
	assert(ubjf_start_container(&state, info) == UBJF_NO_ERROR);
	assert(ubjf_end_container(&state) == UBJF_NO_ERROR);
	assert(strncmp(buffer, expected, sizeof(expected)) == 0);

	ubjf_destroy_buffer_write(&state);
}


void test_write_int_array()
{
	const char expected[22] = "[$I#i\x08"
	                          "\xaa\xbb\xaa\xbb\xaa\xbb\xaa\xbb"
	                          "\xaa\xbb\xaa\xbb\xaa\xbb\xaa\xbb";
	char buffer[sizeof(expected)] = {0};

	const ubjf_value data[8] = {
			{.integer = (int16_t) 0xaabb, .type = UBJF_INT16},
			{.integer = (int16_t) 0xaabb, .type = UBJF_INT16},
			{.integer = (int16_t) 0xaabb, .type = UBJF_INT16},
			{.integer = (int16_t) 0xaabb, .type = UBJF_INT16},
			{.integer = (int16_t) 0xaabb, .type = UBJF_INT16},
			{.integer = (int16_t) 0xaabb, .type = UBJF_INT16},
			{.integer = (int16_t) 0xaabb, .type = UBJF_INT16},
			{.integer = (int16_t) 0xaabb, .type = UBJF_INT16},
	};

	ubjf_write_state_info info = {};
	ubjf_write_state state;
	assert(ubjf_init_buffer_write(&state, info, buffer, sizeof(buffer)) == UBJF_NO_ERROR);

	assert(ubjf_write_array(&state, data, 8, UBJF_INT16) == UBJF_NO_ERROR);
	assert(strncmp(buffer, expected, sizeof(expected)) == 0);

	ubjf_destroy_buffer_write(&state);
}