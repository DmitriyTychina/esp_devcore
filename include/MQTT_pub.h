#ifndef MQTT_pub_h
#define MQTT_pub_h

#include "my_MQTT.h"

void MQTT_pub_allSettings(bool fromROM);

// void MQTT_pub_Commands_ok(e_IDVarTopic _IDVarTopic);
// void MQTT_pub_Settings_ok(e_IDVarTopic _IDVarTopic);
void MQTT_pub_Info_TimeReset(void);
#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
// void MQTT_pub_Settings_NTP_IP(e_IDDirTopic *dir_topic, e_IDVarTopic _IDVarTopic, uint8_t num);
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

#endif //MQTT_pub_h