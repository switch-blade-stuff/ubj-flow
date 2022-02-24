//
// Created by switchblade on 2022-02-23.
//

#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "detail/ubjf.h"

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
	test_parse_noop_array();
	test_bool_parse();
	test_parse_bool_array();
	test_string_parse();
	test_parse_string_array();
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
	ubjf_parse_event_info event_info = {};
	ubjf_read_state state;
	ubjf_error error;
	size_t bytes;

	const char bad_data[8] = "bad data";
	ubjf_init_buffer_read(&state, bad_data, sizeof(bad_data), &event_info, &error);
	assert(error == UBJF_NO_ERROR);
	ubjf_read_next(&state, &error, &bytes);
	assert(bytes == 1);
	assert(error == UBJF_ERROR_BAD_DATA);
	ubjf_destroy_buffer_read(&state);

	ubjf_init_buffer_read(&state, NULL, 0, &event_info, &error);
	assert(error == UBJF_MAKE_PARAM_ERROR(1));

	const char eof_data[1] = "U";
	ubjf_init_buffer_read(&state, eof_data, sizeof(eof_data), &event_info, &error);
	assert(error == UBJF_NO_ERROR);
	ubjf_read_next(&state, &error, &bytes);
	assert(bytes == sizeof(eof_data));
	assert(error == UBJF_EOF);
	ubjf_destroy_buffer_read(&state);
}

struct test_array_data
{
	bool started;
	bool ended;
	size_t count;
};

ubjf_error test_noop_event(struct test_array_data *data)
{
	data->count++;
	return UBJF_NO_ERROR;
}
ubjf_error test_noop_array_start_event(ubjf_token container_type,
                                       int64_t fixed_size,
                                       ubjf_token value_type,
                                       struct test_array_data *data)
{
	data->started = true;
	assert(container_type == UBJF_ARRAY);
	assert(fixed_size != 0);
	assert(value_type == 0);
	return UBJF_NO_ERROR;
}
ubjf_error test_noop_array_end_event(struct test_array_data *data)
{
	data->ended = true;
	return UBJF_NO_ERROR;
}
void test_parse_noop_array()
{
	const char data[22] = "[#i\022NNNNNNNNNNNNNNNNNN";

	struct test_array_data event_data = {};
	ubjf_parse_event_info event_info = {
			.on_noop = (ubjf_on_noop_func) test_noop_event,
			.on_container_begin = (ubjf_on_container_begin_func) test_noop_array_start_event,
			.on_container_end = (ubjf_on_container_end_func) test_noop_array_end_event,
			.udata = &event_data,
	};
	ubjf_read_state state;
	ubjf_error error;
	size_t bytes = 0;

	ubjf_init_buffer_read(&state, data, sizeof(data), &event_info, &error);
	assert(error == UBJF_NO_ERROR);

	for (size_t result = 0, consumed = 0; result != -1; result = ubjf_read_next(&state, &error, &consumed))
		bytes += consumed;

	assert(error == UBJF_EOF);
	assert(bytes == sizeof(data));
	assert(event_data.count == sizeof(data) - 4);
	assert(event_data.started == true);
	assert(event_data.ended == true);

	ubjf_destroy_buffer_read(&state);
}

ubjf_error test_bool_event(ubjf_value value, struct test_array_data *data)
{
	assert(value.type_token & UBJF_BOOL_TYPE_MASK);
	assert(value.boolean == true);
	data->count++;
	return UBJF_NO_ERROR;
}
ubjf_error test_bool_array_start_event(ubjf_token container_type,
                                       int64_t fixed_size,
                                       ubjf_token value_type,
                                       struct test_array_data *data)
{
	data->started = true;
	assert(container_type == UBJF_ARRAY);
	assert(fixed_size != 0);
	assert(value_type == UBJF_TRUE);
	return UBJF_NO_ERROR;
}
ubjf_error test_bool_array_end_event(struct test_array_data *data)
{
	data->ended = true;
	return UBJF_NO_ERROR;
}
void test_parse_bool_array()
{
	const char data[6] = "[$T#i\022";

	struct test_array_data event_data = {};
	ubjf_parse_event_info event_info = {
			.on_value = (ubjf_on_value_func) test_bool_event,
			.on_container_begin = (ubjf_on_container_begin_func) test_bool_array_start_event,
			.on_container_end = (ubjf_on_container_end_func) test_bool_array_end_event,
			.udata = &event_data,
	};
	ubjf_read_state state;
	ubjf_error error;
	size_t bytes = 0;

	ubjf_init_buffer_read(&state, data, sizeof(data), &event_info, &error);
	assert(error == UBJF_NO_ERROR);

	for (size_t result = 0, consumed = 0; result != -1; result = ubjf_read_next(&state, &error, &consumed))
		bytes += consumed;

	assert(error == UBJF_EOF);
	assert(bytes == sizeof(data));
	assert(event_data.count == 18);
	assert(event_data.started == true);
	assert(event_data.ended == true);

	ubjf_destroy_buffer_read(&state);
}

ubjf_error test_string_array_event(ubjf_value value, struct test_array_data *data)
{
	assert(value.type_token == UBJF_STRING);
	data->count++;
	return UBJF_NO_ERROR;
}
ubjf_error test_string_array_start_event(ubjf_token container_type,
                                         int64_t fixed_size,
                                         ubjf_token value_type,
                                         struct test_array_data *data)
{
	data->started = true;
	assert(container_type == UBJF_ARRAY);
	assert(fixed_size == 0);
	assert(value_type == 0);
	return UBJF_NO_ERROR;
}
ubjf_error test_string_array_end_event(struct test_array_data *data)
{
	data->ended = true;
	return UBJF_NO_ERROR;
}
void test_parse_string_array()
{
	const char data[14] = "[Si\0Si\0Si\0Si\0]";

	struct test_array_data event_data = {};
	ubjf_parse_event_info event_info = {
			.on_value = (ubjf_on_value_func) test_string_array_event,
			.on_container_begin = (ubjf_on_container_begin_func) test_string_array_start_event,
			.on_container_end = (ubjf_on_container_end_func) test_string_array_end_event,
			.udata = &event_data,
	};
	ubjf_read_state state;
	ubjf_error error;
	size_t bytes = 0;

	ubjf_init_buffer_read(&state, data, sizeof(data), &event_info, &error);
	assert(error == UBJF_NO_ERROR);

	for (size_t result = 0, consumed = 0; result != -1; result = ubjf_read_next(&state, &error, &consumed))
		bytes += consumed;

	assert(error == UBJF_EOF);
	assert(bytes == sizeof(data));
	assert(event_data.count == 4);
	assert(event_data.started == true);
	assert(event_data.ended == true);

	ubjf_destroy_buffer_read(&state);
}


ubjf_error test_value_event(ubjf_value value, ubjf_value *out_value)
{
	*out_value = value;
	return UBJF_NO_ERROR;
}
void test_bool_parse()
{
	const char data[1] = "T";
	ubjf_value out;

	ubjf_parse_event_info event_info = {
			.on_value = (ubjf_on_value_func) test_value_event,
			.udata = &out,
	};
	ubjf_read_state state;
	ubjf_error error;
	ubjf_init_buffer_read(&state, data, sizeof(data), &event_info, &error);
	assert(error == UBJF_NO_ERROR);

	size_t bytes;
	ubjf_read_next(&state, &error, &bytes);

	assert(bytes == sizeof(data));
	assert(error == UBJF_NO_ERROR);
	assert(out.type_token & UBJF_BOOL_TYPE_MASK);
	assert(out.boolean == true);

	ubjf_destroy_buffer_read(&state);
}

char *test_string_event(int64_t size, void *) { return malloc(size); }
void test_string_parse()
{
	const char data[18] = "Sl\014\00\00\00Hello, world";
	ubjf_value out;

	ubjf_parse_event_info event_info = {
			.on_value = (ubjf_on_value_func) test_value_event,
			.on_string_alloc = test_string_event,
			.udata = &out,
	};
	ubjf_read_state state;
	ubjf_error error;
	ubjf_init_buffer_read(&state, data, sizeof(data), &event_info, &error);
	assert(error == UBJF_NO_ERROR);

	size_t bytes;
	ubjf_read_next(&state, &error, &bytes);

	assert(bytes == sizeof(data));
	assert(error == UBJF_NO_ERROR);
	assert(out.type_token == UBJF_STRING);
	assert(strncmp(out.string, data + 6, 12) == 0);

	free((void *) out.string);
	ubjf_destroy_buffer_read(&state);
}