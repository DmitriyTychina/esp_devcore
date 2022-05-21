#pragma once
#ifdef CORE_NTP
#include <NtpClientLib.h>

#define NTP_TtaskDefault 251

void init_NTP(void);
void init_NTP_with_WiFi(void);
void cb_ut_NTP(void);

extern NTPClient NTP;
#endif