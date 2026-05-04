import os

def binary_to_c_array(input_file_path, output_file_path, array_name="binaryData", bytes_per_line=16):
    """
    Convierte un archivo binario a un array de unsigned char en C.

    Args:
        input_file_path (str): La ruta al archivo binario de entrada.
        output_file_path (str): La ruta al archivo .h o .c de salida.
        array_name (str): El nombre de la variable array en C.
        bytes_per_line (int): Cuántos bytes mostrar por línea en el archivo C para legibilidad.
    """
    try:
        with open(input_file_path, 'rb') as f_in:
            binary_data = f_in.read()

        file_size = len(binary_data)

        with open(output_file_path, 'w') as f_out:
            f_out.write(f"// Generated from: {os.path.basename(input_file_path)}\n")
            f_out.write(f"// File size: {file_size} bytes\n\n")
            f_out.write(f"const unsigned char {array_name}[] = {{\n")

            for i, byte_val in enumerate(binary_data):
                if i > 0:
                    f_out.write(", ")
                if i % bytes_per_line == 0:
                    f_out.write("\n    ")
                f_out.write(f"0x{byte_val:02X}") # Formato hexadecimal (0xAB)

            f_out.write("\n};\n\n")
            f_out.write(f"const unsigned int {array_name}_size = {file_size};\n")

        print(f"Conversión exitosa de '{input_file_path}' a '{output_file_path}'.")
        print(f"Tamaño del archivo binario: {file_size} bytes.")

    except FileNotFoundError:
        print(f"Error: El archivo de entrada '{input_file_path}' no fue encontrado.")
    except Exception as e:
        print(f"Ocurrió un error: {e}")

# --- Configuración (Modifica esto según tus necesidades) ---
input_binary_file = "C:\\Users\\aleja\\Desktop\\output_sumado.bin" # Tu archivo binario de salida del script de PowerShell
output_c_header_file = "C:\\Users\\aleja\\Desktop\\my_binary_data.h" # El archivo .h que se generará
c_array_variable_name = "mySoundData" # Nombre de la variable array en el código C
bytes_per_line_in_c_array = 16 # Número de bytes por línea para que el array C sea legible

# --- Ejecutar la conversión ---
if __name__ == "__main__":
    binary_to_c_array(input_binary_file, output_c_header_file, c_array_variable_name, bytes_per_line_in_c_array)