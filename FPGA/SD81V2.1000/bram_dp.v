module bram_dp #(
    parameter ADDR_WIDTH = 3,   // 2^3 = 8 posiciones
    parameter DATA_WIDTH = 14
) (
    input                   clk_a,
    input                   we_a,
    input  [ADDR_WIDTH-1:0] addr_a,
    input  [DATA_WIDTH-1:0] din_a,
    output reg [DATA_WIDTH-1:0] dout_a,

    input                   clk_b,
    input                   we_b,
    input  [ADDR_WIDTH-1:0] addr_b,
    input  [DATA_WIDTH-1:0] din_b,
    output reg [DATA_WIDTH-1:0] dout_b
);

    // Memoria interna
    reg [DATA_WIDTH-1:0] mem [0:(1<<ADDR_WIDTH)-1];

    // Puerto A
    always @(posedge clk_a) begin
        if (we_a)
            mem[addr_a] <= din_a;
        dout_a <= mem[addr_a];
    end

    // Puerto B
    always @(posedge clk_b) begin
        if (we_b)
            mem[addr_b] <= din_b;
        dout_b <= mem[addr_b];
    end

endmodule
