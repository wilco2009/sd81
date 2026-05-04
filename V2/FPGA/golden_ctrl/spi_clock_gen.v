module spi_clock_gen (
    input wire clk,             // Reloj del sistema (e.g., 100 MHz)
    input wire reset,           // Reset del sistema
    input wire spi_clk_en,      // Habilitación general del reloj (controlado por la FSM)
    
    output reg spi_sck,         // Reloj SPI de baja frecuencia
    output wire spi_cycle_en    // Pulso de un ciclo de clock SPI (para la lógica de bits)
);

// Define la frecuencia deseada para el SPI. 
// CLK_DIV_MAX = 25 -> Divisor de 50 -> 2 MHz SPI clock (Asumiendo 100MHz CLK)
parameter CLK_DIV_MAX = 25; 
localparam N_BITS = 5; 

reg [N_BITS-1:0] clk_counter;

// Lógica de generación de spi_sck
always @(posedge clk) begin
    if (reset) begin
        clk_counter <= 0;
        spi_sck <= 0;
    end else if (spi_clk_en) begin
        if (clk_counter == (CLK_DIV_MAX - 1)) begin
            spi_sck <= ~spi_sck;
            clk_counter <= 0;
        end else begin
            clk_counter <= clk_counter + 1;
        end
    end else begin
        // Si no está habilitado, detenemos el reloj y reseteamos el contador
        spi_sck <= 0;
        clk_counter <= 0;
    end
end

// spi_cycle_en: Pulso en el flanco ascendente del spi_sck (o al inicio del ciclo)
assign spi_cycle_en = spi_clk_en && (clk_counter == 0) && (spi_sck == 1);

endmodule