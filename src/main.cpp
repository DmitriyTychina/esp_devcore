#include <Arduino.h>
// #include <ets_sys.h> // — спецефичные структуры и определения для работы с событиями и таймерами.
// #include <osapi.h> // — таймеры и некоторые системные функции, например os_strcat, os_memcpy, os_delay_us и т.п..
// #include <os_type.h> // — мапинг структур из ets_sys.h.
//  //user_interface.h — множество вспомогательных структур и процедур API, в частности для работы с wi-fi, процедуры system_restart, system_deep_sleep и т.д..
// #include <espconn.h> // — основной файл API со структурами и процедурами по работе с TCP и UDP соединениями.
// #include <mem.h> // — работа с памятью, os_malloc, os_free и т.п.
// #include <gpio.h> // — вспомогательные
#include "main.h"

// Глобальные переменные для постоянного хранения в памяти
// uint8 glob_reason;

#if defined(ESP8266)
#include <user_interface.h> //для system_get_rst_info()
String get_glob_reason(bool _wait)
{
  //     REASON_DEFAULT_RST = 0,      /* включение питания */
  //     REASON_WDT_RST = 1,          /* аппаратный сброс по WDT */
  //     REASON_EXCEPTION_RST = 2,    /* сброс по исключению, GPIO не изменяются */
  //     REASON_SOFT_WDT_RST = 3,     /* программный сброс по WDT, GPIO не изменяются */
  //     REASON_SOFT_RESTART = 4,     /* программный сброс , GPIO не изменяются */
  //     REASON_DEEP_SLEEP_AWAKE = 5, /* выход из глубокого сна */
  //     REASON_EXT_SYS_RST = 6       /* аппаратный сброс */
  rst_info *rst_info;
  rst_info = system_get_rst_info(); // причина сброса
  uint8 glob_reason = rst_info->reason;
  if (_wait && (glob_reason != REASON_DEFAULT_RST) && (glob_reason != REASON_EXT_SYS_RST) && (glob_reason != REASON_SOFT_RESTART /* после прошивки OTA */))
  {
    os_delay_us(500000); // чтобы при "непонятных" сбросах данные не мелькали в serial-порту
  }
  String tmp_str;
  switch (glob_reason)
  {
  case REASON_DEFAULT_RST:
    tmp_str = F("[0]PowerOn");
    break;
  case REASON_WDT_RST:
    tmp_str = F("[1]HardWDT");
    break;
  case REASON_EXCEPTION_RST:
    tmp_str = F("[2]Exception");
    break;
  case REASON_SOFT_WDT_RST:
    tmp_str = F("[3]SoftWDT");
    break;
  case REASON_SOFT_RESTART:
    tmp_str = F("[4]OTA");
    break;
  case REASON_DEEP_SLEEP_AWAKE:
    tmp_str = F("[5]DeepSleepAwake");
    break;
  case REASON_EXT_SYS_RST:
    tmp_str = F("[6]HardReset");
    break;
  default:
    tmp_str = F("Unknown");
  }
  return (tmp_str);
}
#elif defined(ESP32)
String get_glob_reason(bool _wait)
{
    // ESP_RST_UNKNOWN,    //!< Reset reason can not be determined
    // ESP_RST_POWERON,    //!< Reset due to power-on event
    // ESP_RST_EXT,        //!< Reset by external pin (not applicable for ESP32)
    // ESP_RST_SW,         //!< Software reset via esp_restart
    // ESP_RST_PANIC,      //!< Software reset due to exception/panic
    // ESP_RST_INT_WDT,    //!< Reset (software or hardware) due to interrupt watchdog
    // ESP_RST_TASK_WDT,   //!< Reset due to task watchdog
    // ESP_RST_WDT,        //!< Reset due to other watchdogs
    // ESP_RST_DEEPSLEEP,  //!< Reset after exiting deep sleep mode
    // ESP_RST_BROWNOUT,   //!< Brownout reset (software or hardware)
    // ESP_RST_SDIO,       //!< Reset over SDIO
  esp_reset_reason_t rst_info = esp_reset_reason(); // причина сброса
  if (_wait && (rst_info != ESP_RST_POWERON) && (rst_info != ESP_RST_EXT) && (rst_info != ESP_RST_SW /* после прошивки OTA */))
  {
    ets_delay_us(500000); // чтобы при "непонятных" сбросах данные не мелькали в serial-порту
  }
  String tmp_str;
  switch (rst_info)
  {
  case ESP_RST_UNKNOWN:
    tmp_str = F("[0]ESP_RST_UNKNOWN");
    break;
  case ESP_RST_POWERON:
    tmp_str = F("[1]ESP_RST_POWERON");
    break;
  case ESP_RST_EXT:
    tmp_str = F("[2]ESP_RST_EXT");
    break;
  case ESP_RST_SW:
    tmp_str = F("[3]ESP_RST_SW");
    break;
  case ESP_RST_PANIC:
    tmp_str = F("[4]ESP_RST_PANIC");
    break;
  case ESP_RST_INT_WDT:
    tmp_str = F("[5]ESP_RST_INT_WDT");
    break;
  case ESP_RST_TASK_WDT:
    tmp_str = F("[6]ESP_RST_TASK_WDT");
    break;
  case ESP_RST_WDT:
    tmp_str = F("[7]ESP_RST_WDT");
    break;
  case ESP_RST_DEEPSLEEP:
    tmp_str = F("[8]ESP_RST_DEEPSLEEP");
    break;
  case ESP_RST_BROWNOUT:
    tmp_str = F("[9]ESP_RST_BROWNOUT");
    break;
  case ESP_RST_SDIO:
    tmp_str = F("[10]ESP_RST_SDIO");
    break;
  default:
    tmp_str = F("Unknown");
  }
  return (tmp_str);
}
#endif



#if defined(EEPROM_C)
s_sys_settings_ROM *g_p_sys_settings_ROM = NULL;
#elif defined(EEPROM_CPP)

#endif

// Экземпляры классов
// NTC c_NTC;
// HCSR04 ultrasonicSensor;

volatile unsigned long uTask::iCPUStart = 0;
volatile unsigned long uTask::iCPUCycle = 0;
volatile unsigned long uTask::iCPUCore = 0;
volatile unsigned long uTask::iCPUTT = 0;

// ****************** Независимые задачи
uTask ut_wifi(WiFi_TtaskDefault, &cb_ut_state_wifi);      // Подключение к wifi
uTask ut_sysmon(SysMon_TtaskDefault, &cb_ut_sysmon);      // sysmon
uTask ut_debuglog(RSDebug_TtaskDefault, &cb_ut_debuglog); // RSDedug
// #if defined(EEPROM_C) || defined(EEPROM_CPP)
// #endif
// #if defined(EEPROM_C)
uTask ut_emptymemory(emptymemory_TtaskDefault, &cb_ut_emptymemory, true); // автоматически очищаем память
// #elif defined(EEPROM_CPP)

// #endif

#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
// задачи
// ****************** Задача опроса датчиков
// uTask ut_NTC(NTC_TtaskDefault, &cb_NTC, true); // NTC - датчик температуры
uTask ut_door(door_TtaskDefault, &cb_ut_door, true);
// ****************** Отработка алгоритмов и управление
// uTask ut_math(Math_TtaskDefault, &cb_math, true); // математика и алгоритмы
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

// ****************** Зависящие от подключения к wifi
uTask ut_OTA(OTA_TtaskDefault, &cb_ut_ota);    // OTA
#ifdef CORE_NTP
uTask ut_NTP(NTP_TtaskDefault, &cb_ut_NTP);    // time NTP
#endif
uTask ut_MQTT(MQTT_TtaskDefault, &cb_ut_MQTT); // MQTT

void setup()
{
  // Serial.begin(74880); // родная скорость ESP
  // Serial1.begin(115200);
  // Serial.begin(115200); // для RS485 к STM
  // Serial.swap(); // для RS485 к STM
  init_sdebuglog(true);
  rsdebugInflnF("\r********************Start********************");
  rsdebugInfF("Причина сброса: ");
  rsdebugInfln("%s", get_glob_reason(true).c_str());
  // rsdebugInfln("Причина сброса: %s", get_glob_reason(true).c_str());
  rsdebugInflnF("***Проверка сохраненных настроек***");
  ROMVerifySettingsElseSaveDefault();
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->MQTT_user);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->MQTT_pass);
  // rsdebugInfln("***MQTT_user[%d]", ram_data.p_NET_settings()->MQTT_Ttask);
  // rsdebugInfln("***MQTT_user[%d]", ram_data.p_NET_settings()->WiFi_Ttask);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->settings_serv[0].SSID);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->settings_serv[0].MQTTip);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->settings_serv[0].PASS);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->settings_serv[1].SSID);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->settings_serv[1].MQTTip);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->settings_serv[1].PASS);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->settings_serv[2].SSID);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->settings_serv[2].MQTTip);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->settings_serv[2].PASS);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->settings_serv[3].SSID);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->settings_serv[3].MQTTip);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->settings_serv[3].PASS);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->settings_serv[4].SSID);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->settings_serv[4].MQTTip);
  // rsdebugInfln("***MQTT_user[%s]", ram_data.p_NET_settings()->settings_serv[4].PASS);

  // optimistic_Core(2000000);
  rsdebugInflnF("***Инициализация модулей***");
  rsdebugInflnF("--SysMon init");
  SysMon_Init();
  rsdebugInflnF("--RSdebug init");
  init_sdebuglog();

#ifdef USER_AREA
  // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
  // ранняя инициализация
  rsdebugInflnF("--Door init");
  door_init();
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

  rsdebugInflnF("--WiFi init");
  init_WiFi();
  rsdebugInflnF("--OTA init");
  init_OTA();
#ifdef CORE_NTP
  rsdebugInflnF("--NTP init");
  init_NTP();
#endif

#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
// инициализация
// rsdebugInflnF("--tgbcr init");
// math_init();
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
}

void loop()
{
  // rsdebugInflnF("#0");
  ut_sysmon.StopStopwatchCore();
  // rsdebugInflnF("#1");

  ut_OTA.execute();
  // rsdebugInflnF("#2");
  ut_wifi.execute();
  // rsdebugInflnF("#3");
  ut_MQTT.execute();
  // rsdebugInflnF("#4");
#ifdef CORE_NTP
  ut_NTP.execute();
#endif
  // rsdebugInflnF("#5");

#ifdef USER_AREA
  // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
  // вызов задач
  // ut_NTC.execute();
  // ut_math.execute();
  ut_door.execute();
  // rsdebugInflnF("#6");
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

  ut_debuglog.execute();
  // rsdebugInflnF("#7");
  ut_emptymemory.execute();
  // rsdebugInflnF("#8");
  if (ut_sysmon.execute()) // должна быть последняя в loop()
    ut_sysmon.cpuLoadReset();
  // rsdebugInflnF("#9");

  ut_sysmon.StartStopwatchCore();
  // rsdebugInflnF("#10");
}
