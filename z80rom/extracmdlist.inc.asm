		db	.I,.N + $80
		dw	CmdIN

		db	.O,.U,.T + $80
		dw	CmdOUT

		db	.L,.D,.I,.R + $80
		dw	CmdLDIR

		db	.L,.D,.D,.R + $80
		dw	CmdLDDR

		db	.I,.N,.V + $80
		dw	CmdINV

		db	.B,.O,.L,.D + $80
		dw	CmdBOLD

		db	.H,.E,.X + $80
		dw	CmdHEX
