#include <stdint.h>

// Instead of trying to minimize the shift factor to shrink the magic value we're going
// to target a single fixed 64-bit shift which requires a 128-bit factor to work with.
//
// Also this factor is always one lower than actually required, so it requires m*v+v to
// use it, instead of simply m*v. This is to avoid an edge case with the factor of 1 so
// it can be used for fully branchless code.
void calc_magic(
	uint64_t *magic_hi
,	uint64_t *magic_lo
,	__attribute__((unused)) uint64_t unused
,	const uint64_t divisor
) {
	uint64_t temp;
	uint64_t rem; // Exists to simplify inline assembly and avoid compiler warnings

	// This first step guarantees that ( divident_hi:divident_lo / divisor ) < 2^64
	// This allows directly using the 'unrestricted' 128-bit unsigned divide x86_64
	// CPU instruction without risking the #DE "division by zero" CPU exception.
	*magic_hi = ( (uint64_t) -1 ) / divisor;
	temp      = ( (uint64_t) -1 ) % divisor;

	__asm__(	"divq %[divisor]"
		:	"=a" ( *magic_lo ), "=d"( rem )
		:	[ divisor ] "r"( divisor ), "d"( temp ), "a"( ( (uint64_t) -1 ) )
		);
}
