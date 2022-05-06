#pragma once

#include "my_MQTT.h"
#include "my_scheduler.h"

extern char last_topic[];
extern char last_payload[];

#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
// void MQTT_com_NTC_Settings(s_element_MQTT _element);
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

typedef void (*t_Callback)(s_element_MQTT);
struct s_SubscribeElement
{
    e_IDDirTopic IDDirTopic[MQTT_MAX_LEVEL];
    e_IDVarTopic IDVarTopic;
    uint8_t mqttQOS;
    t_Callback Callback;
};
void onMessageReceived(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

void MQTT_sub_All();

void cb_MQTT_com_Debug(s_element_MQTT _element);
void cb_MQTT_com_Info(s_element_MQTT _element);
void cb_MQTT_com_ReadDef(s_element_MQTT _element);
void cb_MQTT_com_ReadROM(s_element_MQTT _element);
void cb_MQTT_com_ReadRAM(s_element_MQTT _element);
void cb_MQTT_com_Save(s_element_MQTT _element);
void cb_MQTT_com_set_WiFi(s_element_MQTT _element);
void cb_MQTT_com_set_MQTT(s_element_MQTT _element);
void cb_MQTT_com_set_NTP(s_element_MQTT _element);
void cb_MQTT_com_set_OTA(s_element_MQTT _element);
void cb_MQTT_com_set_RSDebug(s_element_MQTT _element);
void cb_MQTT_com_set_ROM(s_element_MQTT _element);
void cb_MQTT_com_set_SysMon(s_element_MQTT _element);

void cb_MQTT_sub_Info(s_element_MQTT _element);
void cb_MQTT_sub_Settings(s_element_MQTT _element);
// void MQTT_com_Debug(s_element_MQTT _element);
// void MQTT_com_SaveSettings(s_element_MQTT _element);

void mqtt_subscribe(s_SubscribeElement _sub_element);
void mqtt_unsubscribe(s_SubscribeElement _sub_element);

bool is_equal_ok(const char *char_str);
bool is_equal_no(const char *char_str);
bool is_equal_enable(const char *char_str);
bool is_equal_disable(const char *char_str);

bool Modify_string(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, char _data[]);
bool Modify_pass(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, char _data[]);
bool Modify_task_tTask(o_IDDirTopic *_dir_topic, const char *_payload, uTask *_task, uint32_t _tMin, uint32_t _tMax, uint32_t *_tTask);
uint8_t Modify_num(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, int32_t _tMin, int32_t _tMax, int32_t *_mem_num, int32_t _real_val);
uint8_t Modify_num(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, uint32_t _tMin, uint32_t _tMax, uint32_t *_mem_num, uint32_t _real_val);
uint8_t Modify_num(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, uint16_t _tMin, uint16_t _tMax, uint16_t *_mem_num, uint16_t _real_val);
uint8_t Modify_num(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, int8_t _tMin, int8_t _tMax, int8_t *_mem_num, int8_t _real_val);
uint8_t Modify_num(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, uint8_t _tMin, uint8_t _tMax, uint8_t *_mem_num, uint8_t _real_val);
uint8_t Modify_bool(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, bool *_mem_bool, bool _real_val);
