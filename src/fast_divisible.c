#include <stdint.h>

// This is the more subtle but often more useful 'trick' that Daniel Lemire documented:
//
// Their direct-remainder computation can be short-circuited to this degenerate case if
// you only care about 'divisibility' which means that the modulo/remainder is zero.
//
// This often has applications in random selection (I.E. Inside Out Fisher-Yates but if
// only tracking the "top card" to fairly get one of an unknown number of choices), and
// being entirely branchless and only two multiplications makes it very fast.
uint8_t fast_divisible(
	const uint64_t magic_hi
,	const uint64_t magic_lo
,	__attribute__((unused)) uint64_t unused
,	const uint64_t base
) {
	__uint128_t magic = ( (__uint128_t)( magic_hi ) << 64 ) | magic_lo;
	return ( ( magic * base ) + base ) < magic;
}
