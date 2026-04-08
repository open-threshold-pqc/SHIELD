/////////////////////////////////////////////////////////////////////////////
// Module: Dilithium_modular_addition
// Description: Performs modular addition of two 23-bit inputs using the
//              CRYSTALS-Dilithium modulus q = 8380417 (23'h7FFFFB1).
// Inputs:
//   - a: 23-bit unsigned input operand
//   - b: 23-bit unsigned input operand
// Output:
//   - result: (a + b) mod q
// Usage:
//	 - daBits
// Author:     Kiarash Sedghi (kiarashs@usf.edu)
// Date:       Feb 2025
/////////////////////////////////////////////////////////////////////////////

module Dilithium_modular_addition (
input  [22:0] a,
input  [22:0] b,
output [22:0] result
);

// Dilithium modulus q = 8380417 = 0x7FFFFB1
parameter [22:0] MODULUS = 23'd8380417;

    // Use 24 bits to avoid overflow during addition
    wire [23:0] sum_full = a + b;

    // Modular reduction
    assign result = (sum_full >= MODULUS) ? (sum_full - MODULUS) : sum_full[22:0];

endmodule
