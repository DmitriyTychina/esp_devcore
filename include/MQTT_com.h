#ifndef MQTT_com_h
#define MQTT_com_h

#include "my_MQTT.h"
#include "my_scheduler.h"

#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
// void MQTT_com_NTC_Settings(s_element_MQTT _element);
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

// void MQTT_com_Debug(s_element_MQTT _element);
// void MQTT_com_SaveSettings(s_element_MQTT _element);
void cb_MQTT_com_Info(s_element_MQTT _element);
void cb_MQTT_com_Settings(s_element_MQTT _element);
bool Modify_string(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, char _data[]);
bool Modify_pass(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, char _data[]);
bool Modify_task_tTask(e_IDDirTopic *_dir_topic, const char *_payload, uTask *_task, uint32_t _tMin, uint32_t _tMax, uint32_t *_tTask);
uint8_t Modify_num(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, int32_t _tMin, int32_t _tMax, int32_t *_mem_num, int32_t _real_val);
uint8_t Modify_num(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, uint32_t _tMin, uint32_t _tMax, uint32_t *_mem_num, uint32_t _real_val);
uint8_t Modify_num(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, uint16_t _tMin, uint16_t _tMax, uint16_t *_mem_num, uint16_t _real_val);
uint8_t Modify_num(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, int8_t _tMin, int8_t _tMax, int8_t *_mem_num, int8_t _real_val);
uint8_t Modify_num(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, uint8_t _tMin, uint8_t _tMax, uint8_t *_mem_num, uint8_t _real_val);
uint8_t Modify_bool(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, bool *_mem_bool, bool _real_val);

#endif // MQTT_com_h