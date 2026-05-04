avrdude -Cavrdude.conf -v -patmega2560 -cstk500v1 -PCOM3 -b19200 -Uflash:w:stk500boot_v2_mega2560.hex:i -Ulock:w:0x0F:m 
