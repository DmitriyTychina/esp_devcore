#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
#include <Arduino.h>
#include "my_door.h"
// #include "device_def.h"
// #include "my_debuglog.h"
#include "my_MQTT.h"
#include "my_MQTT_pub.h"
#include "my_MQTT_sub.h"
// #include "my_scheduler.h"

#define LATCH_STATE_MACHINE

#ifdef LATCH_STATE_MACHINE
enum e_state_latch
{
  _IDLE,
  _OPEN,
  _CLOSE
};
#else
bool state_latch = false;
uTask *p_ut_latch = NULL;
#endif

bool state_door; // 1 open

void latch_write(bool _state)
{
  digitalWrite(doorlatch_pin, _state ^ latch_openlevel);
}

void door_init(void)
{
  // state_latch = false;
  pinMode(doorlatch_pin, OUTPUT);
  latch_write(false); // ?????????

  pinMode(statedoor_pin, INPUT_PULLUP);
  refresh_state_door();
  // rsdebuglnF("***math_init***");
  // if(!dev_NTC.p_Math)
  // dev_NTC.p_Math->init
  // rsdebugDnfln("**********open_init**********");
}

void refresh_state_door(bool _state_door)
{
  state_door = _state_door;
  e_IDDirTopic dirs_topic[] = {d_main_topic, d_devices, d_door, d_status};
  const char *chr_state_door = (_state_door ^ statedoor_openlevel) ? "open" : "close";
  o_IDDirTopic o_dirs_topic = {dirs_topic, (uint32_t)(sizeof(dirs_topic) / sizeof(dirs_topic[0]))};
  mqtt_publish(&o_dirs_topic, v_open, chr_state_door);
  rsdebugInfln("Door[%s]", chr_state_door);
  // openlatch(); //test
}

#ifdef LATCH_STATE_MACHINE
void latch_execute(bool start)
{
  static uint32_t start_timer = 0;
  static e_state_latch state_latch = _IDLE;
  const char *chr_state_latch = NULL;
  if (start)
  {
    // rsdebugDnfln("if (start)");
    if (state_latch != _IDLE)
    {
      // rsdebugDnfln("if (start)state_latch != _IDLE");
      chr_state_latch = "wait";
    }
    else
    {
      // rsdebugDnfln("if (start)state_latch == _IDLE");
      latch_write(true);
      chr_state_latch = "open";
      state_latch = _OPEN;
      start_timer = millis();
    }
  }
  else
  {
    // rsdebugDnfln("if (!start)");
    // rsdebugDnfln("state_latch0[%d]", state_latch);
    switch (state_latch)
    {
    case _OPEN:
      // rsdebugDnfln("if (!start)_OPEN:");
      if ((millis() - start_timer) > latch_t_imp)
      {
        latch_write(false);
        chr_state_latch = "wait";
        state_latch = _CLOSE;
        start_timer = millis();
      }
      break;
    case _CLOSE:
      // rsdebugDnfln("if (!start)_CLOSE:");
      if ((millis() - start_timer) > latch_t_idle)
      {
        chr_state_latch = "close";
        state_latch = _IDLE;
      }
      break;
    default:;
      // rsdebugDnfln("if (!start)default:");
      // state_latch = _IDLE;
      // break;
    }
  }
  if (chr_state_latch != NULL)
  {
  e_IDDirTopic dirs_topic[] = {d_main_topic, d_devices, d_door, d_status};
    o_IDDirTopic o_dirs_topic = {dirs_topic, (uint32_t)(sizeof(dirs_topic) / sizeof(dirs_topic[0]))};
  mqtt_publish(&o_dirs_topic, v_latch, chr_state_latch);
    rsdebugInfln("Latch[%s]", chr_state_latch);
    // rsdebugDnfln("start[%s]", start ? "1" : "0");
    // rsdebugDnfln("state_latch[%d]", state_latch);
  }
}
#else
void cb_latch(void)
{
  // rsdebugDnfln("cb_latch(void)");
  e_IDDirTopic dirs_topic[] = {d_main_topic, d_devices, d_door, d_status};
  const char *chr_state_latch;
  if (state_latch)
  {
    // rsdebugDnfln("cb_latch(void) if (state_latch)");
    state_latch = false;
    latch_write(state_latch);
    chr_state_latch = "wait";
    p_ut_latch->setInterval(latch_t_idle);
    p_ut_latch->enable();
  }
  else
  {
    // rsdebugDnfln("cb_latch(void) if (!!!!!!!!state_latch)");
    // p_ut_latch->disable();
    delete p_ut_latch;
    p_ut_latch = NULL;
    chr_state_latch = "close";
  }
  mqtt_publish(dirs_topic, v_latch, chr_state_latch);
  rsdebugInfln("Latch[%s]", chr_state_latch);
}
#endif
#ifdef LATCH_STATE_MACHINE

#else
void latch_start(void)
{
  // rsdebugDnfln("latch_start(void)");
  e_IDDirTopic dirs_topic[] = {d_main_topic, d_devices, d_door, d_status};
  const char *chr_state_latch = NULL;
  if (p_ut_latch)
  {
    // rsdebugDnfln("latch_start(void) chr_state_latch = wait;");
    chr_state_latch = "wait";
  }
  else
  {
    state_latch = true;
    latch_write(state_latch);
    chr_state_latch = "open";
    // rsdebugDnfln("latch_start(void) chr_state_latch = open;");
    p_ut_latch = new uTask(1, &cb_latch);
    p_ut_latch->setInterval(latch_t_imp);
    p_ut_latch->enable();
  }
  if (chr_state_latch != NULL)
  {
    mqtt_publish(dirs_topic, v_latch, chr_state_latch);
    rsdebugInfln("Latch[%s]", chr_state_latch);
  }
}
#endif

void cb_ut_door(void)
{
  bool current_state_door = digitalRead(statedoor_pin);
  if (state_door != current_state_door) // изменилось состояние двери
  {
    refresh_state_door(current_state_door);
  }
#ifdef LATCH_STATE_MACHINE
  latch_execute(false);
#else
  if (p_ut_latch != NULL)
    p_ut_latch->execute();
#endif
}

void cb_MQTT_com_Door(s_element_MQTT _element) // получили команду на открытие замка
{
  // const char *payload = _element.payload;
  if (is_equal_enable(_element.payload)) // открыть дверь
  {
    // rsdebugDnfln("--->latch_execute(true)");
#ifdef LATCH_STATE_MACHINE
    latch_execute(true);
    // rsdebugDnfln("latch_execute(true)--->");
#else
    latch_start();
#endif
    mqtt_publish(_element.topic, "ok");
  }
}

void onStartMQTT_pub_door(void)
{
  // rsdebugDnfln("onStartMQTT_pub_door");
  refresh_state_door();
  e_IDDirTopic dirs_topic[] = {d_main_topic, d_devices, d_door, d_commands};
  o_IDDirTopic o_dirs_topic = {dirs_topic, (uint32_t)(sizeof(dirs_topic) / sizeof(dirs_topic[0]))};
  mqtt_publish_ok(&o_dirs_topic, v_latch);
  o_dirs_topic._IDDirTopic[3] = {d_status};
  mqtt_publish(&o_dirs_topic, v_latch, "close");
}

// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
