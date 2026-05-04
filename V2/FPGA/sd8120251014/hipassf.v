module hpf (
    input clk,
    input [15:0] signal,
    output signed [15:0] filtered
);

wire signed [16:0] s_signal = $signed({1'b0, signal});
reg  signed [16:0] prev_signal = 0;
reg  signed [16:0] prev_filtered;
wire signed [16:0] sum = s_signal - prev_signal + prev_filtered;
wire signed [16:0] scaled = sum - (sum >>> 8);
assign filtered = prev_filtered[15:0];

always @(posedge clk) begin
    prev_signal <= s_signal;
    prev_filtered <= scaled;
end

endmodule