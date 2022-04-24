#include "my_led.h"
// #include <RGB.h>

Task t_led(TICK_MS, TASK_FOREVER, &cb_led, &ts);  //defLed
Task t_ledSetMode(1001, TASK_FOREVER, &cb_set_mode_led, &ts, true); //defLed

GRGB led_state(2, 15, 0); // куда подключены цвета (R, G, B)
s_cycle dev_led[Q_KVANTS];
int dev_led_index;

// по умолчанию
s_kvant a_led_0[] =
    {
        {kv_stop, BLACK, 0}};

// для отбражения статуса соединия state_conn
s_kvant a_led_state_conn_initOTA[] =
    {
        // {kv_blink, BLACK, 0},
        {kv_blink, VIOLET, 20},
        {kv_blink, BLACK, 20},
        {kv_blink, VIOLET, 20},
        {kv_blink, BLACK, 20},
        {kv_blink, VIOLET, 20},
        {kv_blink, BLACK, 900},
        {kv_stop, BLACK, 0}};

s_kvant a_led_state_conn_connectingWIFI[] =
    {
        // {kv_blink, BLACK, 0},
        {kv_blink, BLUE, 20},
        {kv_blink, BLACK, 20},
        {kv_blink, BLUE, 20},
        {kv_blink, BLACK, 20},
        {kv_blink, BLUE, 20},
        {kv_blink, BLACK, 20},
        {kv_blink, BLUE, 20},
        {kv_blink, BLACK, 20},
        {kv_blink, BLUE, 20},
        {kv_blink, BLACK, 20},
        {kv_blink, BLUE, 20},
        {kv_blink, BLACK, 20},
        {kv_blink, BLUE, 20},
        {kv_blink, BLACK, 20},
        {kv_blink, BLUE, 20},
        {kv_blink, BLACK, 20},
        {kv_blink, BLUE, 20},
        {kv_blink, BLACK, 20},
        {kv_blink, BLUE, 20},
        {kv_blink, BLACK, 20},
        {kv_stop, BLACK, 0}};

s_kvant a_led_state_conn_connectingServ[] =
    {
        // {kv_blink, BLACK, 0},
        {kv_blink, BLUE, 100},
        {kv_blink, BLACK, 100},
        {kv_blink, BLUE, 100},
        {kv_blink, BLACK, 100},
        {kv_blink, BLUE, 100},
        {kv_blink, BLACK, 100},
        {kv_blink, BLUE, 100},
        {kv_blink, BLACK, 100},
        {kv_blink, BLUE, 100},
        {kv_blink, BLACK, 100},
        {kv_stop, BLACK, 0}};

s_kvant a_led_state_conn_connectedServ[] =
    {
        // {kv_blink, BLACK, 0},
        {kv_fade, BLUE, 1000},
        {kv_fade, BLACK, 1000},
        {kv_stop, BLACK, 0}};

s_kvant a_led_state_conn_uploadOverOTA[] =
    {
        // {kv_blink, BLACK, 0},
        {kv_blink, VIOLET, 10},
        {kv_blink, BLACK, 10},
        {kv_stop, BLACK, 0}};

s_kvant a_led_state_conn_WebConfigAP[] =
    {
        // {kv_blink, BLACK, 0},
        {kv_fade, BLUE, 100},
        {kv_fade, RED, 100},
        {kv_fade, BLUE, 100},
        {kv_fade, RED, 100},
        {kv_fade, BLUE, 100},
        {kv_fade, RED, 100},
        {kv_fade, BLUE, 100},
        {kv_fade, RED, 100},
        {kv_fade, BLUE, 100},
        {kv_fade, RED, 100},
        {kv_stop, BLACK, 0}};

// для отбражения режима работы контроллера mode_mcu
s_kvant a_led_mode_mcu_start[] =
    {
        // {kv_blink, BLACK, 0},
        {kv_blink, WHITE, 200},
        {kv_blink, BLACK, 300},
        {kv_blink, RED, 300},
        {kv_blink, BLACK, 300},
        {kv_blink, GREEN, 300},
        {kv_blink, BLACK, 300},
        {kv_blink, BLUE, 300},
        {kv_blink, BLACK, 300},
        {kv_stop, BLACK, 0}};

s_kvant a_led_mode_mcu_autonomy[] =
    {
        // {kv_blink, BLACK, 0},
        {kv_fade, GREEN, 1000},
        {kv_fade, BLACK, 1000},
        {kv_stop, BLACK, 0}};

s_kvant a_led_mode_mcu_remote[] =
    {
        // {kv_blink, BLACK, 0},
        {kv_blink, GREEN, 500},
        {kv_blink, BLACK, 1500},
        {kv_stop, BLACK, 0}};

s_kvant a_led_mode_mcu_arm[] =
    {
        // {kv_blink, BLACK, 0},
        {kv_blink, GREEN, 100},
        {kv_blink, BLACK, 900},
        {kv_blink, GREEN, 100},
        {kv_blink, BLACK, 900},
        {kv_stop, BLACK, 0}};

// для отбражения исключительных ситуаций
s_kvant a_led_mode_ex_warn_TO[] =
    {
        // {kv_blink, BLACK, 0},
        {kv_blink, YELLOW, 500},
        {kv_blink, BLACK, 500},
        {kv_stop, BLACK, 0}};

s_kvant a_led_mode_ex_warn_device[] =
    {
        {kv_fade, RED, 500},
        {kv_fade, BLACK, 500},
        {kv_stop, BLACK, 0}};

s_kvant a_led_mode_ex_err_device[] =
    {
        {kv_blink, RED, 500},
        {kv_blink, BLACK, 500},
        {kv_stop, BLACK, 0}};

s_kvant a_led_mode_ex_err_fault[] =
    {
        {kv_blink, RED, 50},
        {kv_blink, BLACK, 50},
        {kv_blink, RED, 50},
        {kv_blink, BLACK, 50},
        {kv_blink, RED, 50},
        {kv_blink, BLACK, 50},
        {kv_blink, RED, 50},
        {kv_blink, BLACK, 50},
        {kv_blink, RED, 50},
        {kv_blink, BLACK, 50},
        {kv_blink, RED, 50},
        {kv_blink, BLACK, 50},
        {kv_blink, RED, 50},
        {kv_blink, BLACK, 50},
        {kv_blink, RED, 50},
        {kv_blink, BLACK, 50},
        {kv_blink, RED, 50},
        {kv_blink, BLACK, 50},
        {kv_blink, RED, 50},
        {kv_blink, BLACK, 50},
        {kv_stop, BLACK, 0}};

// s_kvant a_led_cycle2[] =
//     {
//         {kv_blink, RED, 0},
//         {kv_blink, GREEN, 300},
//         {kv_blink, BLACK, 300},
//         {kv_blink, GREEN, 300},
//         {kv_blink, BLACK, 300},
//         {kv_blink, GREEN, 300},
//         {kv_blink, BLACK, 300},
//         // {kv_fade, RED, 1000},
//         // {kv_fade, ORANGE, 500},
//         // {kv_fade, YELLOW, 500},
//         // {kv_fade, GREEN, 1000},
//         // {kv_fade, BLUE, 1000},
//         // {kv_fade, CAPRI, 500},
//         // {kv_fade, VIOLET, 500},
//         // {kv_fade, WHITE, 500},
//         // // {kv_fade, BBB, 1000},
//         {kv_stop, BLACK, 0}};

void init_dev_led(byte tick_ms)
{
  dev_led_index = 0;
  for (int i = 0; i < Q_KVANTS; i++)
  {
    dev_led[i].kvant_index = 0;
    dev_led[i].kvants = &a_led_0[0];
    dev_led[i].size = sizeof(a_led_0) / sizeof(a_led_0[0]);
  }
  // a_led_cycle = &a_led_cycle2[0];
  // dev_led.size = sizeof(a_led_cycle2) / sizeof(a_led_cycle2[0]);
  led_state.set_tick(tick_ms);
}

void set_mode_led(e_state_conn _state_conn, e_mode_mcu _mode_mcu, e_state_ex _state_ex)
{
  switch (_state_conn)
  {
  case mc_initOTA:
    SET_LED_CYCLE(dev_led[0], a_led_state_conn_initOTA)
    break;
  case mc_connectingWIFI:
    SET_LED_CYCLE(dev_led[0], a_led_state_conn_connectingWIFI)
    break;
  case mc_connectingServ:
    SET_LED_CYCLE(dev_led[0], a_led_state_conn_connectingServ)
    break;
  case mc_connectedServ:
    SET_LED_CYCLE(dev_led[0], a_led_state_conn_connectedServ)
    break;
  case mc_uploadOverOTA:
    SET_LED_CYCLE(dev_led[0], a_led_state_conn_uploadOverOTA)
    break;
  case mc_WebConfigAP:
    SET_LED_CYCLE(dev_led[0], a_led_state_conn_WebConfigAP)
    break;
  default:
    SET_LED_CYCLE(dev_led[0], a_led_0)
  }
  switch (_mode_mcu)
  {
  case mm_start:
    SET_LED_CYCLE(dev_led[1], a_led_mode_mcu_start)
    break;
  case mm_autonomy:
    SET_LED_CYCLE(dev_led[1], a_led_mode_mcu_autonomy)
    break;
  case mm_remote:
    SET_LED_CYCLE(dev_led[1], a_led_mode_mcu_remote)
    break;
  case mm_arm:
    SET_LED_CYCLE(dev_led[1], a_led_mode_mcu_arm)
    break;
  default:
    SET_LED_CYCLE(dev_led[1], a_led_0)
  }
  switch (_state_ex)
  {
  case warn_TO_filtr:
  case warn_TO_tank:
    SET_LED_CYCLE(dev_led[2], a_led_mode_ex_warn_TO)
    break;
  case warn_device:
    SET_LED_CYCLE(dev_led[2], a_led_mode_ex_warn_device)
    break;
  case err_device:
    SET_LED_CYCLE(dev_led[2], a_led_mode_ex_err_device)
    break;
  case err_fault:
    SET_LED_CYCLE(dev_led[2], a_led_mode_ex_err_fault)
    break;
  default:
  case warn_none:
    SET_LED_CYCLE(dev_led[2], a_led_0)
  }
  // warn_none,     // аварий и предупреждений нет
  // warn_TO_filtr, // нужна замена фильтров
  // warn_TO_tank,  // нужно помыть бочку
  // warn_device,   // не критичный отказ датчика - можно работать дальше
  // err_device,    // критичный отказ датчика - ??? определиться что делать
  // err_fault      // критическая авария - закрыть ввод
}

int next_index()
{
  if (++dev_led[dev_led_index].kvant_index >= dev_led[dev_led_index].size)
  {
    dev_led[dev_led_index].kvant_index = 0;
    if (++dev_led_index >= Q_KVANTS)
    {
      dev_led_index = 0;
      return -1;
    }
  }
  return 0;
}

int tick_led()
{
  // Serial.print("dev_led_index=");
  // Serial.print(dev_led_index);
  // Serial.print("                   .kvant_index=");
  // Serial.println(dev_led[dev_led_index].kvant_index);
  if (dev_led[dev_led_index].kvants == NULL)
    return -1;
  // Serial.print("kvant_index=");
  // Serial.println(dev_led.kvant_index);
  // Serial.print(".mode=");
  // Serial.println(a_led_cycle[dev_led.kvant_index].mode);
  int i = dev_led[dev_led_index].kvant_index;
  colors _color = dev_led[dev_led_index].kvants[i].color;
  uint16_t _ms = dev_led[dev_led_index].kvants[i].ms;
  switch (dev_led[dev_led_index].kvants[i].mode)
  {
  default:
  case kv_stop:
    led_state.setHEX(_color);
    return next_index();
    break;
  case kv_fade:
    switch (led_state.fade_fs_tick())
    {
    case e_stop:
      led_state.fadeTo_fs_init(_color, _ms);
      break;
    default:
    case e_work:
      break;
    case e_end:
      // if (dev_led.kvant_index++ >= dev_led.size)
      //   dev_led.kvant_index = 0;
      next_index();
      break;
    }
    return TICK_MS;
    break;
  case kv_blink:
    if (_ms != 0)
      led_state.setHEX(_color);
    // if (dev_led.kvant_index++ >= dev_led.size)
    //   dev_led.kvant_index = 0;
    next_index();
    return _ms;
    break;
  case kv_skip:
    // if (dev_led.kvant_index++ >= dev_led.size)
    //   dev_led.kvant_index = 0;
    next_index();
    return TICK_MS;
    break;
  }
}
