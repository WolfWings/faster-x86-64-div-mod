#include <stdint.h>

void calc_magic( uint64_t *magic_hi, uint64_t *magic_lo, const uint64_t unused, const uint64_t divisor );
uint8_t fast_divisible( const uint64_t magic_hi, const uint64_t magic_lo, const uint64_t unused, const uint64_t base );
uint64_t fast_division( const uint64_t magic_hi, const uint64_t magic_lo, const uint64_t unused, const uint64_t base );
uint64_t fast_modulo( const uint64_t magic_hi, const uint64_t magic_lo, const uint64_t unused, const uint64_t base, const uint64_t divisor );
