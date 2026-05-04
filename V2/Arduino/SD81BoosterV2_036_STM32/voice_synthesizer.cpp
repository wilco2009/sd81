#include "voice_synthesizer.h"
#include "allophones.h" // Contiene los datos de los alófonos
#include <string.h>
#include <stdlib.h> // Para strtol (hexadecimal to long)
#include <Arduino.h> // Para Serial.println, si se usa para depuración
#include "dac_control.h"


// Estructura para almacenar nuestro mapeo
typedef struct {
    const char* secuencia;   // La cadena de texto de entrada (ej., "A", "...", "/PA1/")
    uint8_t longitud;        // Longitud de la secuencia
    const uint8_t* codigoHex; // CAMBIADO: Ahora es un puntero a una secuencia de uint8_t
} t_alofono;

const uint8_t* numero[] = {
    (const uint8_t[]){al_ZZ,al_EH,al_XR,al_OW,0xff},                                        // zero
    (const uint8_t[]){al_WW,al_WW,al_AX,al_AX,al_NN1,al_PA2,al_PA3,al_PA3,0xff},         // one
    (const uint8_t[]){al_TT2,al_UW2,0xff},                                                      // two
    (const uint8_t[]){al_TH,al_RR2,al_IY,0xff},                                                 // three
    (const uint8_t[]){al_FF,al_OR,0xff},                                                        // four
    (const uint8_t[]){al_FF,al_AY,al_VV,0xff},                                                  // five
    (const uint8_t[]){al_SS,al_SS,al_IH,al_PA3,al_KK2,al_SS,0xff},                              // six
    (const uint8_t[]){al_SS,al_SS,al_EH,al_VV,al_EH,al_NN1,0xff},                               // seven
    (const uint8_t[]){al_EY,al_PA3,al_TT2,0xff},                                                // eight
    (const uint8_t[]){al_NN2,al_AY,al_NN1,0xff},                                                // nine
    (const uint8_t[]){al_TT2,al_EH,al_NN1,0xff},                                                // ten
    (const uint8_t[]){al_EH,al_LL,al_EH,al_VV,al_EH,al_NN1,0xff} ,                              // eleven
    (const uint8_t[]){al_TT2,al_WW,al_EH,al_LL,al_VV,0xff},                                     // twelve
    (const uint8_t[]){al_TH,al_ER2,al_PA2,al_DD2,al_PA3,al_TT2,al_IY,al_NN1,0xff},              // thirdteen
    (const uint8_t[]){al_FF,al_OR,al_PA3,al_TT2,al_IY,al_NN1,0xff},                             // fourteen
    (const uint8_t[]){al_FF,al_IH,al_FF,al_PA3,al_TT2,al_EH,al_NN1,0xff},                       // fiveteen
    (const uint8_t[]){al_SS,al_SS,al_IH,al_PA3,al_KK2,al_SS,al_PA3,al_TT2,al_IY,al_NN1,0xff},   // sixteen
    (const uint8_t[]){al_SS,al_SS,al_EH,al_VV,al_EH,al_NN1,al_PA3,al_TT2,al_IY,al_NN1,0xff},    // seventeen
    (const uint8_t[]){al_EY,al_PA3,al_TT2,al_IY,al_NN1,0xff},                                   // eightteen
    (const uint8_t[]){al_NN2,al_AY,al_NN1,al_EH,al_PA3,al_TT2,al_IY,al_NN1,0xff}                // nineteen
};

const uint8_t* decenas[] = {
    (const uint8_t[]){al_TT2,al_WW,al_EH,al_NN1,al_PA3,al_TT2,al_IY,0xff},                  // twenty
    (const uint8_t[]){al_TH,al_ER2,al_PA3,al_TT2,al_IY ,0xff},                              // thirty
    (const uint8_t[]){al_FF,al_OR,al_PA3,al_TT2,al_IY,0xff},                                // fourty
    (const uint8_t[]){al_FF,al_IH,al_VV,al_EH,al_PA3,al_TT2,al_IY,0xff},                    // fivety
    (const uint8_t[]){al_SS,al_SS,al_IH,al_PA3,al_KK2,al_SS,al_PA3,al_TT2,al_IY,0xff},      // sixty
    (const uint8_t[]){al_SS,al_SS,al_EH,al_VV,al_EH,al_NN1,al_PA3,al_TT2,al_IY,0xff},       // seventy
    (const uint8_t[]){al_EY,al_PA3,al_TT2,al_IY,0xff},                                      // eighty
    (const uint8_t[]){al_NN2,al_AY,al_NN1,al_EH,al_PA3,al_TT2,al_IY,0xff}                   // ninety
};

const uint8_t centena[] = {al_HH2,al_AX,al_AX,al_NN1,al_PA2,al_DD2,al_RR2,al_PA2,al_DD1,0xff};
const uint8_t millar[] = {al_TH,al_AW,al_NN1,al_SS,al_SS,al_AE,al_NN1,al_PA2,al_DD1,0xff};
const uint8_t millon[] = {al_MM,al_IH,al_LL,al_YY1,al_AX,al_NN1,0xff};
const uint8_t billon[] = {al_BB2,al_BB2,al_IH,al_LL,al_YY1,al_AX,al_NN1,0xff};

const uint8_t snd_and[] = {al_AE,al_NN1,al_PA2,al_DD1,0xff};


// Alófonos de múltiples caracteres y de un solo carácter
const t_alofono alofono[] = {
    {"/PA1/", 5, (const uint8_t[]){al_PA1,0xff}}, // Uso de literal compuesto explícito
    {"/PA2/", 5, (const uint8_t[]){al_PA2,0xff}},
    {"/PA3/", 5, (const uint8_t[]){al_PA3,0xff}},
    {"/PA4/", 5, (const uint8_t[]){al_PA4,0xff}},
    {"/PA5/", 5, (const uint8_t[]){al_PA5,0xff}},
    {"/OY/",  4, (const uint8_t[]){al_OY,0xff}},
    {"/AY/",  4, (const uint8_t[]){al_AY,0xff}},
    {"/EH/",  4, (const uint8_t[]){al_EH,0xff}},
    {"/KK3/", 5, (const uint8_t[]){al_KK3,0xff}},
    {"/PP/",  4, (const uint8_t[]){al_PP,0xff}},
    {"/JH/",  4, (const uint8_t[]){al_JH,0xff}},
    {"/NN1/", 5, (const uint8_t[]){al_NN1,0xff}},
    {"/IH/",  4, (const uint8_t[]){al_IH,0xff}},
    {"/TT2/", 5, (const uint8_t[]){al_TT2,0xff}},
    {"/RR1/", 5, (const uint8_t[]){al_RR1,0xff}},
    {"/AX/",  4, (const uint8_t[]){al_AX,0xff}},
    {"/MM/",  4, (const uint8_t[]){al_MM,0xff}},
    {"/TT1/", 5, (const uint8_t[]){al_TT1,0xff}},
    {"/DH1/", 5, (const uint8_t[]){al_DH1,0xff}},
    {"/IY/",  4, (const uint8_t[]){al_IY,0xff}},
    {"/EY/",  4, (const uint8_t[]){al_EY,0xff}},
    {"/DD1/", 5, (const uint8_t[]){al_DD1,0xff}},
    {"/UW1/", 5, (const uint8_t[]){al_UW1,0xff}},
    {"/AO/",  4, (const uint8_t[]){al_AO,0xff}},
    {"/AA/",  4, (const uint8_t[]){al_AA,0xff}},
    {"/YY2/", 5, (const uint8_t[]){al_YY2,0xff}},
    {"/AE/",  4, (const uint8_t[]){al_AE,0xff}},
    {"/HH1/", 5, (const uint8_t[]){al_HH1,0xff}},
    {"/BB1/", 5, (const uint8_t[]){al_BB1,0xff}},
    {"/TH/",  4, (const uint8_t[]){al_TH,0xff}},
    {"/UH/",  4, (const uint8_t[]){al_UH,0xff}},
    {"/UW2/", 5, (const uint8_t[]){al_UW2,0xff}},
    {"/AW/",  4, (const uint8_t[]){al_AW,0xff}},
    {"/DD2/", 5, (const uint8_t[]){al_DD2,0xff}},
    {"/GG3/", 5, (const uint8_t[]){al_GG3,0xff}},
    {"/VV/",  4, (const uint8_t[]){al_VV,0xff}},
    {"/GG1/", 5, (const uint8_t[]){al_GG1,0xff}},
    {"/SH/",  4, (const uint8_t[]){al_SH,0xff}},
    {"/ZH/",  4, (const uint8_t[]){al_ZH,0xff}},
    {"/RR2/", 5, (const uint8_t[]){al_RR2,0xff}},
    {"/FF/",  4, (const uint8_t[]){al_FF,0xff}},
    {"/KK2/", 5, (const uint8_t[]){al_KK2,0xff}},
    {"/KK1/", 5, (const uint8_t[]){al_KK1,0xff}},
    {"/ZZ/",  4, (const uint8_t[]){al_ZZ,0xff}},
    {"/NG/",  4, (const uint8_t[]){al_NG,0xff}},
    {"/LL/",  4, (const uint8_t[]){al_LL,0xff}},
    {"/WW/",  4, (const uint8_t[]){al_WW,0xff}},
    {"/XR/",  4, (const uint8_t[]){al_XR,0xff}},
    {"/WH/",  4, (const uint8_t[]){al_WH,0xff}},
    {"/YY1/", 5, (const uint8_t[]){al_YY1,0xff}},
    {"/CH/",  4, (const uint8_t[]){al_CH,0xff}},
    {"/ER1/", 5, (const uint8_t[]){al_ER1,0xff}},
    {"/ER2/", 5, (const uint8_t[]){al_ER2,0xff}},
    {"/OW/",  4, (const uint8_t[]){al_OW,0xff}},
    {"/DH2/", 5, (const uint8_t[]){al_DH2,0xff}},
    {"/SS/",  4, (const uint8_t[]){al_SS,0xff}},
    {"/NN2/", 5, (const uint8_t[]){al_NN2,0xff}},
    {"/HH2/", 5, (const uint8_t[]){al_HH2,0xff}},
    {"/OR/",  4, (const uint8_t[]){al_OR,0xff}},
    {"/AR/",  4, (const uint8_t[]){al_AR,0xff}},
    {"/YR/",  4, (const uint8_t[]){al_YR,0xff}},
    {"/GG2/", 5, (const uint8_t[]){al_GG2,0xff}},
    {"/EL/",  4, (const uint8_t[]){al_EL,0xff}},
    {"/BB2/", 5, (const uint8_t[]){al_BB2,0xff}},
    {"TH",    2, (const uint8_t[]){al_DH1,0xff}},

    // Mapeos de caracteres individuales (A-Z, puntuación)
    {"A", 1, (const uint8_t[]){al_AA,0xff}},
    {"B", 1, (const uint8_t[]){al_BB1,0xff}},
    {"C", 1, (const uint8_t[]){al_KK3,0xff}},
    {"D", 1, (const uint8_t[]){al_DD1,0xff}},
    {"E", 1, (const uint8_t[]){al_EH,0xff}},
    {"F", 1, (const uint8_t[]){al_FF,0xff}},
    {"G", 1, (const uint8_t[]){al_GG1,0xff}},
    {"H", 1, (const uint8_t[]){al_HH1,0xff}},
    {"I", 1, (const uint8_t[]){al_IH,0xff}},
    {"J", 1, (const uint8_t[]){al_JH,0xff}},
    {"K", 1, (const uint8_t[]){al_KK1,0xff}},
    {"L", 1, (const uint8_t[]){al_LL,0xff}},
    {"M", 1, (const uint8_t[]){al_MM,0xff}},
    {"N", 1, (const uint8_t[]){al_KK3,0xff}}, // Nota: Se mantiene el mapeo a KK3 (0x08) según el valor original
    {"O", 1, (const uint8_t[]){al_OW,0xff}},
    {"P", 1, (const uint8_t[]){al_PP,0xff}},
    {"Q", 1, (const uint8_t[]){al_KK3,0xff}},
    {"R", 1, (const uint8_t[]){al_RR2,0xff}},
    {"S", 1, (const uint8_t[]){al_SS,0xff}},
    {"T", 1, (const uint8_t[]){al_TT1,0xff}},
    {"U", 1, (const uint8_t[]){al_AX,0xff}},
    {"V", 1, (const uint8_t[]){al_VV,0xff}},
    {"W", 1, (const uint8_t[]){al_WW,0xff}},
    {"X", 1, (const uint8_t[]){al_KK3,0xff}},
    {"Y", 1, (const uint8_t[]){al_YY1,0xff}},
    {"Z", 1, (const uint8_t[]){al_ZZ,0xff}},
    {"'", 1, (const uint8_t[]){al_PA1,0xff}},
    {" ", 1, (const uint8_t[]){al_PA4,0xff}},
    {",", 1, (const uint8_t[]){al_PA5,0xff}},
    {".", 1, (const uint8_t[]){al_PA5,al_PA5,0xff}}, // Doble pausa para el punto
    // Centinela para marcar el final de la tabla
    {NULL, 0, NULL} // Un puntero NULL indica el final de las entradas válidas
};

// --- Variables internas del sintetizador de voz ---
volatile bool voice_is_playing = false;
static const char* current_alophone_sequence_hex = NULL;
static const uint8_t* current_alophone_sequence = NULL;
static uint16_t current_alophone_index_in_sequence = 0; // Índice en la secuencia hexadecimal
static int current_alophone_data_index = 0; // Índice dentro de la muestra de alófono actual
static int current_alophone_id = -1; // ID del alófono actual (0-63)

// Contador de muestras para la interpolación de la frecuencia de muestreo
static uint32_t voice_sample_counter = 0;
static uint32_t voice_sample_period_ns = 0; // Período en nanosegundos de una muestra de AY
static uint32_t sp0256_sample_period_ns = 0; // Período en nanosegundos de una muestra de SP0256


// --- Funciones internas ---

bool stringcomp(const char* str1, char* str2, int len){
  Serial.print("Comparing "); Serial.print(str1); Serial.print(" vs ");Serial.println(str2);
  for (int i=0;i<len;i++){
    if (toupper(str1[i])!=str2[i]) {
        Serial.println("not match");
        return false;
    }
  }
  Serial.println("match");
  return true;
};

int source_index = 0;
int dest_index = 0;

void procesa_unidad(uint64_t value, uint8_t* out){
    if (value !=0) {
        Serial.print(value); Serial.println(" unidades");
        for (int j=0; numero[value][j]!=0xff;j++){
            out[dest_index] = numero[value][j];
            dest_index++;
        }
    }
}

void procesa_decena(uint64_t decena, uint8_t* out){
    if (decena !=0) {
        Serial.print(decena); Serial.println(" decenas");
        for (int j=0; decenas[decena-2][j]!=0xff;j++){
            out[dest_index] = decenas[decena-2][j];
            dest_index++;
        }
    }
}   

void procesa_centena(uint64_t value, uint8_t* out){
    if (value > 0){
        Serial.print(value); Serial.println(" centenas");
        procesa_unidad(value,out);
        out[dest_index] = al_PA5;
        dest_index++;
        for (int j=0; centena[j]!=0xff;j++){
            out[dest_index] = centena[j];
            dest_index++;
        }
    }
}   

void procesa_and(uint64_t value, uint8_t* out){
    uint64_t unidad = value % 10;
    uint64_t decena = (value % 100)/10;
    uint64_t centena = (value % 1000)/100;
    uint64_t millar = (value % 1000000)/1000;
    uint64_t millon = (value % 1000000000)/1000000;
    uint64_t billon = value/1000000000;
    if (((billon!=0) ||(millon!=0) ||(millar!=0) || (centena !=0)) && (decena!=0 || unidad !=0) ) {
        out[dest_index] = al_PA4;
        dest_index++;
        for (int j=0; snd_and[j]!=0xff;j++){
            out[dest_index] = snd_and[j];
            dest_index++;
        }
    }
}

void procesa_decena_y_unidad(uint64_t value, uint8_t* out){
    if ((value % 100) < 20){
        procesa_unidad(value % 100,out);
    }
    else {
        int unidad = value % 10;
        int decena = (value % 100)/10;
        procesa_decena(decena,out);
        procesa_unidad(unidad,out);
    }
}

void procesa_millar(uint64_t value, uint8_t* out){
    if (value > 0){
        Serial.print(value); Serial.println(" miles");
        uint64_t unidad = value % 10;
        uint64_t decena = (value % 100)/10;
        uint64_t cientos = (value % 1000)/100;
        uint64_t miles = value/1000;
        procesa_millar(miles,out);
        procesa_centena(cientos,out);
        procesa_and(value,out);
        procesa_decena_y_unidad(value,out);
        //procesa_unidad(value,out);
        out[dest_index] = al_PA4;
        dest_index++;
        for (int j=0; millar[j]!=0xff;j++){
            out[dest_index] = millar[j];
            dest_index++;
        }
    }
}   

void procesa_millon(uint64_t value, uint8_t* out){
    if (value > 0){
        Serial.print(value); Serial.println(" millones");
        uint64_t unidad = value % 10;
        uint64_t decena = (value % 100)/10;
        uint64_t cientos = (value % 1000)/100;
        uint64_t miles = (value % 1000000) /1000;
        uint64_t millones = (value % 1000000000) /1000000;
        uint64_t billones = value / 1000000000;
        Serial.print("millares "); Serial.print(miles); Serial.print(" centenas "); Serial.print(cientos); Serial.print(" decenas "); Serial.print(decena); Serial.print(" unidades "); Serial.println(unidad);
        //procesa_millar(miles,out);
        procesa_centena(cientos,out);
        procesa_and(value, out);
        procesa_decena_y_unidad(value,out);
        //procesa_unidad(value,out);
        out[dest_index] = al_PA4;
        dest_index++;
        for (int j=0; millon[j]!=0xff;j++){
            out[dest_index] = millon[j];
            dest_index++;
        }
    }
}

void procesa_billon(uint64_t value, uint8_t* out){
    if (value > 0){
        Serial.print(value); Serial.println(" millones");
        uint64_t unidad = value % 10;
        uint64_t decena = (value % 100)/10;
        uint64_t cientos = (value % 1000)/100;
        uint64_t miles = (value % 1000000) /1000;
        uint64_t millones = (value % 1000000000) /1000000;
        uint64_t billones = value / 1000000000;
        Serial.print("millares "); Serial.print(miles); Serial.print(" centenas "); Serial.print(cientos); Serial.print(" decenas "); Serial.print(decena); Serial.print(" unidades "); Serial.println(unidad);
        procesa_billon(billones,out);
        procesa_millon(millones,out);
        procesa_millar(miles,out);
        procesa_centena(cientos,out);
        procesa_and(value, out);
        procesa_decena_y_unidad(value,out);
        //procesa_unidad(value,out);
        out[dest_index] = al_PA4;
        dest_index++;
        for (int j=0; millon[j]!=0xff;j++){
            out[dest_index] = millon[j];
            dest_index++;
        }
    }
}
// convierte una secuencia de alofonos en modo texto a un array binario 
void text_to_speech(const char* text, uint8_t* out){
    source_index = 0;
    dest_index = 0;
    while (text[source_index]!=0){
        if (isDigit(text[source_index])){             
            uint64_t value = atoll(&text[source_index]); 
            Serial.print("NUMBER "); Serial.print(value);Serial.println(" detected.");
            while (isDigit(text[source_index])) source_index++;
            uint64_t unidad = value % 10;
            uint64_t decena = (value % 100)/10;
            uint64_t centena = (value % 1000)/100;
            uint64_t millares = (value % 1000000)/1000;
            uint64_t millones = (value % 1000000000) /1000000;
            uint64_t billones = value / 1000000000;
            procesa_billon(billones,out);
            procesa_millon(millones,out);
            procesa_millar(millares,out);
            procesa_centena(centena,out);
            procesa_and(value, out);
            procesa_decena_y_unidad(value,out);
        }
        else {
            bool found = false;
            for (int i=0; alofono[i].codigoHex != NULL; i++){
                if(stringcomp(text+source_index, (char*) alofono[i].secuencia,alofono[i].longitud)){
                    for (int j=0; alofono[i].codigoHex[j]!=0xff;j++){
                        out[dest_index] = alofono[i].codigoHex[j];
                        dest_index++;
                    }
                    source_index+=alofono[i].longitud;
                    found = true;
                    Serial.println(source_index);
                    break;
                }
            }
            if (!found) {
                Serial.print("Unknown char :"); Serial.println(text[source_index]);
                source_index++;
            }
        }
    }
    out[dest_index]=0xff; //terminador
    Serial.println("end conversion....");
}

// Función para decodificar un alófono hexadecimal de la cadena
static int get_next_alophone_id() {
    if (current_alophone_sequence == NULL ||current_alophone_sequence[current_alophone_index_in_sequence] == 0xff) {
        return -1; // Fin de la secuencia
    }

    long alophone_id = current_alophone_sequence[current_alophone_index_in_sequence];
    current_alophone_index_in_sequence ++; // Avanzar dos caracteres por alófono
    if (alophone_id < 0 || alophone_id >= 64) {
        Serial.print("Error: Alophone ID fuera de rango: ");
        Serial.println(alophone_id,HEX);
        return -1; // ID inválido
    }
    return (int)alophone_id;
}

// // Función para decodificar un alófono hexadecimal de la cadena
// static int get_next_alophone_id_hex() {
//     if (current_alophone_sequence_hex == NULL || current_alophone_sequence_hex[current_alophone_index_in_sequence] == '\0') {
//         return -1; // Fin de la secuencia
//     }

//     char hex_pair[3];
//     hex_pair[0] = current_alophone_sequence_hex[current_alophone_index_in_sequence];
//     hex_pair[1] = current_alophone_sequence_hex[current_alophone_index_in_sequence + 1];
//     hex_pair[2] = '\0';

//     current_alophone_index_in_sequence += 2; // Avanzar dos caracteres por alófono

//     long alophone_id = strtol(hex_pair, NULL, 16); // Convertir hexadecimal a entero
//     if (alophone_id < 0 || alophone_id >= 64) {
//         Serial.print("Error: Alophone ID fuera de rango: ");
//         Serial.println(alophone_id);
//         return -1; // ID inválido
//     }
//     return (int)alophone_id;
// }

// Inicializa el siguiente alófono para reproducción
static void load_next_alophone() {
    current_alophone_id = get_next_alophone_id();
    current_alophone_data_index = 0;

    if (current_alophone_id == -1) {
        voice_synthesizer_stop(); // No hay más alófonos, detener la reproducción
        Serial.println("Voice synthesizer finished sequence.");
    } else {
        Serial.print("Playing alophone ID: ");
        Serial.println(current_alophone_id,HEX);
    }
}

// // --- Funciones públicas ---

// void Speech_hex(const char* alophone_sequence_hex) {
//     if (alophone_sequence_hex == NULL || strlen(alophone_sequence_hex) == 0) {
//         voice_synthesizer_stop();
//         return;
//     }

//     current_alophone_sequence_hex = alophone_sequence_hex;
//     current_alophone_index_in_sequence = 0;
//     voice_sample_counter = 0; // Resetear el contador de muestras
//     voice_is_playing = true;
//     Serial.print("Starting speech: ");
//     Serial.println(alophone_sequence_hex);

//     // Calcular los períodos en nanosegundos para la interpolación
//     voice_sample_period_ns = 1000000000UL / SAMPLE_RATE;
//     sp0256_sample_period_ns = 1000000000UL / SP0256_SAMPLE_RATE;

//     load_next_alophone(); // Cargar el primer alófono
// }
// --- Funciones públicas ---

// función principal para iniciar la reproducción de voz
void Speech(const uint8_t* alophone_sequence) {
    if (alophone_sequence == NULL || alophone_sequence[0] == 0xff) {
        voice_synthesizer_stop();
        return;
    }

    current_alophone_sequence = alophone_sequence;
    current_alophone_index_in_sequence = 0;
    voice_sample_counter = 0; // Resetear el contador de muestras
    voice_is_playing = true;
    //Serial.println("Starting speech: ");
    //Serial.println(alophone_sequence, HEX);

    // Calcular los períodos en nanosegundos para la interpolación
    voice_sample_period_ns = 1000000000UL / SAMPLE_RATE;
    sp0256_sample_period_ns = 1000000000UL / SP0256_SAMPLE_RATE;

    load_next_alophone(); // Cargar el primer alófono
}

// variable que alberga la conversion de texto a binario
uint8_t alophone_sequence[1000];
// función para iniciar la reproducción de voz desde una cadena de texto
void Speech(const char* alophone_sequence_str) {
    Serial.print("Starting speech: ");
    Serial.println(alophone_sequence_str);
    text_to_speech(alophone_sequence_str, alophone_sequence);
    Speech(alophone_sequence);
}

// Esta función es llamada por la ISR. Devuelve la muestra actual del alófono.
int16_t voice_synthesizer_get_sample() {
    if (!voice_is_playing || current_alophone_id == -1) {
        return 0; // No hay voz reproduciéndose
    }

    // Interpolación para adaptar la frecuencia de muestreo del alófono (11025 Hz)
    // a la frecuencia de muestreo del DAC (44100 Hz).
    // Esto es un simple upsampling por repetición de muestra, ya que 44100 es múltiplo de 11025 (4x).
    // Para una interpolación más suave (lineal, etc.), se necesitaría más lógica.

    // A cada SAMPLE_RATE/SP0256_SAMPLE_RATE muestras del DAC, avanzamos una muestra del alófono.
    // SAMPLE_RATE / SP0256_SAMPLE_RATE = 44100 / 11025 = 4
    if (voice_sample_counter % (SAMPLE_RATE / SP0256_SAMPLE_RATE) == 0) {
        // Asegurarse de no exceder el tamaño del alófono
        if (current_alophone_data_index < allophone_sizes[current_alophone_id]) {
            // Los alófonos son de 8 bits, centrados alrededor de 0 (ej: -128 a 127).
            // Leer el valor y normalizarlo para el rango de salida.
            // La escala es de 0-255, así que restamos 128 para centrarlo alrededor de 0.
            int8_t sample_8bit = (int8_t)allophone_data_pointers[current_alophone_id][current_alophone_data_index]-0x80;
            current_alophone_data_index++;
            // Escalar para el DAC de 12 bits. Rango de salida del AY es hasta ~1600.
            // sample_8bit * VOICE_NORMALIZATION_FACTOR será el valor escalado.
            return (int16_t)(sample_8bit * VOICE_NORMALIZATION_FACTOR);
        } else {
            // Alófono actual terminado, cargar el siguiente
            load_next_alophone();
            // Si no hay más alófonos, se detendrá, de lo contrario, la primera muestra del nuevo alófono.
            if (!voice_is_playing) {
                return 0;
            }
            // Recursivamente llamar para obtener la primera muestra del nuevo alófono
            return voice_synthesizer_get_sample();
        }
    }
    // Si no es el momento de avanzar la muestra del alófono, devolver la muestra actual repetida.
    // Esto es si ya hemos avanzado en current_alophone_data_index en una llamada anterior
    // y no es el momento de cargar una nueva muestra del alófono (debido al upsampling).
    if (current_alophone_id != -1 && current_alophone_data_index > 0) {
        int8_t sample_8bit = (int8_t)allophone_data_pointers[current_alophone_id][current_alophone_data_index - 1];
        return (int16_t)(sample_8bit * VOICE_NORMALIZATION_FACTOR);
    }
    return 0;
}


void voice_synthesizer_stop() {
    voice_is_playing = false;
    current_alophone_sequence_hex = NULL;
    current_alophone_sequence = NULL;
    current_alophone_index_in_sequence = 0;
    current_alophone_data_index = 0;
    current_alophone_id = -1;
    voice_sample_counter = 0;
}

// Esta función es llamada en cada ISR para avanzar el contador de muestras de voz
void voice_synthesizer_update() {
    if (voice_is_playing) {
        voice_sample_counter++;
    }
}

// esta funcion es llamada por la rutina de interrupcion del mixer devolviendo el valor de salida del sintetizador de voz
uint16_t voice_synthesizer_isr(){
    if (voice_is_playing){
      voice_synthesizer_update();
      int16_t voice_sample = voice_synthesizer_get_sample();
      // Normalizar la muestra de voz (rango original +/-1524) a un rango más pequeño (e.g. +/- 1000)
      const float VOICE_OUTPUT_SCALING = 1000.0f / (VOICE_NORMALIZATION_FACTOR * 127.0f); // Scale voice to +/- 1000
      int16_t scaled_voice_sample = (int16_t)(voice_sample * VOICE_OUTPUT_SCALING);
    }
    else return 0;
}