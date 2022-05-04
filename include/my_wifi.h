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

void init_WiFi();
uint8_t get_idx_eth(String str_ssid);
void cb_ut_state_wifi(void);

