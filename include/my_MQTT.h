#ifndef my_MQTT_h
#define my_MQTT_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "AsyncMqttClient.h"
#include "device_def.h"

#define MQTT_TtaskDefault 53

// #define MQTT_QUEUE

// не более 255, индекс массива везде 8 бит
enum e_IDDirTopic : uint8 // в паре с ArrDirTopic[]
{
    d_empty,
    _main_topic,
    _State,
    _Status,
    _Devices,
#ifdef USER_AREA
    // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
    _Door,
// // ESP/NTC - датчик температуры
// _ESP_NTC_Math,
// _ESP_NTC_Settings,
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
    _Info,
    _Settings,
    // Settings/WIFI
    _WIFI,
    _AP1,
    _AP2,
    _AP3,
    _AP4,
    _AP5,
    _MQTT,
    _OTA,
    _NTP,
    // Settings/RSdebug
    _RSdebug,
    _Rdebug,
    _Sdebug,
    _SysMon,
    _Commands,
    // last element (unuse)
    _LastElement_e_IDDirTopic // для того чтобы компилятор сообщил что в ArrDirTopic больше элементов чем в e_IDDirTopic
};
const char ArrDirTopic[_LastElement_e_IDDirTopic][10] /* PROGMEM */ = // в паре с e_IDDirTopic
    {
        "",
        OTA_NAME,
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
        // Settings/RSdebug
        "RSdebug",
        "Rdebug",
        "Sdebug",
        "SysMon",
        "Commands"};

// не более 255, индекс массива везде используем 8бит
enum e_IDVarTopic : uint8 // в паре с ArrVarTopic[]
{
    v_empty,
    _all,
#ifdef USER_AREA
    // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
    // // Math
    // _Value,
    // _Quality,
    // Door
    _Latch,
    _Open,
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
    _Enable,
    _Default,
    _T_task,
    // NTP
    _Tsync,
    _IP1,
    _IP2,
    _IP3,
    _Timezone,
    // Wifi
    _SSID,
    _PASS,
    _IP_serv,
    // MQTT
    _USER,
    // Info/
    _CurrentWiFiAP,
    _CurrentIP,
    _ReasonReset,
    _TimeReset,
    _CntReconnMQTT,
    _CntReconnWiFi,
    _FreeRAM,
    _CPUload,
    _CPUcore,
    // // Systems
    // NTP,
    // Commands
    _Debug,
    _ReadDflt,
    _ReadCrnt,
    _Edit,
    _Save,
    _Mode,
    // last element (unuse)
    _LastElement_e_IDVarTopic // для того чтобы компилятор сообщил что в ArrVarTopic больше элементов чем в e_IDVarTopic
};
const char ArrVarTopic[_LastElement_e_IDVarTopic][14] /* PROGMEM */ = // в паре с e_IDVarTopic
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
        "IP_serv",
        // MQTT
        "USER",
        // Info
        "CurrentWiFiAP",
        "CurrentIP",
        "ReasonReset",
        "TimeReset",
        "CntReconnMQTT",
        "CntReconnWiFi",
        "FreeRAM",
        "CPUload",
        "CPUcore",
        // // Systems
        // "NTP",
        // Commands
        "Debug",
        "ReadDflt",
        "ReadCrnt",
        "Edit",
        "Save",
        "Mode",
};

struct s_element_MQTT
{
    char *topic;
    char *payload;
};

enum e_state_MQTT : uint8_t
{
    _MQTT_disconnected, // отключены
    _MQTT_connecting,   // подключение
    _MQTT_on_connected, // подключились
    _MQTT_connected     // подключены
};
extern e_state_MQTT MQTT_state;  // Состояние подключения к MQTT
extern uint16_t mqtt_count_conn; // количество (пере)подключений к серверу mqtt
// extern uint8_t idx_eth_rom; // номер записи в g_p_ethernet_settings_ROM - для получения адреса сервера MQTT

typedef void (*t_Callback)(s_element_MQTT);
#define size_IDDirTopic 4
struct s_SubscribeElement
{
    e_IDDirTopic IDDirTopic[size_IDDirTopic];
    e_IDVarTopic IDVarTopic;
    uint8_t mqttQOS;
    t_Callback Callback;
};

void mqtt_init(uint8_t _idx);
void onMqttConnect(bool sessionPresent);
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);

void StartMqtt(void);
void StopMqtt(void);
void cb_ut_MQTT();
// void t_publicMQTT_cb(void);
// void t_queueMQTTsub_cb(void);

void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, bool payload);
void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, int32_t payload);
void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, uint32_t payload);
void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, float payload, const char *format = "%.2f");
// void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, float payload);
void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, const char *payload);
void mqtt_publish(const char *_topic, const char *_payload);
void mqtt_publish_ok(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic);
void mqtt_publish_no(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic);

// bool mqtt_status();
// void mqtt_callback(char *topic, uint8_t *payload, unsigned int length);

// bool char_strs_mqtt_is_equal(const char *char_str1, const char *char_str2);
bool is_equal_ok(const char *char_str);
bool is_equal_no(const char *char_str);
bool is_equal_enable(const char *char_str);
bool is_equal_disable(const char *char_str);

String CreateTopic(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic);
// void CreateTopic(char *result, e_IDDirTopic _IDDirTopic, e_IDVarTopic _IDVarTopic);

void mqtt_subscribe(s_SubscribeElement _sub_element);
void mqtt_unsubscribe(s_SubscribeElement _sub_element);

extern AsyncMqttClient client;

#endif // my_MQTT_h