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
uint8 glob_reason;
s_sys_settings_ROM *g_p_sys_settings_ROM = NULL;

// Экземпляры классов
// NTC c_NTC;
// HCSR04 ultrasonicSensor;

volatile unsigned long uTask::iCPUStart = 0;
volatile unsigned long uTask::iCPUCycle = 0;
volatile unsigned long uTask::iCPUCore = 0;
volatile unsigned long uTask::iCPUTT = 0;

// ****************** Независимые задачи
uTask ut_wifi(WiFi_TtaskDefault, &cb_state_wifi);         // Подключение к wifi
uTask ut_sysmon(SysMon_TtaskDefault, &cb_sysmon);         // sysmon
uTask ut_debuglog(RSDebug_TtaskDefault, &cb_my_debuglog); // RSDedug
uTask ut_emptymemory(emptymemory_TtaskDefault, &cb_emptymemory, true);

#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
// задачи
// ****************** Задача опроса датчиков
// uTask ut_NTC(NTC_TtaskDefault, &cb_NTC, true); // NTC - датчик температуры
uTask ut_door(door_TtaskDefault, &cb_door, true);
// ****************** Отработка алгоритмов и управление
// uTask ut_math(Math_TtaskDefault, &cb_math, true); // математика и алгоритмы
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

// ****************** Зависящие от подключения к wifi
uTask ut_OTA(OTA_TtaskDefault, &t_ota_cb);    // OTA
uTask ut_NTP(NTP_TtaskDefault, &t_NTP_cb);    // time NTP
uTask ut_MQTT(MQTT_TtaskDefault, &t_MQTT_cb); // MQTT

void setup()
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
  glob_reason = p_rst_info->reason;
  if ((glob_reason != REASON_DEFAULT_RST) & (glob_reason != REASON_EXT_SYS_RST) & (glob_reason != REASON_SOFT_RESTART /* после прошивки OTA */))
    os_delay_us(500000); // чтобы при "непонятных" сбросах данные не мелькали в serial-порту
  // Serial.begin(74880); // родная скорость ESP
  // Serial1.begin(115200);
  // Serial.begin(115200); // для RS485 к STM
  // Serial.swap(); // для RS485 к STM
  init_sdebuglog(true);
  rsdebugInflnF("\r********************Start********************");
  rsdebugInflnF("***Проверка сохраненных настроек***");
  ROMVerifySettingsElseSaveDefault();
  // optimistic_Core(2000000);
  rsdebugInflnF("***Инициализация модулей***");
  SysMon_Init();
  rsdebugInflnF("SysMon");
  init_sdebuglog();

#ifdef USER_AREA
  // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
  // ранняя инициализация
  door_init();
  rsdebugInflnF("Door");
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

  String tmp_str1, tmp_str2;
  tmp_str1 = F("Причина сброса: ");
  switch (glob_reason)
  {
  case REASON_DEFAULT_RST:
    tmp_str2 = F("включение питания");
    break;
  case REASON_WDT_RST:
    tmp_str2 = F("аппаратный сброс по WDT");
    break;
  case REASON_EXCEPTION_RST:
    tmp_str2 = F("сброс по исключению, GPIO не изменяются");
    break;
  case REASON_SOFT_WDT_RST:
    tmp_str2 = F("программный сброс по WDT, GPIO не изменяются");
    break;
  case REASON_SOFT_RESTART:
    tmp_str2 = F("программный сброс (после OTA), GPIO не изменяются");
    break;
  case REASON_DEEP_SLEEP_AWAKE:
    tmp_str2 = F("выход из глубокого сна");
    break;
  case REASON_EXT_SYS_RST:
    tmp_str2 = F("аппаратный сброс");
    break;
  default:
    tmp_str2 = F("???");
  }
  rsdebugInfln(("%s%s"), tmp_str1.c_str(), tmp_str2.c_str());

  init_WiFi();
  rsdebugInflnF("WiFi");
  init_OTA();
  rsdebugInflnF("OTA");
  init_NTP();
  rsdebugInflnF("NTP");

#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
// инициализация
// math_init();
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
}

void loop()
{
  ut_sysmon.StopStopwatchCore();

  ut_OTA.execute();
  ut_NTP.execute();
  ut_MQTT.execute();
  ut_wifi.execute();

#ifdef USER_AREA
  // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
  // вызов задач
  // ut_NTC.execute();
  // ut_math.execute();
  ut_door.execute();
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

  ut_debuglog.execute();
  ut_emptymemory.execute();
  if (ut_sysmon.execute()) // должна быть последняя в loop()
    ut_sysmon.cpuLoadReset();
    
  ut_sysmon.StartStopwatchCore();
}
