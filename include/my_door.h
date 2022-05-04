#pragma once

#include <Arduino.h>
#include "device_def.h"
#include "my_debuglog.h"
#include "my_MQTT.h"
#include "my_MQTT_pub.h"
#include "my_MQTT_sub.h"
#include "my_scheduler.h"
// #include "main.h"
// #include "my_MQTT_pub.h"
// #include "my_MQTT_sub.h"
// #include "my_EEPROM.h"
// #include "my_NTC.h"
// #include "SensorData.h"


#define door_TtaskDefault 50
// должно быть latch_t_wait > latch_t_imp
#define latch_t_idle 4000
#define latch_t_imp 400
#define latch_openlevel 0
#define statedoor_openlevel 0

void door_init(void);
void refresh_state_door(bool _state_door = digitalRead(statedoor_pin));
void latch_execute(void);
void cb_ut_door(void);
void cb_MQTT_com_Door(s_element_MQTT _element);
void onStartMQTT_pub_door(void);
