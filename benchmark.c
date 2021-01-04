#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <x86intrin.h>
#include <sys/mman.h>
#include <linux/mman.h>

#include "division.h"

static int mmapped = 1;

static const uint64_t constant_factor = 0x81fb832f0120fde0;
//static const uint64_t constant_factor = 0x81fb832f;
static uint64_t rnd;
static uint64_t temp_hi;
static uint64_t temp_lo;
static uint64_t timer_start;

#define SIZE 0x40000000
#define STEPS 0x8000000
#define PROTS PROT_READ | PROT_WRITE
#define FLAGS MAP_SHARED | MAP_ANONYMOUS

#define ROTL(d,lrot) ((d<<(lrot)) | (d>>(8*sizeof(d)-(lrot))))

//===== RomuQuad ==================================================================================
//
// More robust than anyone could need, but uses more registers than RomuTrio.
// Est. capacity >= 2^90 bytes. Register pressure = 8 (high). State size = 256 bits.

static uint64_t wState, xState, yState, zState;  // set to nonzero seed

static uint64_t inline romuQuad_random () {
   uint64_t wp = wState, xp = xState, yp = yState, zp = zState;
   wState = 15241094284759029579u * zp; // a-mult
   xState = zp + ROTL(wp,52);           // b-rotl, c-add
   yState = yp - xp;                    // d-sub
   zState = yp + wp;                    // e-add
   zState = ROTL(zState,19);            // f-rotl
   return xp;
}

static void inline timer_begin( const char *message ) {
	_rdseed64_step( (long long unsigned int *)&rnd );
	rnd |= 0x8000000000000000;
	calc_magic( &temp_hi, &temp_lo, 0, rnd );

	asm volatile( "# timer_begin" : : : );
	fprintf( stderr, "\t\t\t\t\t\t%s\r", message );
	timer_start = _rdtsc();
}

static void inline timer_finish( void ) {
	fprintf( stderr, "%20llu clock cycles taken.\n", _rdtsc() - timer_start );
}

int main( void ) {
	register uint64_t *buffer;
	register uint64_t random_factor;
	uint64_t magic_hi;
	uint64_t magic_lo;
	for (;;) {
		fprintf( stderr, "Requesting 1 * 1GB huge page...\n" );
		buffer = mmap( NULL, SIZE, PROTS, FLAGS | MAP_HUGETLB | MAP_HUGE_1GB, -1, 0 );
		if ( buffer != MAP_FAILED ) {
			break;
		}

		fprintf( stderr, "Unable to allocate 1GB huge page, requesting 512 * 2MB huge pages...\n" );
		buffer = mmap( NULL, SIZE, PROTS, FLAGS | MAP_HUGETLB | MAP_HUGE_2MB, -1, 0 );
		if ( buffer != MAP_FAILED ) {
			break;
		}

		fprintf( stderr, "Unable to allocate 512 * 2MB huge pages, requesting 262144 * 4KB pages...\n" );
		buffer = mmap( NULL, SIZE, PROTS, FLAGS, -1, 0 );
		if ( buffer != MAP_FAILED ) {
			break;
		}

		mmapped = 0;

		fprintf( stderr, "Unable to mmap contiguous 1GB space, falling back to malloc.\n" );
		buffer = malloc( SIZE );
		if ( buffer != NULL ) {
			break;
		}

		fprintf( stderr, "Unable to allocate 1GB buffer. Aborting.\n" );
		exit( -1 );
	}

	_rdseed64_step( (long long unsigned int *)&wState );
	_rdseed64_step( (long long unsigned int *)&xState );
	_rdseed64_step( (long long unsigned int *)&yState );
	_rdseed64_step( (long long unsigned int *)&zState );

	timer_begin( "Filling buffer with random data." );
	for ( int i = STEPS; i != 0; i-- ) {
		buffer[i - 1] = romuQuad_random();
	}
	timer_finish();

	timer_begin( "Raw memory scan." );
	for ( int i = STEPS; i != 0; i-- ) {
		asm volatile( "" : : "r" ( buffer[i - 1] ) : "memory" );
	}
	timer_finish();

	timer_begin( "Division by a random value." );
	random_factor = rnd;
	for ( int i = STEPS; i != 0; i-- ) {
		asm volatile( "" : : "r" ( buffer[i - 1] / rnd ) : "memory" );
	}
	timer_finish();

	timer_begin( "Division by a compile-time constant." );
	for ( int i = STEPS; i != 0; i-- ) {
		asm volatile( "" : : "r" ( buffer[i - 1] / constant_factor ) : "memory" );
	}
	timer_finish();

	timer_begin( "Division using fast_division." );
	magic_hi = temp_hi;
	magic_lo = temp_lo;
	for ( int i = STEPS; i != 0; i-- ) {
		asm volatile( "" : : "r" ( fast_division( magic_hi, magic_lo, 0, buffer[i - 1] ) ) : "memory" );
	}
	timer_finish();

	timer_begin( "Remainder of a random value." );
	random_factor = rnd;
	for ( int i = STEPS; i != 0; i-- ) {
		asm volatile( "" : : "r" ( buffer[i - 1] % random_factor ) : "memory" );
	}
	timer_finish();

	timer_begin( "Remainder of a compile-time constant." );
	for ( int i = STEPS; i != 0; i-- ) {
		asm volatile( "" : : "r" ( buffer[i - 1] % constant_factor ) : "memory" );
	}
	timer_finish();

	timer_begin( "Remainder using fast_modulo." );
	random_factor = rnd;
	magic_hi = temp_hi;
	magic_lo = temp_lo;
	for ( int i = STEPS; i != 0; i-- ) {
		asm volatile( "" : : "r" ( fast_modulo( magic_hi, magic_lo, 0, buffer[i - 1], random_factor ) ) : "memory" );
	}
	timer_finish();

	timer_begin( "Divisibility test using random value." );
	random_factor = rnd;
	for ( int i = STEPS; i != 0; i-- ) {
		asm volatile( "" : : "r" ( ( buffer[i - 1] % random_factor ) == 0 ) : "memory" );
	}
	timer_finish();

	timer_begin( "Divisibility test using compile-time constant." );
	for ( int i = STEPS; i != 0; i-- ) {
		asm volatile( "" : : "r" ( ( buffer[i - 1] % constant_factor ) == 0 ) : "memory" );
	}
	timer_finish();

	timer_begin( "Divisibility test using fast_divisible." );
	magic_hi = temp_hi;
	magic_lo = temp_lo;
	for ( int i = STEPS; i != 0; i-- ) {
		asm volatile( "" : : "r" ( fast_divisible( magic_hi, magic_lo, 0, buffer[i - 1] ) ) : "memory" );
	}
	timer_finish();

	if ( mmapped == 1 ) {
		munmap( buffer, SIZE );
	} else {
		free( buffer );
	}

	return 0;
}
