#ifndef main_h
#define main_h

#include <Arduino.h>
#include "device_def.h"
#include "my_debuglog.h"
#include "my_EEPROM.h"
#include "my_MQTT.h"
#include "my_NTP.h"
#include "my_ota.h"
#include "my_scheduler.h"
#include "my_wifi.h"
#include "my_sysmon.h"

#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
// include
#include "my_door.h"
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
String get_glob_reason(bool wait);

// Глобальные переменные для постоянного хранения в памяти
// extern uint8 glob_reason;        // причина сброса
extern uint16_t wifi_count_conn; // количество (пере)подключений к WiFi

extern uTask ut_wifi;
extern uTask ut_sysmon;
extern uTask ut_debuglog;
extern uTask ut_emptymemory;
extern uTask ut_OTA;
extern uTask ut_NTP;
extern uTask ut_MQTT;

#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
// задачи
// extern uTask ut_NTC;
// extern uTask ut_math;
extern uTask ut_door;
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

#include "my_MQTT.h"
extern e_state_MQTT MQTT_state;  // Состояние подключения к MQTT
extern uint16_t mqtt_count_conn; // количество (пере)подключений к серверу mqtt

#include "my_wifi.h"
extern e_state_WiFi wifi_state; // Состояние WIFi

// // enum e_microprogramm
// // {
// //     // OFF,
// //     // ARM,
// //     // AUTO,
// //     WS_TANK, //
// //     UP_ARM,
// //     UP_AUTO,
// //     UP_WS_TANK,
// //     DOWN_ARM,
// //     DOWN_AUTO,
// //     DOWN_WS_TANK,
// //     WOV_ARM,
// //     WOV_AUTO,
// //     WOV_WS_TANK
// // };

// enum e_state_conn
// {
//     mc_connectingWIFI,
//     mc_initOTA,
//     mc_connectingServ,
//     mc_connectedServ,
//     mc_uploadOverOTA,
//     mc_WebConfigAP,
//     mc_NULL
// };

// enum e_state_ESP
// {
//     mm_start,    //Запуск и инициализация железа. Моргнуть (50) всеми светодиодами по очереди. Отобразить в глобальной переменной причину запуска: включено питание, аппаратный сброс, программный сброс, WDT и т.д. и позже передать на сервер.
//     mm_autonomy, //Автономный режим. Работает по выбранной автономной программе не зависимо от связи с сервером. При наличии связи с сервером шлет ему данные.
//     mm_remote,   //Дистанционный режим. Логика работы автоматики полностью лежит на сервере - он полностью контролирует датчики и управляет исполнительными механизмами. При отсутствии связи с сервером автоматика работает по выбранной по умолчанию автономной программе.
//     mm_arm,      //Ручной режим. Ручной контроль датчиков и управление исполнительными механизмами с сервера. При отсутствии связи с сервером автоматика работает по выбранной по умолчанию автономной программе.
//     mm_NULL
// };

// enum e_state_ex // исключительные случаи - порядок по приоритету
// {
//     warn_none,     // аварий и предупреждений нет
//     warn_TO_filtr, // нужна замена фильтров
//     warn_TO_tank,  // нужно помыть бочку
//     warn_device,   // не критичный отказ датчика - можно работать дальше
//     err_device,    // критичный отказ датчика - ??? определиться что делать
//     err_fault,     // критическая авария - закрыть ввод
//     ex_NULL
// };

// // enum e_state_TO
// // {
// //     TO_NONE,
// //     TO_filtr, // нужна замена фильтров
// //     TO_tank   // нужно помыть бочку
// // };

// struct s_conn_param_RAM
// {
//     // e_state_WiFi state_WiFi = _wifi_disconnected; // Состояние WIFi
//     s_conn_param_ROM *tmp_conn_param_ROM = NULL;  // указатель на параметры сервера, читаем из ROM при дисконекте и удаляем из памяти при получении IP
//     s_wifi_serv wifi_serv = {WIFI_SSID1, WIFI_PASS1, IPmqttWIFI1, mqtt_user, mqtt_pass};
//     // uint8_t nowIP[4];         // Tекущий IP - это в RAM
//     // e_state_internet state_NTP; // Состояние NTP
//     // e_state_MQTT MQTT_state;    // Состояние MQTT
//     // e_state_ESP state_ESP;      // Состояние ESP
//     // e_state_STM state_STM;      // Состояние STM32
//     // nowtime;            // Текущее местное время // здесь, или в sys?
// };

// #define addr_log_ROM addr_NTC_ROM + len_NTC_ROM
// #define len_log_ROM sizeof(bool)

// struct s_all_param_RAM
// {
//     // #ifdef log_ // не получилось реализовать
//     bool log; // по умолчанию
//               // #endif
//     s_conn_param_RAM conn_param;
//     // uint32_t reason; // Причина сброса // Один раз по MQTT
//     // NTC *p_NTC = NULL;
// };

// struct s_sys_param_RAM
// {
//     // e_mode_sys mode_sys;        // Режим работы системы
//     // e_submode_sys submode_sys;  // Подрежим работы системы
//     // e_state_ex state_ex;        // Исключительные случаи
//     // e_state_WiFi state_WiFi; // Состояние WIFi
//     // // e_state_internet state_NTP; // Состояние NTP
//     // // e_state_MQTT MQTT_state;    // Состояние MQTT
//     // // e_state_ESP state_ESP;      // Состояние ESP
//     // // e_state_STM state_STM;      // Состояние STM32
//     // reason_time; // Время сброса
//     // nowtime;            // Текущее местное время
// };

// e_microprogramm programm_cyrrent; // текущая программа
// e_microprogramm programm_noSTM;   // программа дистанционного режима, работет при отсутствии связи с сервером
// e_microprogramm programm_noMQTT;  // программа ручного режима, работет при отсутствии связи с сервером

// typedef union flash_data {
//     struct main_data
//     {
//         ; /* data */
//     };
//     char arr_data[sizeof(main_data)];
// };

// typedef union {
//     uint32_t value = 0;
//     struct
//     { // не более 32 бита
//         unsigned bit_vnimanie_sonar : 1;
//     };
//     struct
//     { // не более 32 бита
//         unsigned bit_izm_sonar : 1;
//         unsigned bit_izm_pereliv : 1;
//         unsigned bit_izm_nasos : 1;
//         unsigned bit_izm_rezim : 1;
//     };
//     // struct
//     // { // не более 32 бита
//     //     unsigned bit0:1;
//     //     unsigned bit1:1;
//     //     unsigned bit2:1;
//     //     unsigned bit3:1;
//     //     unsigned bit4:1;
//     //     unsigned bit5:1;
//     //     unsigned bit6:1;
//     //     unsigned bit7:1;
//     //     unsigned bit8:1;
//     //     unsigned bit9:1;
//     //     unsigned bit10:1;
//     //     unsigned bit11:1;
//     //     unsigned bit12:1;
//     //     unsigned bit13:1;
//     //     unsigned bit14:1;
//     //     unsigned bit15:1;
//     //     unsigned bit16:1;
//     //     unsigned bit17:1;
//     //     unsigned bit18:1;
//     //     unsigned bit19:1;
//     //     unsigned bit20:1;
//     //     unsigned bit21:1;
//     //     unsigned bit22:1;
//     //     unsigned bit23:1;
//     //     unsigned bit24:1;
//     //     unsigned bit25:1;
//     //     unsigned bit26:1;
//     //     unsigned bit27:1;
//     //     unsigned bit28:1;
//     //     unsigned bit29:1;
//     //     unsigned bit30:1;
//     //     unsigned bit31:1;
//     // };
// };

// #define qnt_rezhim 5
// const char *c_rezhim[qnt_rezhim] =
//     {
//         "Автоматика отключена",
//         "Ручной",
//         "Автоматический",
//         "Только сетевая вода",
//         "Автоматический, заполнять без насоса"};

// #define qnt_command 5
// const char *c_command[qnt_command] =
//     {
//         "Нет команды",
//         "Заполнить бочку",
//         "Слить бочку",
//         "Замена фильтра Ф1",
//         "Замена фильтра Ф2"};

// #define qnt_rezh_lcd 4
// const char *c_rezh_lcd[qnt_rezh_lcd] =
//     {
//         "Счетчики",
//         "Фильты",
//         "Бочка",
//         "Сонар"};

// struct ss_voda //f-фактически измеренное, m-математически вычесленное, c-константа для расчетов
// {
//     u32bit vnimanie;        // светодиод "внимание"
//     u32bit izm_var;         // изменения значений переменных для алгоритма
//                             //    u32bit izm_blynk;       // изменения значений переменных для blynk
//                             //   int rezhim;             // текущий режим работы
//     uint f_sonar_value = 0; // значение от сонара
//     //unsigned long f_millis=0;
//     //unsigned long c_millis=5000;
//     uint m_sonar_value_mm = 999; // фильтрованное актуальное расстояние от датчика до поверхности воды
//     uint c_bot_mm = 535;         // для расчетов: расстояние от датчика до нижнего уровня воды в бочке
//     uint c_top_mm = 100;         // для расчетов: расстояние от датчика до верхнего уровня воды в бочке
//     uint c_min_urov = 5;         // для автоматического режима: уровень воды в процентах когда надо включить наполнение бочки
//     uint c_max_urov = 85;        // для автоматического режима: уровень воды в процентах когда надо отключить наполнение бочки
//     uint c_min_for_vect = 2;     // минимальное значение изменения среди всех в буфере при котором определяется вектор движения
//     uint m_min_urov = 0;         // минимальное значение в буфере
//     uint m_max_urov = 0;         // максимальное значение в буфере
//     uint m_sub = 999;
//     uint c_max_sub = 100;   //
//     int m_vector = 999;     // -1 отбор, 0 нет отбора, 1 набор
//     int m_voda_proc;        // расчитанный уровень воды в процентах
//     int m_voda_litr;        // расчитанный уровень воды в литрах - от счетчиков
//     bool v_pereliv = false; // текущее состояние датчика перелива, всплывает - размыкается=true
//     bool v_protok;          // текущее состояние датчика протока
//     int v_cntr_vvod;        // текущий расход воды на вводе
//     long v_summ_vvod;       // расходованно воды с ввода
//     int v_cntr_in;          // текущий расход воды в бочку
//     long v_summ_in;         // залито воды в бочку
//     int v_cntr_out;         // текущий расход воды из бочки
//     long v_summ_out;        // слито воды из бочки
// } s_voda;

#endif // main_h