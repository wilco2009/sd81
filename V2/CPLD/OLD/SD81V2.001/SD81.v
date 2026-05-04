`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    23:01:17 12/26/2022 
// Design Name: 
// Module Name:    SD81 
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
module SD81(
	 // Z80 Addr lines
	 input A0,
    input A1,
    input A2,
    input A3,
    input A4,
    input A5,
    input A6,
    input A7,
    input A8,
    input A9,
    input A10,
    input A11,
    input A12,
    input A13,
    input A14,
    input A15,
	 // Memory Addr lines
	 output A0x,
    output A1x,
    output A2x,
    output A3x,
    output A4x,
    output A5x,
    output A6x,
    output A7x,
    output A8x,
    output A9x,
    output A10x,
    output A11x,
    output A12x,
    output A13x,
    output A14x,
    output A15x,
    output A16x,
    output A17x,
    output A18x,
    output nWRx,
	 // DATA BUS
	 inout D0,		
	 inout D1,
	 inout D2,
	 inout D3,
	 inout D4,
	 inout D5,
	 inout D6,
	 inout D7,
	 // OTHER Z80 SIGNALS
	 input nCLOCK,
    input nIORQ,
    input nRD,
    input nWR,
    input nMREQ,
    input nM1,
    input nRFSH,
	 input nRESET,
	 input nWAIT,
	 input nBUSRQ,
	 input nBUSAK,
	 input nHALT,
	 input nNMI,
	 
	 // COMMS SIGNALS
    output GET_CTRL_REG,		// New control info ready on latch
    input nRST_CTRL_REG,			// Ctrl info readed 
    output GET_DATA_REG,		// New data ready on latch
    input nRST_DATA_REG,			// Data readed
    output nOE_CTRL_CLOCK,		// Output Enable CTRL Clock Latch
    output nOE_IL,					// Output Enable Input Latch
    output LE_OL,					// Latch Enable Output Latch
    output nMEM_CE,			// Chip Enable External MEM
	 output nMEM_OE,			// Output Enable External MEM
    output nOE_RAMCS,				// Output Enable RAMCS latch
    output nOE_ROMCS,				// Output Enable ROMCS latch
	 input ROM_WR_EN,			// ROM write enabled active high
	 input SEL_128CHARS,
	 input FULL_PAGING,				// Upper/lower half selector
	 output SPARE1,
	 output SPARE2,
	 output SPARE3
    );
	 
// ***************************************************
//		MEMORY MAPPER
// ***************************************************
		wire mapper_port_wr = A7&A6&A5&~A4&~A3&A2&A1&A0&~nIORQ&~nWR;
		wire mapper_port_rd = A7&A6&A5&~A4&~A3&A2&A1&A0&~nIORQ&~nRD;
	 
		reg [5:0] block[0:7];
		reg [2:0] rowcnt;
		reg [6:0] ram_data_latch;
		
		// mapper port
		always @(posedge mapper_port_wr or negedge nRESET) 
		begin
			if (nRESET==1'b0) 
			begin
				block[0] <= 6'd0;
				block[1] <= 6'd1;
				block[2] <= 6'd2;
				block[3] <= 6'd3;
				block[4] <= 6'd4;
				block[5] <= 6'd5;
				block[6] <= 6'd2;
				block[7] <= 6'd3;
			end else begin
				block[{D2,D1,D0}] <= FULL_PAGING ? {A13,A12,A11,A10,A9,A8} : {0,D7,D6,D5,D4,D3};
			end
		end
		
		assign {D7,D6,D5,D4,D3,D2,D1,D0} = mapper_port_rd? block[{A10,A9,A8}] : 8'bzzzzzzzz;
// ************************************************
// 	CHAR GENERATOR
// ************************************************
		wire row_rst;
		wire sync;
		reg sync_clr;
		reg IC19B;
		reg IC18A;
		
		wire kbd_n = nIORQ | nRD | A0;
		reg vs,old_hsync,hsync,NMIlatch;
		reg [7:0] sync_counter;
		wire sync_reset = ~(~nM1 & ~nIORQ);
		always @(posedge nCLOCK or negedge sync_reset) begin

			if (!sync_reset) begin
				 sync_counter <= 7'b0;
			end else begin
			 
				old_hsync <= hsync;
				if (~old_hsync & hsync)  rowcnt <= rowcnt + 1'd1;
				if (vs) rowcnt <= 0;
				if (~nIORQ & ~nWR & ~NMIlatch) vs <= 0;
				if (~kbd_n & ~NMIlatch)        vs <= 1;
				if (~nIORQ & ~nWR & (A0 ^ A1)) NMIlatch <= A1; else NMIlatch <= 0;
					
				sync_counter <= sync_counter + 1'd1;
				if (sync_counter == 206) sync_counter <= 0;
				if(sync_counter == 15)  hsync <= 1;
				if(sync_counter == 31)  hsync <= 0;
			end
		end
		assign ram_data_latch_LE = nRD;
		
//		wire row_rst;
//		wire sync;
//		reg sync_clr;
//		reg IC19B;
//		reg IC18A;
//		
//		wire kbd_n = nIORQ | nRD | A0;
//		reg vs,old_hsync,hsync,NMIlatch;
//		reg [7:0] sync_counter;
//		wire sync_reset = ~(~nM1 & ~nIORQ);
//		always @(posedge nCLOCK or negedge sync_reset) begin
//
//			if (!sync_reset) begin
//				  hsync <= 1'b0;
//				 sync_counter <= 7'b0;
//			end else begin
//			 
//				old_hsync <= hsync;
//				if (~old_hsync & hsync)  rowcnt <= rowcnt + 1'd1;
//				if (vs) rowcnt <= 0;
//				if (~nIORQ & ~nWR & ~NMIlatch) vs <= 0;
//				if (~kbd_n & ~NMIlatch)        vs <= 1;
//				if (~nIORQ & ~nWR & (A0 ^ A1)) NMIlatch <= A1; else NMIlatch <= 0;
//					
//				sync_counter <= sync_counter + 1'd1;
//				if (sync_counter == 206) sync_counter <= 0;
//				if(sync_counter == 15)  hsync <= 1;
//				if(sync_counter == 31)  hsync <= 0;
//			end
//		end
//		assign ram_data_latch_LE = ~nMREQ & nCLOCK & nRFSH;

		always @(posedge ram_data_latch_LE or negedge nRESET)
		begin
			if (nRESET==1'b0) 
			begin
				ram_data_latch <= 7'b0;
			end else begin
				ram_data_latch <= {D7,D5,D4,D3,D2,D1,D0};
			end
		end
		
		assign SPARE1 = rowcnt[0];
		assign SPARE2 = rowcnt[1];
		assign SPARE3 = rowcnt[2];
		
// ************************************************
// 	ADDRESSING
// ************************************************
		// lines are directly connected except for the case /nRESET==0
		assign {A12x,A11x,A10x} = ~nRESET?3'bzzz:{A12,A11,A10};
		assign {A9x,A8x,A7x,A6x,A5x,A4x,A3x,A2x,A1x,A0x} = ~nRESET?10'bzzzzzzzzzz:(nRFSH | A14 | A15)?{A9,A8,A7,A6,A5,A4,A3,A2,A1,A0}:SEL_128CHARS?{ram_data_latch[6:0],rowcnt[2:0]}:{A9,ram_data_latch[5:0],rowcnt[2:0]};
		assign {A18x,A17x,A16x,A15x,A14x,A13x} = ~nRESET?6'bzzzzzz:block[{A15,A14,A13}];
		
		// LOW ROM is write protected
		assign nWRx = (~nRESET?1'bz: (ROM_WR_EN? (nWR | nMREQ) :(nWR | nMREQ | (~A13&~A14&~A15))));
		
		// Internal RAM always inactive
		assign nOE_RAMCS = 1'b0; //~nRESET?1'bz:1'b1; // Output enable internal RAMCS latch
		//external MEM active for RAM and ROM
		assign nMEM_OE = ~nRESET?1'bz:nRD&nRFSH;		// external MEM always active
		assign nMEM_CE = ~nRESET?1'bz:nMREQ&nRFSH;	// external MEM always active
		assign nOE_ROMCS = nMEM_CE; // Output enable internal ROMCS latch

// ************************************************
// 	COMMS WITH MICROCONTROLER
// ************************************************
 
		reg DATA_REG = 1'b0;			// Data LATCH ready
		reg DATA_REG_in = 1'b0;
		reg CTRL_REG = 1'b0;			// CTRL LATCH ready
		wire port_CTRL_REG;			// Control I/O Port
		wire port_DATA_REG;			// Data I/O Port
		wire DATA_in_out;				// in/out 0xA7 detected
		
		wire DATA_out;					// out 0xA7 detected
	   wire DATA_in;					// in 0xA7 detected
		 
		wire CTRL_out;					// out 0xAF detected
		wire CTRL_in;					// in 0xA7 detected
			 
		assign port_CTRL_REG = A7 & ~A6 & A5 & ~A4 & A3 & A2 & A1 & ~nIORQ ;  	// 0xAF
		assign port_DATA_REG = A7 & ~A6 & A5 & ~A4 & ~A3 & A2 & A1 & ~nIORQ ;  	// 0xA7
		
		assign DATA_out = port_DATA_REG  & ~nWR;
		assign DATA_in  = port_DATA_REG  & ~nRD;
		assign DATA_in_out = DATA_in | DATA_out;
		 
		assign CTRL_out = port_CTRL_REG  & ~nWR;
		assign CTRL_in  = port_CTRL_REG  & ~nRD;

		// D Reg
		// Reset = nRST_CTRL_REG
		// CLK =  A7 & A5 & A2 & A1 & !nWR & !IORQ & !A6 & !A4 & A3
		// D = Vcc
		always @(posedge CTRL_out or negedge nRST_CTRL_REG) begin  
			if (nRST_CTRL_REG==1'b0)
				CTRL_REG = 1'b0;
			else
				CTRL_REG = 1'b1;
		end 

		// D Reg
		// Reset = nRST_DATA_REG
		// CLK =  A7 & A5 & A2 & A1 & !nWR & !IORQ & !A6 & !A4 & !A3
		// D = Vcc
		always @(posedge DATA_in_out or negedge nRST_DATA_REG) begin
			if (nRST_DATA_REG==1'b0)
				DATA_REG = 1'b0;
			else
				DATA_REG = 1'b1;
		end
			
		always @(posedge DATA_in or negedge nRST_DATA_REG) begin
			if (nRST_DATA_REG==1'b0)
				DATA_REG_in = 1'b0;
			else
				DATA_REG_in = 1'b1;
		end

		assign LE_OL = DATA_out; 
		assign nOE_IL = ~DATA_in;
		
		assign nOE_CTRL_CLOCK = ~CTRL_in; // Enable D7 if in 0xA7 detected
		
		assign GET_CTRL_REG = CTRL_REG;
		
		assign GET_DATA_REG = DATA_REG;
		
endmodule
