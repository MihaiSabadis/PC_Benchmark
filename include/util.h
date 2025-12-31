#pragma once
#include <stdint.h>
static inline uint64_t rotl64(uint64_t x, int r) { return (x << r) | (x >> (64 - r)); }
static inline uint32_t rotl32(uint32_t x, int r) { return (x << r) | (x >> (32 - r)); }