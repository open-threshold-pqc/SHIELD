/////////////////////////////////////////////////////////////////
//                        Polly_Challenge                      //
/////////////////////////////////////////////////////////////////
// Module Name  : dilithium_polly_challenge                    //
// Description  :                                              //
//   Top-level module that integrates index selection and      //
//   coefficient assignment based on 136-byte input data.      //
//                                                             //
//   Functionality:                                            //
//     - Accepts 1088-bit input (136 bytes) as `data_in`.      //
//     - Uses bits [TAU-1:0] as a sign mask.                   //
//     - Selects TAU 8-bit indices via the `index_selector`.   //
//     - Updates N coefficients using `coeff_updater`,         //
//       where each coefficient is 2 bits.                     //
//     - Output is a flat 2*N-bit vector of coefficients.      //
//                                                             //
//   Encoding Summary:										   //
//	   In Z_Q, 0, 1, and -1 are represented using Q, Q+1, and  //
//	   Q-1 which only differ in lower 2-bits. Hence, instead of//
//	   returning 23-bits for each coefficient, we encode them: //
//     Values {0, +1, -1} are encoded in Z_Q using 2 bits:     //
//       - 0  → 01                                             //
//       - +1 → 10                                             //
//       - -1 → 00                                             //
//                                                             //
// Ports:                                                      //
//   Inputs:  - data_in    : [8*136-1:0]                       //
//   Outputs: - coeffs_out : [2*N-1:0]                         //
//                                                             //
// Author:     Kiarash Sedghi (kiarashs@usf.edu)               //
// Date:       June 2025                                       //
/////////////////////////////////////////////////////////////////

module dilithium2_polly_challenge #(
parameter integer TAU = 39,
parameter integer N   = 256
) (
input  wire [8*136-1:0] data_in,
output wire [2*N-1:0]   coeffs_out
);

polly_challenge #(
.N(N),
.TAU(TAU)
) core (
.data_in(data_in),
.coeffs_out(coeffs_out)
);

endmodule

module dilithium3_polly_challenge #(
parameter integer TAU = 49,
parameter integer N   = 256
) (
input  wire [8*136-1:0] data_in,
output wire [2*N-1:0]   coeffs_out
);

polly_challenge #(
.N(N),
.TAU(TAU)
) core (
.data_in(data_in),
.coeffs_out(coeffs_out)
);

endmodule

module dilithium5_polly_challenge #(
parameter integer TAU = 60,
parameter integer N   = 256
) (
input  wire [8*136-1:0] data_in,
output wire [2*N-1:0]   coeffs_out
);

polly_challenge #(
.N(N),
.TAU(TAU)
) core (
.data_in(data_in),
.coeffs_out(coeffs_out)
);

endmodule

/////////////////////////////////////////////////////////////////
//////////////////////// Polly Challenge ////////////////////////
/////////////////////////////////////////////////////////////////

module polly_challenge #(
parameter integer N   = 256,
parameter integer TAU = 39
) (
input  wire [8*136-1:0] data_in,
output reg  [2*N-1:0]   coeffs_out  // N coefficients × 2 bits
);

wire [8*TAU-1:0] index_out;
wire [2*N-1:0]   coeffs_flat;
wire [TAU-1:0]   sign_mask = data_in[TAU-1:0];

index_selector #(.N(N),.TAU(TAU) ) idx_stage (
.data_in(data_in),
.index_out(index_out)
);

coeff_updater #(.N(N), .TAU(TAU)) coeff_stage (
.index_out(index_out),
.sign_mask(sign_mask),
.coeffs_flat(coeffs_flat)
);

always @(*) begin
coeffs_out = coeffs_flat;
end

endmodule


/////////////////////////////////////////////////////////////////
////////////////////// Coefficient Updater //////////////////////
/////////////////////////////////////////////////////////////////

module coeff_updater #(
parameter integer N,
parameter integer TAU
) (
input  wire [8*TAU-1:0] index_out,
input  wire [TAU-1:0]   sign_mask,
output reg  [2*N-1:0]   coeffs_flat
);

integer i, j;
reg [7:0]  index_in;
reg        sign_bit;
reg [1:0]  new_val;
reg [1:0]  val_b;

always @(*) begin
coeffs_flat = {N{2'b01}};  // Initialize all to 2'b01

for (i = 0; i < TAU; i = i + 1) begin
index_in = index_out[i*8 +: 8];
sign_bit = sign_mask[i];
new_val  = sign_bit ? 2'b00 : 2'b10;
val_b    = coeffs_flat[index_in * 2 +: 2];

for (j = 0; j < N; j = j + 1) begin
if (j == index_in)
coeffs_flat[j*2 +: 2] = new_val;
else if (j == (N - TAU + i) && index_in != (N - TAU + i))
coeffs_flat[j*2 +: 2] = val_b;
end
end
end
endmodule

/////////////////////////////////////////////////////////////////
//////////////////////// INDEX SELECTION ////////////////////////
/////////////////////////////////////////////////////////////////

module index_selector #(
parameter integer N,
parameter integer TAU
) (
input  wire [8*136-1:0]      data_in,
output reg  [8*TAU-1:0]      index_out
);

reg [135:0] used;
reg [7:0]   last_used;
integer i, j;

reg [7:0] b_val;
reg [7:0] selected_val;
reg [7:0] next_used;
reg       found;

always @(*) begin
used = 136'b0;
        last_used = 8'd7;
index_out = 0;

for (i = 0; i < TAU; i = i + 1) begin
selected_val = 8'd0;
            next_used    = last_used;
            found        = 1'b0;

for (j = 8; j < 136; j = j + 1) begin
b_val = data_in[j*8 +: 8];
if (!used[j] && b_val < (N - TAU + 1 + i) && !found && j > last_used) begin
selected_val = b_val;
used[j]      = 1'b1;
                    next_used    = j[7:0];
                    found        = 1'b1;
end
end

index_out[i*8 +: 8] = selected_val;
last_used = next_used;
end
end

endmodule
