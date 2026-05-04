`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    23:36:41 10/09/2025 
// Design Name: 
// Module Name:    sim_int 
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
module sim_int(
	input wire clk,
	input wire nreset,
	input wire enable_int,
	input wire disable_int,
	input wire vsync,
	input wire [15:0] int_addr,
	input wire nM1,
	input wire nRD,
	input wire nMREQ,
	input wire nHALT,
	input wire [7:0] data_in,
	output reg [7:0] data_out,
	output reg enable_out,
	output reg [2:0] state,
	output reg enabled
    );
	
//	reg [2:0] state = 0;
	reg [3:0] clk_cnt = 0;
	reg [7:0] prev_inst = 0;
	reg [7:0] prev_inst_read;
	
	reg old_m1rd = 0;
	reg old_rd = 0;
	reg old_vsync = 0;
	wire m1rd = ~(nMREQ|nRD|nM1);
	wire rd = ~(nMREQ|nRD);
	wire m1flange = ~m1rd && old_m1rd;
	wire rdflange = ~rd && old_rd;
	wire vsyncflange = vsync && ~old_vsync;

	always @(posedge clk or negedge nreset) begin
		if (~nreset) begin
			state <= 0;
			old_m1rd <= 0;
			old_rd <= 0;
			enable_out <= 0;
		end else begin
			old_m1rd <= m1rd;
			old_rd <= rd;
			old_vsync <= vsync;
			if (enable_int) enabled <= 1'b1;
			if (disable_int) enabled <= 1'b0;
			if (m1rd) prev_inst_read <= data_in;
			if (m1flange) prev_inst <= prev_inst_read;
			case (state)
				0:	if (vsyncflange && enabled && nHALT) state <= 1; 					// waiting for a int
				1: case (prev_inst)
						8'h76: if (m1flange) state <= 0;
						8'hDD,8'hFD: if (m1flange) state <= 2;
						8'hCB,8'hED: if (m1flange) state <= 3;
						default: begin
							if (m1rd) begin
								data_out <= 8'hCD;			// insert a call
								enable_out <= 1'b1;
							end else if (m1flange) begin
								enable_out <= 1'b0;
								state <= 4;
							end
						end
					endcase
				2: case (prev_inst)							// DD or FD opcode
						8'hED: if (m1flange) state <= 3;
						8'hDD,8'hFD: if (m1flange) state <= 2;
						default: if (m1flange) state <= 1;
					endcase
				3: if (m1flange) state <= 1;				// drop an opcode
				4:begin											// insert low call address
					if (rdflange) begin
						enable_out <= 1'b0;
						state <= 5;
					end 
					if (rd) begin
						data_out <= int_addr[7:0];
						enable_out <= 1'b1;
					end
				end
				5:begin											// insert high call address
					if (rdflange) begin
						enable_out <= 1'b0;
						state <= 0;
						enabled <= 0;
					end 
					if (rd) begin
						data_out <= int_addr[15:8];
						enable_out <= 1'b1;
					end
				end
			endcase
		end
	end


endmodule
