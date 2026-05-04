module shift_module (
    input clk,
    input [7:0] data,
    input load,
    output serial_out
);

reg [7:0] value;
reg  old_load;

always @(posedge clk) begin
    old_load <= load;
    if (~old_load && load)
        value <= data;
    else
        value <= {value[6:0], 1'b0};
end

assign serial_out = ~value[7];

endmodule
