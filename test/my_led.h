#ifndef my_led_h
#define my_led_h

#include <Arduino.h>
#include <RGB.h>
// #include "main.h"
#include "main_include.h"

#include <TaskScheduler.h>
// #include "my_scheduler.h"

extern Scheduler ts;

/*  Доступные цвета
  WHITE   // белый
  SILVER  // серебро
  GRAY    // серый
  BLACK   // чёрный
  RED     // красный
  MAROON  // бордовый
  YELLOW  // жёлтый
  OLIVE   // олива
  LIME    // лайм
  GREEN   // зелёный
  AQUA    // аква
  TEAL    // цвет головы утки чирка https://odesign.ru/teal-color/
  BLUE    // синий
  NAVY    // тёмно-синий
  PINK    // розовый
  PURPLE  // пурпурный
  ORANGE  // оранжевый
  VIOLET  // фиолетовый
  CAPRI   // голубой
  */

#define TICK_MS 20
#define Q_KVANTS 3
#define SET_LED_CYCLE(_cycle, _data)                    \
    {                                                   \
        _cycle.kvants = &_data[0];                      \
        _cycle.size = sizeof(_data) / sizeof(_data[0]); \
        _cycle.kvant_index = 0;                         \
    }

enum e_kvant_mode
{
    kv_stop,
    kv_fade,
    kv_blink,
    kv_skip
};

enum e_cycle_mode
{
    // cy_stop_kvant, //?
    cy_stop,
    cy_work
};

struct s_kvant
{
    e_kvant_mode mode;
    colors color;
    uint16_t ms;
};

struct s_cycle
{
    s_kvant *kvants; // указатель на массив квантов
    int size;        // количество квантов
    int kvant_index; // индекс текущего кванта
    // e_cycle_mode cycle_mode; // текущий режим цикла
    // int _cnt;                // переменная для осчета тиков
    // int32_t repeat_cycle;    // повтор цикла -1 не повторять, 0 повторять сразу или через repeat_cycle тиков, но не раньше чем закончится цикл
};

void init_dev_led(byte tick_ms);
// void wait_stop_cycle_led(); //del?
void set_mode_led(e_state_conn _state_conn, e_mode_mcu _mode_mcu, e_state_ex _state_ex);
int tick_led();
// enum e_state_conn
// {
//     mc_init,
//     mc_connectingWIFI,
//     mc_connectingServ,
//     mc_connectedServ,
//     mc_uploadOverOTA
// };

struct s_led_state
{
    uint32_t us_timer = 0;
    uint8_t curr_state = 0;
    uint32_t states[2];
};

// #define B_LED_BUILTIN_ON 0
// #define B_LED_BUILTIN_OFF 1

// #define B_LED_BUILTIN 2

// struct s_B_led
// {
//     uint32_t us_timer=0;
//     uint8_t curr_state=0;
//     uint32_t states[2];
// };

// void initB_led();
// //void init3color_led(int pin);
// void setB_led(uint32_t on_ms, uint32_t off_ms);
// void updateB_led();

// #define B_led_on() digitalWrite(B_LED_BUILTIN, B_LED_BUILTIN_ON)
// #define B_led_off() digitalWrite(B_LED_BUILTIN, B_LED_BUILTIN_OFF)
// #define B_led_toggle() digitalWrite(B_LED_BUILTIN, !digitalRead(B_LED_BUILTIN))

// void updateB_led();

#endif // my_led_h