module up_dn_serial_reg
(
    input [7:0] data,
    input       clk,
    input       load,
    output      serial_out
);

reg [3:0] serial_up;
reg [3:0] serial_dn;
reg [3:0] latch_dn_data;
reg latch_dn;

reg old_load;

always @(posedge clk) begin
  old_load <= load;
  latch_dn <= load & ~old_load;
  if (load & ~old_load) begin
    serial_up <= {data[7], data[5], data[3], data[1]};
    latch_dn_data <= {data[6], data[4], data[2], data[0]};
  end else begin
    serial_up <= {serial_up[2:0], 1'b0};
  end
end

always @(negedge clk) begin
  if (latch_dn) begin
    serial_dn <= latch_dn_data;
  end else begin
    serial_dn <= {serial_dn[2:0], 1'b0};
  end
end

// Dice ChatGPT que esto no se debe hacer:
assign serial_out = ~(clk ? serial_up[3] : serial_dn[3]);

//// Se hace con esto, que es un multiplexor:
//ODDR2 #(
//  .DDR_ALIGNMENT("NONE"), // no importante para este uso
//  .INIT(1'b0),            // valor inicial del registro de salida
//  .SRTYPE("SYNC")         // "SYNC" o "ASYNC"
//) ddr (
//  .Q(serial_out),
//  .C0(clk),
//  .C1(~clk),
//  .D0(serial_up[3]),
//  .D1(serial_dn[3]),
//
//  // igual estos los coge por defecto y no hace falta ponerlos
//  .CE(1'b1),
//  .R(1'b0),
//  .S(1'b0)
//);
//
endmodule
