#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "device_def.h"
#include "my_MQTT.h"

enum e_state_WiFi : uint8
{
    _wifi_disconnected,    // отключены
    _wifi_startfindAP,     // запуск поиска точек доступа
    _wifi_findAP,          // поиск точек доступа
    // _wifi_startconnecting, // запуск подключения
    _wifi_connecting,      // подключение
    _wifi_gotIP,           // подключились, получили IP
    _wifi_connected        // подключены
};

// struct u_data_WiFi
// {
//     e_state_WiFi state_WiFi = _wifi_disconnected; // Состояние WIFi
//     // uint32_t counter;
// };

// extern e_state_WiFi state_WiFi; // Состояние WIFi
// extern u_data_WiFi data_WiFi;     // Состояние WIFi
extern uint16_t wifi_count_conn; // количество (пере)подключений к WiFi
extern e_state_WiFi wifi_state; // Состояние WIFi
// extern uint8_t idx_eth_rom; // номер записи в g_p_ethernet_settings_ROM - для получения адреса сервера MQTT

#define lenMQTTip 16
#define lenMQTTuser 8
#define lenMQTTpass 8

#define WiFi_TtaskDefault 251

struct s_settings_serv
{
    char SSID[16];          // Имя точки WiFi
    char PASS[24];          // Пароль точки WiFi
    char MQTTip[lenMQTTip]; // IP сервера MQTT в этой сети
    // char MQTTlogin[8]; // Логин сервера MQTT
    // char MQTTpass[8];  // Пароль сервера MQTT
};

struct s_ethernet_settings_ROM
{
    // uint16_t crc;
    uint32_t WiFi_Ttask = WiFi_TtaskDefault;
    s_settings_serv settings_serv[5] = {{WIFI_SSID3, WIFI_PASS3, IPmqttWIFI3},
                                        {WIFI_SSID2, WIFI_PASS2, IPmqttWIFI2},
                                        {WIFI_SSID1, WIFI_PASS1, IPmqttWIFI1},
                                        {WIFI_SSID4, WIFI_PASS4, IPmqttWIFI4},
                                        {WIFI_SSID5, WIFI_PASS5, IPmqttWIFI5}}; // Параметры точкек WiFi и серверов MQTT в этих сетях
    char MQTT_user[lenMQTTuser] = mqtt_user;                                    // Логин сервера MQTT
    char MQTT_pass[lenMQTTpass] = mqtt_pass;                                    // Пароль сервера MQTT
    uint32_t MQTT_Ttask = MQTT_TtaskDefault;
    // bool stawebserver = false;                                                                                          // Разрешить web-сервер в режиме клиента
    //    char apName[16] = my_AP_NAME;                                                             // Имя точки доступа WiFi
    //    char apPass[24] = my_AP_PASS;                                                             // Пароль точки доступа WiFi
    //    uint8_t apIP[4] = IPAP;                                                                   // IP web-cервера точки доступа WiFi
    // uint16_t Tms = 500; // Период, мс --- для шедулера -не нужен -меняем динамически
};

void init_WiFi();
uint8_t get_idx_eth(String str_ssid);
void cb_ut_state_wifi(void);

extern s_ethernet_settings_ROM *g_p_ethernet_settings_ROM;
