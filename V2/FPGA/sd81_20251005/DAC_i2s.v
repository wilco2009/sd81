module i2s_tx #(
    parameter DATA_WIDTH = 16
)(
    input  wire        clk,       // reloj de 49.152 MHz
    input  wire        rst,       // reset síncrono
    input  wire [15:0] sample_l,  // muestra canal izquierdo
    input  wire [15:0] sample_r,  // muestra canal derecho
    input  wire        sample_valid, // pulso: hay nueva muestra a fs=48kHz

    // Señales I²S
    output reg         bclk,      // Bit clock (1.536 MHz)
    output reg         lrclk,     // Word select / Left-Right clock (48 kHz)
    output reg         sdata      // Datos serie
);

    // ---------------------------------------
    // Divisor de reloj para BCLK y LRCLK
    // ---------------------------------------
    localparam BCLK_DIV = 8;  // 26 MHz / 8 = 3.25 MHz (2× oversampling)
    reg [7:0] div_cnt = 0;

    always @(posedge clk) begin
        if (rst) begin
            div_cnt <= 0;
            bclk    <= 0;
        end else begin
            if (div_cnt == (BCLK_DIV/2 - 1)) begin
                bclk <= ~bclk;
                div_cnt <= 0;
            end else begin
                div_cnt <= div_cnt + 1;
            end
        end
    end

    // ---------------------------------------
    // Contadores de bits y LRCLK
    // ---------------------------------------
    reg [5:0] bit_cnt = 0;
    reg [DATA_WIDTH-1:0] shift_reg = 0;
    reg [15:0] left_reg, right_reg;

    always @(posedge bclk) begin
        if (rst) begin
            bit_cnt  <= 0;
            lrclk    <= 0;
            sdata    <= 0;
        end else begin
            if (bit_cnt == 0) begin
                // cargar nueva palabra según canal
                if (!lrclk)
                    shift_reg <= left_reg;
                else
                    shift_reg <= right_reg;
            end else begin
                shift_reg <= {shift_reg[DATA_WIDTH-2:0], 1'b0};
            end

            // MSB primero
            sdata <= shift_reg[DATA_WIDTH-1];

            // avanzar bit
            bit_cnt <= bit_cnt + 1;

            if (bit_cnt == DATA_WIDTH-1) begin
                bit_cnt <= 0;
                lrclk   <= ~lrclk; // cambiar canal cada 16 bits
            end
        end
    end

    // ---------------------------------------
    // Captura de nuevas muestras a fs=48kHz
    // ---------------------------------------
    always @(posedge clk) begin
        if (rst) begin
            left_reg  <= 16'h0000;
            right_reg <= 16'h0000;
        end else if (sample_valid) begin
            left_reg  <= sample_l;
            right_reg <= sample_r;
        end
    end

endmodule
