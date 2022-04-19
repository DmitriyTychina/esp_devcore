#ifndef my_door_h
#define my_door_h

#include <Arduino.h>
#include "device_def.h"
#include "my_debuglog.h"
#include "my_MQTT.h"
#include "my_scheduler.h"
// #include "main.h"
// #include "MQTT_pub.h"
// #include "MQTT_com.h"
// #include "my_EEPROM.h"
// #include "my_NTC.h"
// #include "SensorData.h"


#define door_TtaskDefault 50
// должно быть latch_t_wait > latch_t_imp
#define latch_t_idle 4000
#define latch_t_imp 400
#define latch_openlevel 1
#define statedoor_openlevel 0

void door_init(void);
void refresh_state_door(bool _state_door = digitalRead(statedoor_pin));
void latch_execute(void);
void cb_door(void);
void MQTT_com_Door(s_element_Queue_MQTT _element);
void onStartMQTT_pub_door(void);

#endif // my_door_h