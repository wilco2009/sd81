#include "RTC.h"

STM32RTC& rtc = STM32RTC::getInstance(); // Obtiene la instancia del objeto RTC

// Definir un valor "mágico" para nuestra bandera
#define RTC_INITIALIZED_FLAG_VALUE 0x32F1 // Un número aleatorio para identificarlo

// --- DEFINICIONES PARA ACCESO DIRECTO A REGISTROS DE RESPALDO (ULTIMO RECURSO) ---
// Dirección de memoria del primer registro de datos de respaldo (BKP_DR0) para STM32F4xx
#define RTC_BKP_DR0_ADDRESS ((uint32_t)0x40002850)
// Macro para acceder al valor en esa dirección de memoria (se asegura que sea volátil)
#define RTC_BACKUP_REGISTER0 (*((volatile uint32_t *)RTC_BKP_DR0_ADDRESS))

bool isTime(int hours, int minutes, int seconds, int hundredths){
  return (hours >=0) && (hours <= 23) && (minutes >=0) && (minutes <= 59) && (seconds >=0) && (seconds<=59)&&(hundredths>=0)&&(hundredths<=99);
}

bool leapYear(int year){
  // Un año es bisiesto si es divisible por 400
  // O si es divisible por 4 pero NO por 100.
  return ((year % 400 == 0) || ((year % 4 == 0) && (year % 100 != 0)));
}

int maxday(int year, int month){
  int md;
  switch (month){
    case 1:
    case 3:
    case 5:
    case 7:
    case 8:
    case 10:
    case 12: return 31;
    case 4:
    case 6:
    case 9:
    case 11: return 30;
    case 2:  return leapYear(year)?29:28;
    default: 0;
  }
}

bool isDate(int year, int month, int day){
  int md = maxday(year,month);
  return (year > 0)&&(year <100)
    &&  (month > 0)&&(month<13)
    &&  (day > 0)&&(day<=md);
}

// returns a datetime YYYY-MM-DD hh:mm:ss.cc
void rtc_get_time(char* s){
    int year_4digits = rtc.getYear()+2000;
    int month = rtc.getMonth();
    int day = rtc.getDay();
    int hours = rtc.getHours();
    int minutes = rtc.getMinutes();
    int seconds = rtc.getSeconds();
    int hundredths = rtc.getSubSeconds()/10;
    // int sub = rtc.getSubSeconds(); // 0..pre (cuenta hacia 0)
    // int hundredths = (( (pre - sub) * 100u ) + ((pre + 1u)/2u)) / (pre + 1u); // 0..99

    sprintf(s,"%04d-%02d-%02d %02d:%02d:%02d.%02d",year_4digits,month,day,hours,minutes,seconds,hundredths);
}

void Serial_print_time(void){
    int year_2digits = rtc.getYear();
    int month = rtc.getMonth();
    int day = rtc.getDay();
    int hours = rtc.getHours();
    int minutes = rtc.getMinutes();
    int seconds = rtc.getSeconds();

    int full_year = year_2digits + 2000;

    Serial.print("Date: ");
    Serial.print(day);
    Serial.print("/");
    Serial.print(month);
    Serial.print("/");
    Serial.print(full_year);
    Serial.print("  Time: ");
    Serial.print(hours);
    Serial.print(":");
    Serial.print(minutes);
    Serial.print(":");
    Serial.println(seconds);
  }


void rtc_init(void){
  log_3("Starting configuration and querying RTC date and time...\n\r");

  // Habilitar acceso al dominio de respaldo (NECESARIO para acceder a registros del RTC/reloj)
  HAL_PWR_EnableBkUpAccess();

  // --- OPCIONAL/AVANZADO: Forzar un reset del dominio de respaldo ---
  // Esto borrará todos los datos de respaldo y la configuración del RTC.
  // ÚSALO SOLO SI LOS PROBLEMAS PERSISTEN Y NECESITAS UN REINICIO COMPLETO.
  // Una vez que funcione, deberías comentar estas líneas.
  // Serial.println("Forzando reset del dominio de respaldo...");
  // __HAL_RCC_BACKUPRESET_FORCE(); // Aplica el reset
  // delay(10);
  // __HAL_RCC_BACKUPRESET_RELEASE(); // Libera el reset
  // Serial.println("Reset del dominio de respaldo completado.");

   rtc.setClockSource(STM32RTC::LSE_CLOCK);
  // Intentar iniciar el RTC. Esto debería habilitar el reloj y establecer la fuente.
  rtc.begin(); // Esto llamará a HAL_RTC_Init() internamente, que debería habilitar RTCEN.

  // --- Sección para verificar los relojes y el RTCEN usando librerías HAL ---

  uint32_t rtc_clock_source = __HAL_RCC_GET_RTC_SOURCE();

  if (rtc_clock_source == RCC_RTCCLKSOURCE_LSI) {
    log_3("Clock source configured as LSI.\n\r");
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY) != RESET) {
      log_1("LSI oscillator is active and stable.\n\r");
    } else {
      log_0("WARNING: LSI selected, but not stable.\n\r");
    }
  } else if (rtc_clock_source == RCC_RTCCLKSOURCE_LSE) {
    log_3("Clock source configured as LSE.\n\r");
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) != RESET) {
      log_3("LSE oscillator is active and stable.\n\r");
    } else {
      log_0("WARNING: LSE selected, but not stable.\n\r");
    }
  } else if (rtc_clock_source == RCC_RTCCLKSOURCE_HSE_DIV2) {
    log_0("RTC clock source configured as HSE/2.\n\r");
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) != RESET) {
      log_0("HSE oscillator is active and stable.\n\r");
    } else {
      log_0("WARNING: HSE/2 selected but not stable.\n\r");
    }
  } else {
    log_0("RTC clock source unknown or not configured.\n\r");
  }

  // Verificar si el reloj del RTC está habilitado en el dominio de respaldo (RTCEN)
  if ((RCC->BDCR & RCC_BDCR_RTCEN) != RESET) {
      log_3("RTC clock is enabled in BDCR.\n\r");
  } else {
      log_0("WARNING: RTC clock (RTCEN) not enabled. Trying to enable it...\n\r");
      __HAL_RCC_RTC_ENABLE();
      delay(10);
      if ((RCC->BDCR & RCC_BDCR_RTCEN) != RESET) {
         log_0("RTCEN successfully enabled.\n\r");
      } else {
         log_0("Failed to enable RTCEN. Critical problem.\n\r");
      }
  }

  // --- USAR EL REGISTRO DE RESPALDO COMO BANDERA (ACCESO DIRECTO A LA DIRECCIÓN) ---
  uint32_t backup_flag_read = RTC_BACKUP_REGISTER0; // Acceso directo a la dirección de memoria

  if (backup_flag_read != RTC_INITIALIZED_FLAG_VALUE) {
    log_0("RTC appears to be misconfigured (flag not found). Setting date and time...\n\r");
    
    uint8_t currentHours = 13;
    uint8_t currentMinutes = 38; // 1:38 PM [Current time is Sunday, July 20, 2025 at 1:38:06 PM CEST.]
    uint8_t currentSeconds = 0;
    uint8_t currentDay = 20;
    uint8_t currentMonth = 7;
    uint8_t currentYear = 25; // Año 2025 -> 25 (dos dígitos)

    rtc.setHours(currentHours);
    rtc.setMinutes(currentMinutes);
    rtc.setSeconds(currentSeconds);
    rtc.setTime(currentHours, currentMinutes, currentSeconds); 

    rtc.setDay(currentDay);
    rtc.setMonth(currentMonth);
    rtc.setYear(currentYear);
    rtc.setDate(currentDay, currentMonth, currentYear); 

    log_0("RTC configured a %d/%d/%d %d:%d:%d \n\r",currentDay,currentMonth,currentYear + 2000,currentHours,currentMinutes,currentSeconds); // Mostrar el año completo en el mensaje

    RTC_BACKUP_REGISTER0 = RTC_INITIALIZED_FLAG_VALUE; // Escribir directamente a la dirección de memoria
    log_0("RTC initialization flag set. \n\r");
  } else {
    //log_3("RTC ya fue inicializado. \n\r");
  }

  log_3("RTC initiated and verified.\n\r");

}
