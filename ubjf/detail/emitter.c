//
// Created by switchblade on 2022-03-03.
//

#include "write.h"

typedef struct
{
	ubjf_error (*const write_value)(ubjf_write_state *, ubjf_value);
	ubjf_error (*const start_container)(ubjf_write_state *, ubjf_container_info);
	ubjf_error (*const write_object_key)(ubjf_write_state *, ubjf_string);
	ubjf_error (*const end_container)(ubjf_write_state *);
} ubjf_emitter_vtable;

#ifndef UBJF_NO_SPEC_12
ubjf_error ubjf_s12_write_value(ubjf_write_state *, ubjf_value);
ubjf_error ubjf_s12_start_container(ubjf_write_state *, ubjf_container_info);
ubjf_error ubjf_s12_write_object_key(ubjf_write_state *, ubjf_string);
ubjf_error ubjf_s12_end_container(ubjf_write_state *);
#endif

static const ubjf_emitter_vtable ubjf_emitter_syntax_table[] = {
#ifndef UBJF_NO_SPEC_12
		[UBJF_SPEC_12] = {
				.write_value = ubjf_s12_write_value,
				.start_container = ubjf_s12_start_container,
				.write_object_key = ubjf_s12_write_object_key,
				.end_container = ubjf_s12_end_container,
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
#ifndef UBJF_DISABLE_CHECKS
	if (UBJF_UNLIKELY(!state))
		return UBJF_MAKE_PARAM_ERROR(0);
	else if (UBJF_UNLIKELY(!data))
		return UBJF_MAKE_PARAM_ERROR(1);
	else if (UBJF_UNLIKELY(n < 0))
		return UBJF_MAKE_PARAM_ERROR(2);
#endif

	ubjf_container_info info = {
			.container_type = UBJF_ARRAY,
			.length = n,
			.value_type = value_type,
	};

	ubjf_error result = ubjf_start_container(state, info);
	if (UBJF_UNLIKELY(result != UBJF_NO_ERROR))
		return result;

	for (int64_t i = 0; i < n; ++i)
	{
		result = ubjf_write_value(state, data[i]);
		if (UBJF_UNLIKELY(result != UBJF_NO_ERROR))
			return result;
	}

	return ubjf_end_container(state);
}