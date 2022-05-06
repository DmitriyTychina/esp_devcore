#include <Arduino.h>
// #include <ets_sys.h> // — спецефичные структуры и определения для работы с событиями и таймерами.
// #include <osapi.h> // — таймеры и некоторые системные функции, например os_strcat, os_memcpy, os_delay_us и т.п..
// #include <os_type.h> // — мапинг структур из ets_sys.h.
//  //user_interface.h — множество вспомогательных структур и процедур API, в частности для работы с wi-fi, процедуры system_restart, system_deep_sleep и т.д..
// #include <espconn.h> // — основной файл API со структурами и процедурами по работе с TCP и UDP соединениями.
// #include <mem.h> // — работа с памятью, os_malloc, os_free и т.п.
// #include <gpio.h> // — вспомогательные
#include <user_interface.h> //для system_get_rst_info()

#include "main.h"

// Глобальные переменные для постоянного хранения в памяти
// uint8 glob_reason;

String get_glob_reason(bool _wait)
{
  //     REASON_DEFAULT_RST = 0,      /* включение питания */
  //     REASON_WDT_RST = 1,          /* аппаратный сброс по WDT */
  //     REASON_EXCEPTION_RST = 2,    /* сброс по исключению, GPIO не изменяются */
  //     REASON_SOFT_WDT_RST = 3,     /* программный сброс по WDT, GPIO не изменяются */
  //     REASON_SOFT_RESTART = 4,     /* программный сброс , GPIO не изменяются */
  //     REASON_DEEP_SLEEP_AWAKE = 5, /* выход из глубокого сна */
  //     REASON_EXT_SYS_RST = 6       /* аппаратный сброс */
  rst_info *p_rst_info;
  p_rst_info = system_get_rst_info(); // причина сброса
  uint8 glob_reason = p_rst_info->reason;
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
uTask ut_NTP(NTP_TtaskDefault, &cb_ut_NTP);    // time NTP
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
  rsdebugInflnF("--NTP init");
  init_NTP();

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
  // ut_OTA.execute();
  ut_MQTT.execute();
  // rsdebugInflnF("#4");
  // ut_OTA.execute();
  ut_NTP.execute();
  // rsdebugInflnF("#5");
  // ut_OTA.execute();

#ifdef USER_AREA
  // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
  // вызов задач
  // ut_NTC.execute();
  // ut_math.execute();
  ut_door.execute();
  // rsdebugInflnF("#6");
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

  // ut_OTA.execute();
  ut_debuglog.execute();
  // rsdebugInflnF("#7");
  // ut_OTA.execute();
  ut_emptymemory.execute();
  // rsdebugInflnF("#8");
  // ut_OTA.execute();
  if (ut_sysmon.execute()) // должна быть последняя в loop()
    ut_sysmon.cpuLoadReset();
  // rsdebugInflnF("#9");

  ut_sysmon.StartStopwatchCore();
  // rsdebugInflnF("#10");
}
