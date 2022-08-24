#pragma once

#define SysMon_TtaskDefault 10000

extern bool v_b_SysMon_info_to_mqtt;
extern bool v_b_SysMon_info_to_rsdebug;

struct s_Info_t
{
    int8_t WiFiRSSI;
    uint32_t FreeRAM;
#if defined(ESP32)
#elif defined(ESP8266)
    uint32_t RAMFragmentation;
    uint32_t MaxFreeBlockSize;
#endif
    unsigned long cpuTot;
    float CPUload;
    float CPUCore;
    float CPUidle;
};

void SysMon_Init(void);
void cb_ut_sysmon(void);
