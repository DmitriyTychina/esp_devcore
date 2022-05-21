#pragma once

#include "global_def.h"

// #define CORE_NTP


// #define OTA_NAME "ESP-0000" // Имя сетевого порта для удаленной прошивки
// #define OTA_PASS "0000"     // Пароль доступа для удаленной прошивки

// Host name
// #define main_espname OTA_NAME
// #define my_HOST_NAME OTA_NAME
// #define my_AP_NAME main_espname
// #define my_AP_PASS "zsd9431bbb"
// #define IPAP
//     {
//         192, 168, 11, 11
//     }

// #define T_RECONMQTT_MS 998L // Период проверки коннекта с mqtt-сервером по таймеру
// #define T_alg_main_MS 500L  // Период обработки основных алгоритмов по таймеру

#define doorlatch_pin 4
#define statedoor_pin 0 // 5

/* --------------------------------
01--rst--
02  adc tout                                                ***
03--en--
04--io16 wake--
05  io14* pwm                                               ***
06  io12* pwm                                               ***
07  io13* rxd2                                              ***
08--vcc--
09--mosi--
10--miso--
11--io9--
12--io10--                                                  *** board_build.flash_mode = dio
13--int--
14--clk--
15--gnd--
16--io15 pwm txd2 /test led anode-OK/                       ***stateLED -tx debug monitor
17--io2 txd1 buildin led blue /test led anode-OK/           ***buildin led connect state -tx1 debug monitor
18--io0 button burn /test led anode-OK/ /not sonar echo/    ***mode button (burn)
19  io4* pwm                                                ***doorlatch_pin
20  io5*                                                    ***statedoor_pin
21  io3 rxd0                                                ***RX RS485
22  io1 txd0                                                ***TX RS485
*/
