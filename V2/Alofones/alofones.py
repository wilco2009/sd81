import os
import struct

f = open('ALOFONES.DAT', 'wb')
code = open('VOICETABLE.h', 'w')
code.write("const long int alof_pos[] PROGMEM = { \n")
pos = 0
for bank in range (0xE4, 0xF0):
    for alofone in range (0x80, 0x100):
        try:
            file_name = hex(bank)[2:]+hex(alofone)[2:]+".WAV"
            fwav = open(file_name.upper(),'rb')
            b = fwav.read()
            (riff, fsize, wave, fmt, fd_len, fmt, channels, 
             rate, n1, n2, bits, chunk, size 
             ) = struct.unpack('4sI4s4sIHHIIHHII',b[:44])
            error = 0
            if riff != b"RIFF": 
                error = 1
            if wave != b"WAVE": 
                error = 2
            if fmt != 1: 
                error = 3        # PCM
            if channels != 1: 
                error = 4
            if rate != 11025 : 
                error = 5
            if bits != 8: 
                error = 6
            if error !=0: 
                print(file_name.upper()+f" Unsupported file format {error}")
                print(f'riff:{riff} wave:{wave} fmt:{fmt} channels:{channels} rate:{rate} bits:{bits}')
            size = len(b)
            #size = fwav.seek(0, os.SEEK_END)
            f.write(b)
            fwav.close()
        except FileNotFoundError:
            size = 0
        code.write(f'\t\t{pos:10},\t\t // {file_name} {size:10} bytes\n')
        pos+=size

        print(file_name.upper(), " ", size, end="\r")
code.write(f'\t\t{pos:10}\n')
code.write('};\n')
f.close()
print()
print("done")