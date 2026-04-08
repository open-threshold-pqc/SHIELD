/////////////////////////////////////////////////////////////////
//           Dilithium Decomposition Modules (Dilithium2 & 35) //
/////////////////////////////////////////////////////////////////
// Description:                                                //
//   This file implements two modules for decomposing a 23-bit //
//   input 'a', as used in the Dilithium lattice-based         //
//   signature scheme. Each module splits 'a' into two parts:  //
//   - a high-order part (a1), and                             //
//   - a low-order part (a0),                                  //
//   which are concatenated to produce the output 'r'.         //
//                                                             //
// Modules:                                                    //
//                                                             //
//  1. Dilithium2_decompose                                    //
//     - Input : 23-bit value 'a'                              //
//     - Output: 29-bit value 'r' as {a1[7:0], a0[22:0]}       //
//         • a1: 6-bit clamped high-order part                 //
//         • a0: 23 least significant bits of adjusted result  //
//		   ** Note: a0 is in Z_Q and not negative			   //
//     - Function:                                             //
//         • Zero-extends 'a' to 32 bits                       //
//         • Uses fixed-point rounding to compute a1_unclamped //
//         • Clamps a1 to a maximum of 43                      //
//         • Computes a0 = a - 2*GAMMA2*a1, wraps if negative  //
//                                                             //
//  2. Dilithium35_decompose                                   //
//     - Input : 23-bit value 'a'                              //
//     - Output: 27-bit value 'r' as {a1[3:0], a0[22:0]}       //
//         • a1: 4-bit truncated high-order part               //
//         • a0: 23 least significant bits of adjusted result  //
//		   ** Note: a0 is in Z_Q and not negative			   //
//     - Function:                                             //
//         • Zero-extends 'a' to 32 bits                       //
//         • Computes a1 with different scaling & rounding     //
//         • Applies 4-bit mask to a1 (a1 = a1_full[3:0])      //
//         • Computes a0 = a - 2*GAMMA2*a1, wraps if negative  //
//                                                             //
// Parameters:                                                 //
//   - Q      : 8380417 (23-bit prime modulus)                 //
//   - GAMMA2 :                                                //
//       • Dilithium2  =  95232 (17-bit constant)              //
//       • Dilithium35 = 261888 (19-bit constant)              //
//                                                             //
// Notes:                                                      //
//   - All operations use 32-bit logic internally              //
//   - Output ordering is MSB-first: a1 in higher bits         //
//   - This implementation follows the Dilithium spec closely  //
//                                                             //
// Author: Kiarash Sedghi (kiarashs@usf.edu)                   //
// Date  : June 2025                                           //
/////////////////////////////////////////////////////////////////

module dilithium2_decompose (
input  wire [22:0] a,
output wire [28:0] r  // 6 bits for a1_clamped, 23 bits for a0
);
localparam integer Q      = 32'd8380417;
    localparam integer GAMMA2 = 32'd95232;

wire [31:0] a_ext = {9'd0, a};  // Extend to 32 bits for internal math
    wire [31:0] rounding_intermediate;
    wire [7:0]  a1_unclamped;
    wire [5:0]  a1_clamped;  // now only 6 bits!
    wire [31:0] prod, a0_temp, a0_full;
    wire [22:0] a0;

    // Compute a1 before clamping
    assign rounding_intermediate = (((a_ext + 32'd127) >> 7) * 32'd11275) + 32'd8388608;
assign a1_unclamped = rounding_intermediate >> 24;

// Clamp to max 43 (stays within 6 bits)
wire signed [31:0] diff = 32'sd43 - $signed({24'd0, a1_unclamped});
wire signed [31:0] mask = diff >>> 31;
wire [7:0] clamped_full = a1_unclamped ^ (mask[7:0] & a1_unclamped);
assign a1_clamped = clamped_full[5:0];  // explicitly slice lower 6 bits

// Compute a0
assign prod     = a1_clamped * (2 * GAMMA2);
assign a0_temp  = a_ext - prod;
assign a0_full  = a0_temp + (a0_temp[31] ? Q : 32'd0);
    assign a0       = a0_full[22:0];

    assign r = {a1_clamped, a0};  // total 6 + 23 = 29 bits
endmodule



module dilithium35_decompose (
    input  wire [22:0] a,
    output wire [26:0] r  // 4 bits for a1, 23 bits for a0
);
    // Parameters
    localparam integer Q      = 32'd8380417;
localparam integer GAMMA2 = 32'd261888;

    // Intermediate wires
    wire [31:0] a_ext = {9'd0, a};  // Zero-extend input to 32 bits
wire [31:0] rounding_intermediate;
wire [7:0]  a1_full;  // intermediate before masking
wire [3:0]  a1;       // only 4 bits used
wire [31:0] prod, a0_temp, a0_full;
wire [22:0] a0;       // 23-bit truncated a0

// Compute unclamped a1 before masking
assign rounding_intermediate = (((a_ext + 32'd127) >> 7) * 32'd1025) + 32'd2097152;
    assign a1_full = rounding_intermediate >> 22;

    // Clamp to 4 bits: equivalent to `a1 &= 15` in C
    assign a1 = a1_full[3:0];

    // Compute a0
    assign prod     = a1 * (2 * GAMMA2);
    assign a0_temp  = a_ext - prod;
    assign a0_full  = a0_temp + (a0_temp[31] ? Q : 32'd0);
assign a0       = a0_full[22:0];

// Final result: 4-bit a1 and 23-bit a0
assign r = {a1, a0};

endmodule

