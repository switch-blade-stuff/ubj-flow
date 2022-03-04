//
// Created by switchblade on 2022-03-03.
//

#include "../../write.h"

typedef struct
{
	ubjf_error (*const write_value)(ubjf_write_state *, ubjf_value);
	ubjf_error (*const start_container)(ubjf_write_state *, ubjf_container_info);
	ubjf_error (*const write_object_key)(ubjf_write_state *, ubjf_string);
	ubjf_error (*const end_container)(ubjf_write_state *);
	ubjf_error (*const write_array)(ubjf_write_state *restrict, const ubjf_value *restrict, int64_t, ubjf_type);
} ubjf_emitter_vtable;

#ifndef UBJF_NO_SPEC_12
ubjf_error ubjf_s12_write_value(ubjf_write_state *, ubjf_value);
ubjf_error ubjf_s12_start_container(ubjf_write_state *, ubjf_container_info);
ubjf_error ubjf_s12_write_object_key(ubjf_write_state *, ubjf_string);
ubjf_error ubjf_s12_end_container(ubjf_write_state *);
ubjf_error ubjf_s12_write_array(ubjf_write_state *restrict, const ubjf_value *restrict, int64_t, ubjf_type);
#endif

static const ubjf_emitter_vtable ubjf_emitter_syntax_table[] = {
#ifndef UBJF_NO_SPEC_12
		[UBJF_SPEC_12] = {
				.write_value = ubjf_s12_write_value,
				.start_container = ubjf_s12_start_container,
				.write_object_key = ubjf_s12_write_object_key,
				.end_container = ubjf_s12_end_container,
				.write_array = ubjf_s12_write_array,
		},
#endif
};

ubjf_error ubjf_write_value(ubjf_write_state *state, ubjf_value value)
{
	return ubjf_emitter_syntax_table[state->syntax].write_value(state, value);
}
ubjf_error ubjf_start_container(ubjf_write_state *state, ubjf_container_info info)
{
	return ubjf_emitter_syntax_table[state->syntax].start_container(state, info);
}
ubjf_error ubjf_write_object_key(ubjf_write_state *state, ubjf_string key)
{
	return ubjf_emitter_syntax_table[state->syntax].write_object_key(state, key);
}
ubjf_error ubjf_end_container(ubjf_write_state *state)
{
	return ubjf_emitter_syntax_table[state->syntax].end_container(state);
}
ubjf_error ubjf_write_array(ubjf_write_state *state, const ubjf_value *data, int64_t n, ubjf_type value_type)
{
	return ubjf_emitter_syntax_table[state->syntax].write_array(state, data, n, value_type);
}