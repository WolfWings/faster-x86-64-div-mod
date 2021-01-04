This library is meant to be used as a series of very thin shim functions, which each
fit inside a separate 64-byte cacheline on x86_64/AMD64 platforms.

The 'magic factors' are kept as separate 64-bit hi/lo values to allow direct address
calculation as the stepping stays at *8 which AMD64 directly supports without adding
separate address calculations.

This is based on a combination of several concepts:
 * The 'magic(' through 'magicu2(' chapter of Hacker's Delight (2nd Edition) dealing
   with signed and unsigned division by constants.
 * Direct use of the fixed-register d:a forms of the mul/div instructions using more
   knowledge of the specifics to avoid the #DE exception risk on div and utilize the
   full 128-bit width on muls directly.
 * The 'fastmod' header by Daniel Lemire for doing quick 32-bit operations on 64-bit
   platforms, extended here to 64-bit division on 64-bit platforms with the previous
   referenced tools and techniques.

Much of 'magic' and the 'addone' issues goes away with the "( m * b ) + b" construct
used here, as it allows for all factors >= 1, with a 64-bit shift, to stay inside of
128-bit values in total. Without that construct the magic value for 1 requires a 65-
bit shift, for example.

For 32-bit math this sort of juggling is less needed, but for 64-bit values (such as
using 64-bit PRNG's and avoiding/reducing modulo biases) if a dynamic value is being
used 3 times or more this approach is faster. An unsigned 64-bit division takes over
10 times as long, 64-bit operations take between 70-150 clock cycles for division vs
5 clock cycles each for multiplication.

Calculating the 'magic values' takes two in-order divisions (so 200-300 cycles), and
using them requires 2-4 multiplications to use (so 10-20 clock cycles), versus using
integer division directly which takes 75-150 clock cycles per.

Worst-case this technique wins when you use it the fifth time:
  * ( 300 / 75 ) = 4 divisions breaks even for creating the constants, every time is
    a win for this technique from that point onwards.

Best-case is even more favorable, with the second use becoming a win:
  * ( 200 / 150 ) = 2 divisions is a win already.

In reality the 'victory' point varies a bit, but generally if you need a constant at
least three times this is often a win to use it dynamically. If it is a compile-time
constant? It's a win 100% of the time then.

The 'unused' argument is to avoid the DX register in the System V AMD64 ABI to allow
for better optimization by avoiding pointless moves and decoding bandwidth.
