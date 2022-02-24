//
// Created by switchblade on 2022-02-23.
//

#pragma once

#ifdef __linux__

#include <byteswap.h>

#elif
#ifdef _MSC_VER

#include <stdlib.h>
#define bswap_32(x) _byteswap_ulong(x)
#define bswap_64(x) _byteswap_uint64(x)

#elif defined(__APPLE__)

#include <libkern/OSByteOrder.h>
#define bswap_32(x) OSSwapInt32(x)
#define bswap_64(x) OSSwapInt64(x)

#elif defined(__sun) || defined(sun)

#include <sys/byteorder.h>
#define bswap_32(x) BSWAP_32(x)
#define bswap_64(x) BSWAP_64(x)

#elif defined(__FreeBSD__)

#include <sys/endian.h>
#define bswap_32(x) bswap32(x)
#define bswap_64(x) bswap64(x)

#elif defined(__OpenBSD__)

#include <sys/types.h>
#define bswap_32(x) swap32(x)
#define bswap_64(x) swap64(x)

#elif defined(__NetBSD__)

#include <sys/types.h>
#include <machine/bswap.h>
#if defined(__BSWAP_RENAME) && !defined(__bswap_32)
#define bswap_32(x) bswap32(x)
#define bswap_64(x) bswap64(x)
#endif

#endif

#ifndef bswap_16
#define bswap_16(value)                         \
	do {                                        \
		uint8_t *data = (uint8_t *) &(value);   \
		uint8_t temp = data[1];                 \
		data[1] = data[0];                      \
		data[0] = temp;                         \
	} while (0)
#endif

#endif

#ifndef UBJF_BIG_ENDIAN

#define FIX_ENDIANNESS_16(value) bswap_16(value)
#define FIX_ENDIANNESS_32(value) bswap_32(value)
#define FIX_ENDIANNESS_64(value) bswap_64(value)
#else
#define FIX_ENDIANNESS_16(...)
#define FIX_ENDIANNESS_32(...)
#define FIX_ENDIANNESS_64(...)
#endif
