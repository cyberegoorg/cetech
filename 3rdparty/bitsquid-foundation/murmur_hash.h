#pragma once

#include "types.h"

namespace foundation
{
	/// Implementation of the 64 bit MurmurHash2 function
	/// http://murmurhash.googlepages.com/
	uint64_t murmur_hash_64(const void *key, uint32_t len, uint64_t seed);
}