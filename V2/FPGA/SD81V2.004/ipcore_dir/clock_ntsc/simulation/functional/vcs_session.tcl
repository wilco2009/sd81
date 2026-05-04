gui_open_window Wave
gui_sg_create clock_ntsc_group
gui_list_add_group -id Wave.1 {clock_ntsc_group}
gui_sg_addsignal -group clock_ntsc_group {clock_ntsc_tb.test_phase}
gui_set_radix -radix {ascii} -signals {clock_ntsc_tb.test_phase}
gui_sg_addsignal -group clock_ntsc_group {{Input_clocks}} -divider
gui_sg_addsignal -group clock_ntsc_group {clock_ntsc_tb.CLK_IN1}
gui_sg_addsignal -group clock_ntsc_group {{Output_clocks}} -divider
gui_sg_addsignal -group clock_ntsc_group {clock_ntsc_tb.dut.clk}
gui_list_expand -id Wave.1 clock_ntsc_tb.dut.clk
gui_sg_addsignal -group clock_ntsc_group {{Status_control}} -divider
gui_sg_addsignal -group clock_ntsc_group {clock_ntsc_tb.RESET}
gui_sg_addsignal -group clock_ntsc_group {clock_ntsc_tb.LOCKED}
gui_sg_addsignal -group clock_ntsc_group {{Counters}} -divider
gui_sg_addsignal -group clock_ntsc_group {clock_ntsc_tb.COUNT}
gui_sg_addsignal -group clock_ntsc_group {clock_ntsc_tb.dut.counter}
gui_list_expand -id Wave.1 clock_ntsc_tb.dut.counter
gui_zoom -window Wave.1 -full
