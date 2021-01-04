#include <stdint.h>

// This is mostly the Hacker's Delight "Unsigned Division" technique, but
// expanded out to using 128-bit magic factors to handle 64-bit division,
// but optimized for x86_64/AMD64 to fit in very few registers as well.
uint64_t fast_division(
	const uint64_t magic_hi
,	const uint64_t magic_lo
,	__attribute__((unused)) uint64_t unused
,	const uint64_t base
) {
        __uint128_t half_lo = ( ( (__uint128_t)magic_lo ) * base ) + base;
        __uint128_t half_hi = ( ( (__uint128_t)magic_hi ) * base ) + base;
        __uint128_t merged = half_lo + half_hi;
        merged >>= 64;
        return (uint64_t) merged;
}
