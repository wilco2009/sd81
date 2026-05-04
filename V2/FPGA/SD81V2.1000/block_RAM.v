`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date:    15:57:20 08/17/2025 
// Design Name: 
// Module Name:    attr_RAM 
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
module block_RAM#(
    parameter DATA_WIDTH = 8,
    parameter ADDR_WIDTH = 14
)(	 
    input clk,
    input we,
    input [ADDR_WIDTH-1:0] addr,
    input [DATA_WIDTH-1:0] din,
    output reg [DATA_WIDTH-1:0] dout
    );

    // 1. Calcular la profundidad de la RAM
    localparam RAM_DEPTH = 1 << ADDR_WIDTH;

    // 2. Declarar la memoria como un array de registros
    reg [DATA_WIDTH-1:0] my_ram [0:RAM_DEPTH-1];
initial begin
    my_ram[14848] = 8'h05;
    my_ram[14849] = 8'h06;
    my_ram[14850] = 8'h07;
    my_ram[14851] = 8'h08;
end
    // 3. Describir el comportamiento síncrono
    always @(posedge clk) begin
        // Operación de escritura
        if (we) begin
            my_ram[addr] <= din;
        end
        
        // Operación de lectura síncrona
        // La salida se registra para un mejor rendimiento
        dout <= my_ram[addr];
    end
endmodule
