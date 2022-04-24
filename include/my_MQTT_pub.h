#pragma once

#include "my_MQTT.h"

void MQTT_pub_allSettings(bool fromROM);

// void MQTT_pub_Commands_ok(e_IDVarTopic _IDVarTopic);
// void MQTT_pub_Settings_ok(e_IDVarTopic _IDVarTopic);
void MQTT_pub_Info_TimeReset(void);
void MQTT_pub_allInfo(bool reset);

// void onMessagePublish(uint16_t packetId);

#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
// void MQTT_pub_Settings_NTP_IP(e_IDDirTopic *dirs_topic, e_IDVarTopic _IDVarTopic, uint8_t num);
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, int32_t payload);
void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, uint32_t payload);
void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, float payload, const char *format = "%.2f");
void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, bool payload);
// void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, float payload);
void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, const char *payload);
void mqtt_publish(const char *_topic, const char *_payload);
void mqtt_publish_ok(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic);
void mqtt_publish_no(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic);
