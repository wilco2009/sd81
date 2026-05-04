module golden_ctrl(
    input wire CLK100,
    input wire nRESET,
    output wire spi_cs_n,
    output wire spi_sck,
    output wire spi_mosi,
    input wire spi_miso
);
    
    // Conexiones internas
    wire start_reconfig = 1'b1; // Inicialización: Asumimos que comienza automáticamente (o usa una señal de entrada)
    wire spi_read_request; 
    wire spi_data_ready;							
    wire [31:0] spi_data_in;   				
    wire icap_csib;
    wire icap_rdwrb;
    wire [31:0] icap_i;
    wire reconfig_done; // Señal de finalización del proceso

    // Usamos el mismo wire para entrada y salida de datos del reader
    // (Asegúrate que el spi_flash_reader usa spi_data_out para su salida)
    wire [31:0] spi_data_out; 
    assign spi_data_in = spi_data_out;


    // --- 1. INSTANCIA: ICAP SEQUENCER (FSM) ---
    icap_sequencer ICAP_seq_inst (
        .clk(CLK100),                  
        .reset(!nRESET),                
        .start_reconfig(start_reconfig),
        
        // Interfaz SPI/ICAP
        .spi_read_request(spi_read_request),  
        .spi_data_ready(spi_data_ready),  		
        .spi_data_in(spi_data_in),   			
        
        // Interfaz al Primitivo
        .icap_csib(icap_csib),            
        .icap_rdwrb(icap_rdwrb),          
        .icap_i(icap_i),                   
        .reconfig_done(reconfig_done)     
    );


    // --- 2. INSTANCIA: PRIMITIVO XILINX ICAP_SPARTAN6 (CORREGIDO) ---
    ICAP_SPARTAN6 ICAP_INST (
        .CLK   (CLK100),
        .CE  (icap_csib),                    
        .WRITE (icap_rdwrb),                   
        .I     (icap_i),                       
        // Conexiones de salida obligatorias para evitar el error 267
        .O     (),      
        .BUSY  ()      
    );

    // --- 3. INSTANCIA: SPI FLASH READER ---
    spi_flash_reader spi_flash_reader_inst(
        .clk(CLK100),                  
        .reset(!nRESET),                
        
        // Interfaz al Sequencer
        .spi_read_request(spi_read_request),  
        .spi_data_ready(spi_data_ready),      
        .spi_data_out(spi_data_out),  			
        
        // Interfaz a los pines físicos
        .spi_cs_n(spi_cs_n),             
        .spi_sck(spi_sck),              
        .spi_mosi(spi_mosi),             
        .spi_miso(spi_miso)             
    );

endmodule