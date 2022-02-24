//
// Created by switchblade on 2022-02-23.
//

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "ubjf.h"

void test_error_msg();
void test_parse_error();
void test_bool_parse();
void test_string_parse();
void test_parse_noop();

int main()
{
	test_error_msg();
	test_parse_error();
	test_parse_noop();
	test_bool_parse();
	test_string_parse();
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
	ubjf_read_parse_callback_info callback_info = {};
	ubjf_read_state state;
	ubjf_error error;
	size_t bytes;

	const char bad_data[8] = "bad data";
	ubjf_init_buffer_read(&state, bad_data, sizeof(bad_data), &callback_info, &error);
	assert(error == UBJF_NO_ERROR);
	ubjf_read_next(&state, &error, &bytes);
	assert(bytes == 1);
	assert(error == UBJF_ERROR_BAD_DATA);
	ubjf_destroy_buffer_read(&state);

	ubjf_init_buffer_read(&state, NULL, 0, &callback_info, &error);
	assert(error == UBJF_MAKE_PARAM_ERROR(1));

	const char eof_data[1] = "U";
	ubjf_init_buffer_read(&state, eof_data, sizeof(eof_data), &callback_info, &error);
	assert(error == UBJF_NO_ERROR);
	ubjf_read_next(&state, &error, &bytes);
	assert(bytes == sizeof(eof_data));
	assert(error == UBJF_EOF);
	ubjf_destroy_buffer_read(&state);
}

ubjf_error test_noop_callback(size_t *noop_count)
{
	++*noop_count;
	return UBJF_NO_ERROR;
}

void test_parse_noop()
{
	const char data[18] = "NNNNNNNNNNNNNNNNNN";

	size_t count = 0;
	ubjf_read_parse_callback_info callback_info = {
			.on_noop = (ubjf_on_noop_func) test_noop_callback,
			.udata = &count,
	};
	ubjf_read_state state;
	ubjf_error error;
	size_t bytes = 0;

	ubjf_init_buffer_read(&state, data, sizeof(data), &callback_info, &error);
	assert(error == UBJF_NO_ERROR);

	for (size_t result = 0, consumed = 0; result != -1; result = ubjf_read_next(&state, &error, &consumed))
		bytes += consumed;

	assert(error == UBJF_EOF);
	assert(bytes == sizeof(data));
	assert(count == sizeof(data));

	ubjf_destroy_buffer_read(&state);
}

ubjf_error test_value_callback(ubjf_value value, ubjf_value *out_value)
{
	*out_value = value;
	return UBJF_NO_ERROR;
}

void test_bool_parse()
{
	const char data[1] = "T";
	ubjf_value out;

	ubjf_read_parse_callback_info callback_info = {
			.on_value = (ubjf_on_value_func) test_value_callback,
			.udata = &out,
	};
	ubjf_read_state state;
	ubjf_error error;
	ubjf_init_buffer_read(&state, data, sizeof(data), &callback_info, &error);
	assert(error == UBJF_NO_ERROR);

	size_t bytes;
	ubjf_read_next(&state, &error, &bytes);

	assert(bytes == sizeof(data));
	assert(error == UBJF_NO_ERROR);
	assert(out.type & UBJF_BOOL_TYPE_MASK);
	assert(out.boolean == true);

	ubjf_destroy_buffer_read(&state);
}

char *test_string_callback(int64_t size, void *) { return malloc(size); }

void test_string_parse()
{
	const char data[16] = "SI\014\00Hello, world";
	ubjf_value out;

	ubjf_read_parse_callback_info callback_info = {
			.on_value = (ubjf_on_value_func) test_value_callback,
			.on_string_alloc = test_string_callback,
			.udata = &out,
	};
	ubjf_read_state state;
	ubjf_error error;
	ubjf_init_buffer_read(&state, data, sizeof(data), &callback_info, &error);
	assert(error == UBJF_NO_ERROR);

	size_t bytes;
	ubjf_read_next(&state, &error, &bytes);

	assert(bytes == sizeof(data));
	assert(error == UBJF_NO_ERROR);
	assert(out.type == UBJF_STRING);
	assert(strncmp(out.string, data + 4, 12) == 0);

	free((void *) out.string);
	ubjf_destroy_buffer_read(&state);
}