#pragma once

// #include "global.h"
#include <NtpClientLib.h>

#define NTP_TtaskDefault 251

struct s_NTP_settings_ROM
{
    char serversNTP[3][24] = {"132.163.96.1", "ntp2.stratum2.ru", "pool.ntp.org"}; // NTP-сервера времени
    int8_t timezone = 3;                                                           // timezone // Часовой пояс (+3 - Москва)
    uint16_t T_syncNTP = 60;                                                       // Период синхронизации времени с NTP-сервером в минутах
    uint32_t Ttask = NTP_TtaskDefault;
};

void init_NTP(void);
void init_NTP_with_WiFi(void);
void cb_ut_NTP(void);

extern NTPClient NTP;
extern s_NTP_settings_ROM *g_p_NTP_settings_ROM;
