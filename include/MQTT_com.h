#ifndef MQTT_com_h
#define MQTT_com_h

#include "my_MQTT.h"
#include "my_scheduler.h"

#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
// void MQTT_com_NTC_Settings(s_element_Queue_MQTT _element);
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

// void MQTT_com_Debug(s_element_Queue_MQTT _element);
// void MQTT_com_SaveSettings(s_element_Queue_MQTT _element);
void MQTT_com_Settings(s_element_Queue_MQTT _element);
bool Modify_task_tTask(e_IDDirTopic *_dir_topic, const char *_payload, uTask *_task, uint32_t _tMin, uint32_t _tMax, uint32_t *_tTask);

#endif //MQTT_com_h