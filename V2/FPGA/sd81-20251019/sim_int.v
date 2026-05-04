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
	input wire int_mode,	
	input wire [15:0] int_addr,
	input wire nM1,
	input wire nRD,
	input wire nMREQ,
	input wire [7:0] data_in,
	output reg [7:0] data_out,
	output wire enable_out
    );
	
	reg [2:0] state = 0;
	reg [3:0] clk_cnt = 0;
	reg [7:0] prev_inst = 0;
	reg patching = 0;
	
	assign enable_out = patching && (nRD==0) && (nMREQ==0);

	always @(posedge clk or negedge nreset) begin
		if (~nreset) begin
		end else if (enable_int) begin
			if (int_mode==0) begin
				case (state)
					0: begin // searching for the instruccion opcode fetch
						if (~nM1) begin
							state <= 1;
							patching <= 1;
							data_out <= 0; // insert first NOP
							clk_cnt <= 0;
						end
					end
					1: begin // simulating two nops = 8 clock cycles
						if (clk_cnt < 8) cnt <= cnt+1;
						else begin
							patching <= 0;
							state <= 0;
						end
					end
				endcase 
			end else begin // mode 1
				case (state)
					0: begin // searching for a single instruction
						if (~nM1) begin
							if ((prev_inst!=8'hed)&&(prev_inst!=8'hdd)&&(prev_inst!=8'hcb)&&(pred_inst!=8'hfd)) begin
								state <= 1;
								patching <= 1;
								data_out <= 8'hcd;
								clk_cnt <= 0;
							end else if (~nMREQ&~nRD) prev_inst <= data_in;
						end
					end
					
					1: begin // patching call
						if (clk_cnt < 4) cnt <= cnt+1;
						else begin
							state <= 2;
							data_out <= int_addr[7:0];
							clk_cnt <= 0;
						end
					end
					
					2: begin // patching low address
						if (clk_cnt < 3) cnt <= cnt+1;
						else begin
							state <= 3;
							data_out <= int_addr[15:8];
							clk_cnt <= 0;
						end
					end
					
					3: begin // patching high address
						if (clk_cnt < 3) cnt <= cnt+1;
						else begin
							state <= 0;
							data_out <= 0;
							clk_cnt <= 0;
							patching <= 0;
						end
					end
				endcase
				
			end
		end
	end


endmodule
