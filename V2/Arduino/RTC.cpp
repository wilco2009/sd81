#include <STM32RTC.h>


STM32RTC& rtc = STM32RTC::getInstance(); // Obtiene la instancia del objeto RTC

// Definir un valor "mágico" para nuestra bandera
#define RTC_INITIALIZED_FLAG_VALUE 0x32F1 // Un número aleatorio para identificarlo

// --- DEFINICIONES PARA ACCESO DIRECTO A REGISTROS DE RESPALDO (ULTIMO RECURSO) ---
// Dirección de memoria del primer registro de datos de respaldo (BKP_DR0) para STM32F4xx
#define RTC_BKP_DR0_ADDRESS ((uint32_t)0x40002850)
// Macro para acceder al valor en esa dirección de memoria (se asegura que sea volátil)
#define RTC_BACKUP_REGISTER0 (*((volatile uint32_t *)RTC_BKP_DR0_ADDRESS))


void rtc_init(void){
  log_3("Iniciando configuracion y consulta de fecha y hora del RTC...\n\r");

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
    log_3("Fuente de reloj del RTC configurada como LSI.\n\r");
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSIRDY) != RESET) {
      log_1("✅ El oscilador LSI esta activo y estable.\n\r");
    } else {
      log_0("❌ ADVERTENCIA: LSI seleccionado, pero NO ESTA ESTABLE.\n\r");
    }
  } else if (rtc_clock_source == RCC_RTCCLKSOURCE_LSE) {
    log_3("Fuente de reloj del RTC configurada como LSE.\n\r");
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_LSERDY) != RESET) {
      log_3("✅ El oscilador LSE esta activo y estable.\n\r");
    } else {
      log_0("❌ ADVERTENCIA: LSE seleccionado, pero NO ESTA ESTABLE. Revisa hardware (cristal/condensadores).\n\r");
    }
  } else if (rtc_clock_source == RCC_RTCCLKSOURCE_HSE_DIV2) {
    log_0("Fuente de reloj del RTC configurada como HSE/2.\n\r");
    if (__HAL_RCC_GET_FLAG(RCC_FLAG_HSERDY) != RESET) {
      log_0("✅ El oscilador HSE esta activo y estable.\n\r");
    } else {
      log_0("❌ ADVERTENCIA: HSE/2 seleccionado, pero NO ESTA ESTABLE.\n\r");
    }
  } else {
    log_0("Fuente de reloj del RTC desconocida o no configurada.\n\r");
  }

  // Verificar si el reloj del RTC está habilitado en el dominio de respaldo (RTCEN)
  if ((RCC->BDCR & RCC_BDCR_RTCEN) != RESET) {
      log_3("✅ El reloj del RTC (RTCEN) esta habilitado en BDCR.\n\r");
  } else {
      log_0("❌ ADVERTENCIA: El reloj del RTC (RTCEN) NO esta habilitado. Intentando habilitacion...\n\r");
      __HAL_RCC_RTC_ENABLE();
      delay(10);
      if ((RCC->BDCR & RCC_BDCR_RTCEN) != RESET) {
         log_0("RTCEN habilitado con exito.\n\r");
      } else {
         log_0("Fallo al habilitar RTCEN. Problema critico.\n\r");
      }
  }

  // --- USAR EL REGISTRO DE RESPALDO COMO BANDERA (ACCESO DIRECTO A LA DIRECCIÓN) ---
  uint32_t backup_flag_read = RTC_BACKUP_REGISTER0; // Acceso directo a la dirección de memoria

  if (backup_flag_read != RTC_INITIALIZED_FLAG_VALUE) {
    log_0("RTC parece estar desconfigurado (bandera no encontrada). Configurando fecha y hora...\n\r");
    
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

    log_0("RTC configurado a %d/%d/%d %d:%d:%d \n\r",currentDay,currentMonth,currentYear + 2000,currentHours,currentMinutes,currentSeconds); // Mostrar el año completo en el mensaje

    RTC_BACKUP_REGISTER0 = RTC_INITIALIZED_FLAG_VALUE; // Escribir directamente a la dirección de memoria
    log_0("Bandera de inicializacion del RTC establecida. \n\r");
  } else {
    log_3("RTC ya fue inicializado (bandera encontrada). No se reconfigura.\n\r");
  }

  log_3("RTC iniciado y verificado.\n\r");
}
