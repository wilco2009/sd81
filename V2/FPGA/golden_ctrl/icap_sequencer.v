module icap_sequencer (
    input wire clk,                  // Reloj del Sistema (e.g., 100MHz)
    input wire reset,                // Reset del Sistema
    input wire start_reconfig,       // Señal de inicio
    
    // Interfaz con el Lector SPI (Módulo externo)
    output wire spi_read_request,    // El Sequencer activa esta señal para pedir datos
    input wire spi_data_ready,       
    input wire [31:0] spi_data_in,   
    
    // Interfaz con el Primitivo Xilinx ICAP_SPARTAN6
    output reg icap_csib,            // Control: Chip Select (low active)
    output reg icap_rdwrb,           // Control: Read/Write (0 = Write)
    output reg [31:0] icap_i,        // Datos/Comandos que van al ICAP
    output reg reconfig_done         // Indicador de finalización
);

// --- 1. Definición de Estados de la Máquina ---
parameter S_IDLE         = 4'd0;
parameter S_DESYNC_CMD   = 4'd1;
parameter S_DESYNC_DATA  = 4'd2;
parameter S_FDRI_INIT    = 4'd3;  
parameter S_FDRI_DATA    = 4'd4;  
parameter S_IPROG_CMD    = 4'd5;  
parameter S_IPROG_DATA   = 4'd6;  
parameter S_DONE         = 4'd7;  

// --- 2. Constantes de Comandos ICAP ---
parameter BITSTREAM_SIZE_WORDS = 115200; // Asume 450KB / 4. **Ajustar al valor exacto de la Application Image**

// Escribir al Registro de Comandos (CMD), Conteo=1
parameter C_WRITE_CMD  = 32'h20003401; // CMD Register address for Spartan-6
parameter C_DESYNC_VAL = 32'h0000000D;

// Escribir al Registro FDRI (Frame Data Register Input). Tipo 2 (0x30)
parameter C_WRITE_FDRI_INIT = {24'h300080, BITSTREAM_SIZE_WORDS[12:0]}; 

parameter C_IPROG_VAL  = 32'h00000001; // Valor IPROG (Salto)
parameter C_NOOP       = 32'hFFFFFFFF; 

// --- 3. Variables de Estado (Secuenciales) y Registros Next (Combinacionales) ---
reg [3:0] state;
reg [3:0] next_state;
reg [31:0] word_counter;

// Registros 'next' para evitar múltiples drivers
reg [31:0] word_counter_next;
reg icap_csib_next;
reg icap_rdwrb_next;
reg [31:0] icap_i_next;
reg reconfig_done_next;

// La señal spi_read_request se convierte en un wire para ser conducida combinacionalmente
assign spi_read_request = (state == S_FDRI_DATA) && !spi_data_ready && (word_counter < BITSTREAM_SIZE_WORDS);

// --- 4. Lógica Secuencial (Actualiza Registros en el flanco de reloj) ---
always @(posedge clk) begin
    if (reset) begin
        state <= S_IDLE;
        word_counter <= 0;
        icap_csib <= 1;
        icap_rdwrb <= 1;
        icap_i <= C_NOOP;
        reconfig_done <= 0;
    end else begin
        state <= next_state;
        word_counter <= word_counter_next;
        
        // Actualiza las salidas registradas
        icap_csib <= icap_csib_next;
        icap_rdwrb <= icap_rdwrb_next;
        icap_i <= icap_i_next;
		  reconfig_done <= reconfig_done_next;
    end
end

// --- 5. Lógica Combinacional (Calcula el Siguiente Estado y las Salidas) ---
always @* begin
    // Valores por defecto: Asignación a los registros 'next'
    next_state = state;
    word_counter_next = word_counter;
    icap_csib_next = 1;
    icap_rdwrb_next = 1;
    icap_i_next = C_NOOP;
	 reconfig_done_next = 0;
    
    // Lógica de FSM y Cálculo de Salidas
    case (state)
        S_IDLE: begin
            if (start_reconfig) begin
                next_state = S_DESYNC_CMD;
                word_counter_next = 0;
            end
            reconfig_done_next = 0;
        end

        // 1. DESYNC
        S_DESYNC_CMD: begin
            icap_csib_next = 0;
            icap_rdwrb_next = 0;
            icap_i_next = C_WRITE_CMD; 
            next_state = S_DESYNC_DATA;
        end
        S_DESYNC_DATA: begin
            icap_csib_next = 0;
            icap_rdwrb_next = 0;
            icap_i_next = C_DESYNC_VAL; 
            next_state = S_FDRI_INIT;
        end
        
        // 2. Inicializa el Stream FDRI
        S_FDRI_INIT: begin
            icap_csib_next = 0;
            icap_rdwrb_next = 0;
            icap_i_next = C_WRITE_FDRI_INIT; // Comando con el tamaño del bitstream
            next_state = S_FDRI_DATA;
        end
        
        // 3. Bucle Principal de Envío de Datos
        S_FDRI_DATA: begin
            icap_csib_next = 0;
            icap_rdwrb_next = 0;
            
            if (word_counter < BITSTREAM_SIZE_WORDS) begin
                if (spi_data_ready) begin
                    // Dato listo: envíalo y cuenta
                    icap_i_next = spi_data_in; 
                    word_counter_next = word_counter + 1;
                    next_state = S_FDRI_DATA;
                end else begin
                    // Espera el dato (spi_read_request se activa por 'assign')
                    next_state = S_FDRI_DATA; 
                end
            end else begin
                // Todos los datos enviados
                next_state = S_IPROG_CMD;
            end
        end
        
        // 4. IPROG (El Salto)
        S_IPROG_CMD: begin
            icap_csib_next = 0;
            icap_rdwrb_next = 0;
            icap_i_next = C_WRITE_CMD; 
            next_state = S_IPROG_DATA;
        end
        S_IPROG_DATA: begin
            icap_csib_next = 0;
            icap_rdwrb_next = 0;
            icap_i_next = C_IPROG_VAL; 
            next_state = S_DONE;
        end
        
        S_DONE: begin
            // La FPGA conmutará a la Application Image
            icap_csib_next = 1;
            next_state = S_DONE;
            reconfig_done_next = 1;
        end
        
        default: next_state = S_IDLE;
    endcase
end

endmodule