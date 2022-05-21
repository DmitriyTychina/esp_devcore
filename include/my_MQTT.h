#pragma once

#include <Arduino.h>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ESP32)
#include <WiFi.h>
#endif
#include "AsyncMqttClient.h"
#include "device_def.h"

#define MQTT_TtaskDefault 53
#define MQTT_MAX_LEVEL 6
#define MQTT_MAX_SYMBOLS 16
// #define MQTT_QUEUE

// не более 255, индекс массива везде 8 бит
enum e_IDDirTopic// : uint8 // в паре с ArrDirTopic[]
{
    d_empty,
    d_main_topic,
    d_net,
    d_system,
    d_state,
    d_status,
    d_devices,
#ifdef USER_AREA
    // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
    d_door,
// // ESP/NTC - датчик температуры
// _ESP_NTC_Math,
// _ESP_NTC_Settings,
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
    d_info,
    d_settings,
    // Settings/WIFI
    d_wifi,
    d_ap1,
    d_ap2,
    d_ap3,
    d_ap4,
    d_ap5,
    d_mqtt,
    d_ota,
    d_ntp,
    d_astro,
    // Settings/RSdebug
    d_rs_debug,
    d_r_debug,
    d_s_debug,
    d_sysmon,
    d_info_rsdebug,
    d_info_mqtt,
    d_flash,
    d_ram,
    d_rom,
    d_free_up_ram,
    d_chip_esp,
    d_status_led,
    d_commands,
    // last element (unuse)
    _LastElement_e_IDDirTopic // для того чтобы компилятор сообщил что в ArrDirTopic больше элементов чем в e_IDDirTopic
};
const char ArrDirTopic[_LastElement_e_IDDirTopic][MQTT_MAX_SYMBOLS] /* PROGMEM */ = // в паре с e_IDDirTopic
    {
        "",
        OTA_NAME,
        "Net",
        "System",
        "State",
        "Status",
        "Devices",
#ifdef USER_AREA
        // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
        "Door",
// // ESP/NTC - датчик температуры
// "ESP/NTC/Math",
// "ESP/NTC/Settings",
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif          // USER_AREA
        "Info", // остальные папки топиков пишем без основного топика системы
        "Settings",
        // Settings/WIFI
        "WiFi",
        "AP1",
        "AP2",
        "AP3",
        "AP4",
        "AP5",
        "MQTT",
        "OTA",
        "NTP",
        "Astro",
        // Settings/RSdebug
        "RSdebug",
        "Rdebug",
        "Sdebug",
        "SysMon",
        "InfoToRSdebug",
        "InfoToMQTT",
        "Flash",
        "RAM",
        "ROM",
        "FreeUpRAM",
        "ChipESP",
        "StatusLED",
        "Commands"};

// не более 255, индекс массива везде используем 8бит
enum e_IDVarTopic// : uint8 // в паре с ArrVarTopic[]
{
    v_empty,
    v_all,
#ifdef USER_AREA
    // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
    // // Math
    // _Value,
    // _Quality,
    // Door
    v_latch,
    v_open,
// //DeviceESP/NTC/Settings
// _T_MQTT,
// _DataDefault,
// _DataMin,
// _DataMax,
// _DataAge,
// _Kfiltr,
// _Vcc,
// _Rs,
// _Ka,
// _Kb,
// _Kc,
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
    // Settings
    v_enable,
    v_default,
    v_t_task,
    // NTP
    v_t_sync,
    v_ip1,
    v_ip2,
    v_ip3,
    v_timezone,
    // Wifi
    v_ssid,
    v_pass,
    v_server,
    // MQTT
    v_user,
    // Info/
    v_current_wifi_ssid,
    v_current_wifi_rssi,
    v_currentip,
    v_reason_reset,
    v_time_reset,
    v_cnt_reconn_mqtt,
    v_cnt_reconn_wifi,
    v_free_ram,
    v_ram_fragm,
    v_ram_max_free_block,
    v_total_size,
    v_size_sketch,
    v_free_size_sketh,
    v_flash_freq,
    v_used,
    v_cpu_freq,
    v_ver_core,
    v_ver_sdk,
    v_cpu_load_work,
    v_cpu_load_core,
    v_error,
    // // Systems
    // NTP,
    // Commands
    vc_ReadAll,
    vc_Debug,
    vc_Test,
    vc_ReadDef,
    vc_ReadROM,
    vc_ReadRAM,
    vc_Edit,
    vc_Save,
    vc_Mode,
    // last element (unuse)
    _LastElement_e_IDVarTopic // для того чтобы компилятор сообщил что в ArrVarTopic больше элементов чем в e_IDVarTopic
};
const char ArrVarTopic[_LastElement_e_IDVarTopic][MQTT_MAX_SYMBOLS] /* PROGMEM */ = // в паре с e_IDVarTopic
    {
        "",
        "#",
#ifdef USER_AREA
        // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
        // // Math
        // "Value",
        // "Quality",
        // Door
        "Latch",
        "Open",
// //DeviceESP/NTC/Settings
// "T_MQTT",
// "DataDefault",
// "DataMin",
// "DataMax",
// "DataAge",
// "Kfiltr",
// "Vcc",
// "Rs",
// "Ka",
// "Kb",
// "Kc",
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
       // Settings
        "Enable",
        "Default",
        "T_task",
        // NTP
        "Tsync",
        "IP1",
        "IP2",
        "IP3",
        "Timezone",
        // WiFi
        "SSID",
        "PASS",
        "Server",
        // MQTT
        "USER",
        // Info
        "CurrentSSID",
        "CurrentRSSI",
        "CurrentIP",
        "ReasonReset",
        "TimeReset",
        "CntReconnMQTT",
        "CntReconnWiFi",
        "FreeRAM",
        "RAMFragm",
        "RAMMaxFreeBl",
        "TotalSize",
        "SizeSketch",
        "FreeSizeSketh",
        "FlashFreq",
        "Used",
        "CPUFreq",
        "VerCore",
        "VerSDK",
        "CPUload_work",
        "CPUload_core",
        "Error",
        // // Systems
        // "NTP",
        // Commands
        "$ReadAll",
        "$Debug",
        "$Test",
        "$ReadDef",
        "$ReadROM",
        "$ReadRAM",
        "$Edit",
        "$Save",
        "$Mode",
};

struct s_element_MQTT
{
    char *topic;
    char *payload;
};

struct o_IDDirTopic
{
    e_IDDirTopic *_IDDirTopic;
    uint32_t _len_IDDirTopic;
};

enum e_state_MQTT : uint8_t
{
    _MQTT_on_disconnected, // отключились
    _MQTT_disconnected,    // отключены
    _MQTT_connecting,      // подключение
    _MQTT_on_connected,    // подключились
    _MQTT_connected        // подключены
};
extern e_state_MQTT MQTT_state;  // Состояние подключения к MQTT
extern uint16_t mqtt_count_conn; // количество (пере)подключений к серверу mqtt
// extern uint8_t idx_eth_rom; // номер записи в g_p_ethernet_settings_ROM - для получения адреса сервера MQTT

uint8_t mqtt_init(void);
void cb_ut_MQTT();
void StartMqtt(void);
void StopMqtt(void);
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void MQTTconnected(void);
// void MQTTdisconnected(void);
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);

// void t_publicMQTT_cb(void);
// void t_queueMQTTsub_cb(void);

// bool mqtt_status();
// void mqtt_callback(char *topic, uint8_t *payload, unsigned int length);

// bool char_strs_mqtt_is_equal(const char *char_str1, const char *char_str2);

String CreateTopic(o_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic);
// void CreateTopic(char *result, e_IDDirTopic _IDDirTopic, e_IDVarTopic _IDVarTopic);

extern AsyncMqttClient client;
