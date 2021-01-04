#include <stdint.h>

// This is primarilly the "Faster Remainder by Direct Computation" technique, see
// here for deeper details: https://arxiv.org/abs/1902.01961
uint64_t fast_modulo(
	const uint64_t magic_hi
,	const uint64_t magic_lo
,	__attribute__((unused)) uint64_t unused
,	const uint64_t base
,	const uint64_t divisor
) {
	__uint128_t magic = ( (__uint128_t)( magic_hi ) << 64 ) | magic_lo;
        __uint128_t scaled = ( magic * base ) + base;
        __uint128_t half_lo = ( divisor * ( scaled & 0xffffffffffffffff ) ) >> 64;
        __uint128_t half_hi =   divisor * ( scaled                          >> 64 );
        __uint128_t merged = half_lo + half_hi;
        merged >>= 64;
        return (uint64_t) merged;
}
