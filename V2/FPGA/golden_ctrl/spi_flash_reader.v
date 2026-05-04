module spi_flash_reader (
    input wire clk,                  // Reloj del sistema (e.g., 100MHz)
    input wire reset,                // Reset del sistema
    
    // Interfaz con el icap_sequencer
    input wire spi_read_request,     // Petición para el siguiente dato de 32 bits
    output reg spi_data_ready,       // Señal al Sequencer: Dato de 32 bits listo
    output reg [31:0] spi_data_out,  // Palabra de 32 bits del Bitstream
    
    // Interfaz con los pines de la Flash SPI
    output reg spi_cs_n,             // Chip Select (Bajo activo)
    output wire spi_sck,             // Reloj SPI (DRIVEN BY spi_clock_gen)
    output reg spi_mosi,             // Master Out Slave In
    input wire spi_miso              // Master In Slave Out
);

// --- 1. Constantes y Parámetros ---
parameter START_ADDRESS = 32'h00080000;
parameter CMD_FAST_READ = 8'h0B;
parameter DUMMY_CYCLES  = 8;
parameter ADDR_BITS     = 24;
parameter CMD_BITS      = 8;

// --- 2. Máquina de Estados SPI ---
parameter S_IDLE        = 3'd0;
parameter S_CMD_TX      = 3'd1;
parameter S_ADDR_TX     = 3'd2;
parameter S_DUMMY       = 3'd3;
parameter S_READ_STREAM = 3'd4;

// --- 3. Registros de Estado (Secuencial) ---
reg [2:0] state;
reg [31:0] word_address;         
reg [7:0] tx_data_reg;           
reg [7:0] bit_counter;           
reg [31:0] shift_reg_rx;         
reg spi_clk_en;                  // Controla el generador de reloj

// --- 4. Registros 'NEXT' (Combinacional) ---
reg [2:0] next_state;
reg [7:0] bit_counter_next;
reg [31:0] word_address_next;
reg [31:0] spi_data_out_next;    // **CORRECCIÓN FINAL**
reg spi_cs_n_next;
reg spi_mosi_next;
reg spi_data_ready_next;
reg spi_clk_en_next;

// --- 5. Interfaz con spi_clock_gen ---
wire spi_cycle_en;               // Pulso de habilitación para cada ciclo SPI

// -------------------------------------------------------------------------
// I. INSTANCIACIÓN DEL GENERADOR DE RELOJ SPI
// -------------------------------------------------------------------------

spi_clock_gen CLK_GEN_INST (
    .clk(clk),
    .reset(reset),
    .spi_clk_en(spi_clk_en),
    .spi_sck(spi_sck),
    .spi_cycle_en(spi_cycle_en)
);


// -------------------------------------------------------------------------
// II. Lógica Secuencial (Actualización de Registros)
// -------------------------------------------------------------------------

always @(posedge clk) begin
    if (reset) begin
        // --- Inicialización de variables de estado ---
        state <= S_IDLE;
        bit_counter <= 0;
        word_address <= START_ADDRESS;
        tx_data_reg <= 8'h0;
        
        // --- Inicialización de salidas y control interno ---
        spi_cs_n <= 1'b1;
        spi_mosi <= 1'b0;
        spi_data_ready <= 1'b0;
        spi_data_out <= 32'h0;
        spi_clk_en <= 1'b0;
    end else begin
        // --- Actualización de Registros NEXT ---
        state <= next_state;
        bit_counter <= bit_counter_next;
        word_address <= word_address_next;
        
        // --- Actualización de salidas y control interno ---
        spi_cs_n <= spi_cs_n_next;
        spi_data_ready <= spi_data_ready_next;
        spi_data_out <= spi_data_out_next;      // **USANDO EL NEXT**
        spi_clk_en <= spi_clk_en_next;

        // --- Lógica de Shift Registers (Sincronizada con spi_cycle_en) ---
        if (spi_clk_en && spi_cycle_en) begin
            if (state == S_CMD_TX) begin
                tx_data_reg <= {tx_data_reg[6:0], 1'b0};
            end
            
            if (state == S_DUMMY || state == S_READ_STREAM) begin
                shift_reg_rx <= {shift_reg_rx[30:0], spi_miso};
            end
        end
    end
end


// -------------------------------------------------------------------------
// III. Lógica Combinacional (Cálculo de Next State y Control)
// -------------------------------------------------------------------------

always @* begin
    // --- Valores por defecto (Mantenidos) ---
    next_state = state;
    bit_counter_next = bit_counter;
    word_address_next = word_address;
    spi_data_out_next = spi_data_out;       // **DEFECTO: MANTENER VALOR**
    
    // --- Control de Salidas (Mantener el valor anterior) ---
    spi_cs_n_next = spi_cs_n;
    spi_mosi_next = spi_mosi;
    spi_data_ready_next = 1'b0;
    spi_clk_en_next = spi_clk_en;

    case (state)
        S_IDLE: begin
            if (spi_read_request) begin
                spi_cs_n_next = 1'b0;
                spi_clk_en_next = 1'b1;
                tx_data_reg = CMD_FAST_READ;
                bit_counter_next = CMD_BITS;
                next_state = S_CMD_TX;
            end
        end

        // 1. Envío de Comando
        S_CMD_TX: begin
            spi_mosi_next = tx_data_reg[7]; 
            if (spi_cycle_en) begin
                bit_counter_next = bit_counter - 1;
                if (bit_counter == 1) begin 
                    bit_counter_next = ADDR_BITS;
                    next_state = S_ADDR_TX;
                end
            end
        end
        
        // 2. Envío de Dirección (24 bits)
        S_ADDR_TX: begin
            // Enviar el MSB de la dirección (bit 23)
            spi_mosi_next = word_address[23]; 

            if (spi_cycle_en) begin
                // Desplazar el registro de dirección para exponer el siguiente bit
                word_address_next = {word_address[22:0], 1'b0}; 
                bit_counter_next = bit_counter - 1;

                if (bit_counter == 1) begin
                    bit_counter_next = DUMMY_CYCLES;
                    spi_mosi_next = 1'b0;
                    next_state = S_DUMMY;
                end
            end
        end

        // 3. Ciclos Dummy
        S_DUMMY: begin
            spi_mosi_next = 1'b0; 
            if (spi_cycle_en) begin
                bit_counter_next = bit_counter - 1;

                if (bit_counter == 1) begin
                    bit_counter_next = 32;
                    next_state = S_READ_STREAM;
                end
            end
        end

        // 4. Lectura de Datos (Stream)
        S_READ_STREAM: begin
            spi_mosi_next = 1'b0; 
            
            if (spi_cycle_en) begin
                bit_counter_next = bit_counter - 1;

                if (bit_counter == 1) begin 
                    spi_data_out_next = shift_reg_rx;   // **ACTUALIZA EL NEXT**
                    spi_data_ready_next = 1'b1;         // Dato listo

                    if (spi_read_request) begin
                        bit_counter_next = 32; 
                        next_state = S_READ_STREAM;
                    end else begin
                        spi_clk_en_next = 1'b0; 
                        next_state = S_READ_STREAM;
                    end
                end
            end else if (!spi_clk_en && spi_read_request) begin
                spi_clk_en_next = 1'b1;
                next_state = S_READ_STREAM;
            end
        end

        default: next_state = S_IDLE;
    endcase
end

endmodule