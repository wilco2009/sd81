`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    13:32:40 10/03/2025 
// Design Name: 
// Module Name:    beeper 
// Project Name: 
// Target Devices: 
// Tool versions: 
// Description: 
//
// Dependencies: 
//
// Revision: 
// Revision 0.01 - File Created
// Additional Comments: 
//
//////////////////////////////////////////////////////////////////////////////////
module beeper(
	input clk,
	input nreset,
	input wire cs,
	input wire [1:0] data_in,
	output wire signed [12:0] data_out
    );
	
	reg signed [12:0] beeper_table [0:3];
	initial begin
		beeper_table[0] = 0; // -13'sd2048;
		beeper_table[1] = 0; //-13'sd2048;
		beeper_table[2] = 3000; //13'sd1000;
		beeper_table[3] = 4095; //13'sd2048;
	end
	
	parameter delta=2;
	
	reg signed [12:0] outreg;
	reg [1:0] old_linear;
	reg [1:0] linear;
	always@(posedge clk) begin
	 if (~nreset) outreg <= 0;
//	 else outreg <= beeper_table[data_in];
	 old_linear <= linear;
	 if (cs) linear <= data_in;  // o {data_in[1], data_in[0]} o lo que corresponda
	 if (linear != old_linear) outreg <= {beeper_table[linear]};
	 else begin
		if (outreg > 0) outreg <= outreg - delta; 
		else if (outreg < 0) outreg <= outreg + delta;    
	 end
	end
	
	assign data_out = outreg;

endmodule
