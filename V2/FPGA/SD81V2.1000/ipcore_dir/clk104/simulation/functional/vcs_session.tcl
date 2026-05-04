gui_open_window Wave
gui_sg_create clk104_group
gui_list_add_group -id Wave.1 {clk104_group}
gui_sg_addsignal -group clk104_group {clk104_tb.test_phase}
gui_set_radix -radix {ascii} -signals {clk104_tb.test_phase}
gui_sg_addsignal -group clk104_group {{Input_clocks}} -divider
gui_sg_addsignal -group clk104_group {clk104_tb.CLK_IN1}
gui_sg_addsignal -group clk104_group {{Output_clocks}} -divider
gui_sg_addsignal -group clk104_group {clk104_tb.dut.clk}
gui_list_expand -id Wave.1 clk104_tb.dut.clk
gui_sg_addsignal -group clk104_group {{Status_control}} -divider
gui_sg_addsignal -group clk104_group {clk104_tb.RESET}
gui_sg_addsignal -group clk104_group {clk104_tb.LOCKED}
gui_sg_addsignal -group clk104_group {{Counters}} -divider
gui_sg_addsignal -group clk104_group {clk104_tb.COUNT}
gui_sg_addsignal -group clk104_group {clk104_tb.dut.counter}
gui_list_expand -id Wave.1 clk104_tb.dut.counter
gui_zoom -window Wave.1 -full
