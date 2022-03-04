//
// Created by switchblade on 2022-03-03.
//

#include "../../read.h"

typedef ubjf_error (*ubjf_parser_func)(ubjf_read_state *restrict, size_t *restrict);

#ifndef UBJF_NO_SPEC_12
ubjf_error ubjf_s12_read_next(ubjf_read_state *restrict, size_t *restrict);
#endif

static const ubjf_parser_func ubjf_parser_syntax_table[] = {
#ifndef UBJF_NO_SPEC_12
		[UBJF_SPEC_12] = ubjf_s12_read_next,
#endif
};

ubjf_error ubjf_read_next(ubjf_read_state *state, size_t *nodes)
{
	return ubjf_parser_syntax_table[state->syntax](state, nodes);
}