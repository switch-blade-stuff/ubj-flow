//
// Created by switchblade on 2022-02-25.
//

#pragma once

typedef enum
{
#ifndef UBJF_NO_SPEC_12
	/** Use UBJson spec 12 syntax. */
	UBJF_SPEC_12,
#endif
} ubjf_syntax;

typedef enum
{
	/** Return `UBJF_ERROR_HIGHP` error on high-precision values (this is the default behavior). */
	UBJF_HIGHP_THROW = 0,
	/** Skip high-precision values. */
	UBJF_HIGHP_SKIP,
	/** Parse high-precision values as strings. */
	UBJF_HIGHP_AS_STRING,
} ubjf_highp_mode;
