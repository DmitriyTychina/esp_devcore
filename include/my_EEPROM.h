#ifndef my_EEPROM_h
#define my_EEPROM_h

#include <Arduino.h>
#include "main.h"
#include "my_wifi.h"
#include "my_NTP.h"
#include "my_sysmon.h"
#include "my_debuglog.h"
#include "my_ota.h"

#define emptymemory_TtaskDefault 20000

struct s_sys_settings_ROM
{
    // uint16_t crc;
    uint32_t OTA_Ttask = OTA_TtaskDefault;
    uint32_t RSDebug_Ttask = RSDebug_TtaskDefault;
    uint32_t SysMon_Ttask = SysMon_TtaskDefault;
    bool RSDebug_SDebug = true;
    bool RSDebug_RDebug = true;
    // e_microprogramm programm_autonomy; // программа автономного режима, работет не зависимо от связи с сервером
    // e_microprogramm programm_remote;   // программа дистанционного режима, работет при отсутствии связи с сервером
    // e_microprogramm programm_arm;      // программа ручного режима, работет при отсутствии связи с сервером
    // char namesystem[16] = "Water"; // Имя системы // Один раз по MQTT
    // uint8_t ver[6] = "00.20";      // Версия ПО // Один раз по MQTT
};

struct s_all_settings_ROM
{
    uint16_t crc;
    // uint16_t len;
    // s_ethernet_settings_ROM ethernet_settings_ROM1;
    s_ethernet_settings_ROM ethernet_settings_ROM;
    s_NTP_settings_ROM NTP_settings_ROM;
    // s_sys_settings_ROM sys_settings_ROM1;
    s_sys_settings_ROM sys_settings_ROM;
    // uint16_t crc3=1;
    // uint16_t crc2=0;
    // s_NTC_settings_ROM NTC_settings_ROM;
};

extern s_all_settings_ROM *s1;
extern s_sys_settings_ROM *g_p_sys_settings_ROM;

// соблюдаем порядок вычисления адреса - не нужно
// #define addr_all_settings_ROM 0
#define len_all_settings_ROM sizeof(*s1) // всегда первая CRC
#define len_crc sizeof(s1->crc)

// #define addr_ethernet_settings_ROM (addr_all_settings_ROM+len_crc) // первая после CRC
#define addr_ethernet_settings_ROM ((uint8_t *)&s1->ethernet_settings_ROM - (uint8_t *)s1) // первая после CRC
// #define len_ethernet_settings_ROM ((char*)&s1->NTP_settings_ROM-(char*)&s1->ethernet_settings_ROM)
#define len_ethernet_settings_ROM sizeof(s1->ethernet_settings_ROM)

// #define addr_NTP_settings_ROM (addr_ethernet_settings_ROM + len_ethernet_settings_ROM) // такая запись давала адрес на 2 байта меньше (???)
#define addr_NTP_settings_ROM ((uint8_t *)&s1->NTP_settings_ROM - (uint8_t *)s1) // только такая запись давала реальный адрес
#define len_NTP_settings_ROM sizeof(s1->NTP_settings_ROM)

// // #define addr_NTC_settings_ROM (addr_NTP_settings_ROM + len_NTP_settings_ROM)
// #define addr_NTC_settings_ROM ((char *)&s1->NTC_settings_ROM - (char *)s1)
// #define len_NTC_settings_ROM sizeof(s1->NTC_settings_ROM)

// // #define addr_sys_settings_ROM (addr_NTC_settings_ROM + len_NTC_settings_ROM)
#define addr_sys_settings_ROM ((uint8_t *)&s1->sys_settings_ROM - (uint8_t *)s1)
#define len_sys_settings_ROM sizeof(s1->sys_settings_ROM)

// void Print_s_NTP_settings(s_NTP_settings_ROM *_s);

void ROMVerifySettingsElseSaveDefault(bool force = false);
void all_settings_Default(s_all_settings_ROM *__p_all_settings_ROM);

// void GetSettings(void *_p_settings_ROM, uint16_t addr, uint16_t len);
// void EmptySettings(void *_p_settings_ROM);

// void LoadInMemorySettingsSys();
void EmptyMemorySettingsSys(bool force = false);

// void LoadInMemorySettingsEthernet();
void EmptyMemorySettingsEthernet(bool force = false);

// void LoadInMemorySettingsNTP();
void EmptyMemorySettingsNTP(bool force = false);

// void LoadInMemorySettingsNTC();
// void EmptyMemorySettingsNTC(bool force = false);

void SaveAllSettings(void);
void NotSaveEmptyMemorySettings(void);
void LoadInMemorySettings(char *_name, void **p_MemorySettings, uint16_t _addr, uint16_t _size);

#define LoadInMemorySettingsSys() LoadInMemorySettings("Sys", (void **)&g_p_sys_settings_ROM, addr_sys_settings_ROM, len_sys_settings_ROM)
#define LoadInMemorySettingsEthernet() LoadInMemorySettings("связи", (void **)&g_p_ethernet_settings_ROM, addr_ethernet_settings_ROM, len_ethernet_settings_ROM)
#define LoadInMemorySettingsNTP() LoadInMemorySettings("NTP", (void **)&g_p_NTP_settings_ROM, addr_NTP_settings_ROM, len_NTP_settings_ROM)

// #define GetSettingsSys(data) GetSettings((void **)data, addr_sys_settings_ROM, len_sys_settings_ROM)
// #define EmptySettingsSys(data) toolEmptySettings((void **)data)

// #define GetSettingsEthernet(data) GetSettings((void **)data, addr_ethernet_settings_ROM, len_ethernet_settings_ROM)
// #define EmptySettingsEthernet(data) toolEmptySettings((void **)data)

// #define GetSettingsNTP(data) GetSettings((void **)data, len_NTP_settings_ROM)
// #define EmptySettingsNTP(data) toolEmptySettings((void **)data)

// #define GetSettingsNTC(data) GetSettings((void **)data, addr_NTC_settings_ROM, len_NTC_settings_ROM)
// #define EmptySettingsNTC(data) EmptySettings((void **)data)

uint16_t CRC16(void *pData, uint16_t nCount);
// void readROM(void *p_struct, uint16_t addr, uint16_t len);
// void get_p_all_settings_ROM(s_all_settings_ROM *_p_all_settings_ROM);
bool compareCRC(void *p_struct, uint16_t len);
void calcCRC(void *p_struct, uint16_t len);
// void writeROM(void *p_struct, uint16_t addr, uint16_t len);

union u_NeedSaveSettings
{
    uint32_t value = 0;
    struct s_bit
    {
        unsigned WIFI : 1;
        unsigned MQTT : 1;
        unsigned NTP : 1;
        unsigned OTA : 1;
        unsigned RSDebug : 1;
        unsigned SysMon : 1;
        // unsigned NTC : 1;
    } bit;
};

void cb_ut_emptymemory(void);

// extern bool NeedSaveSettings_NTP;
extern u_NeedSaveSettings NeedSaveSettings;

const uint16_t crc_tabl[] PROGMEM = {
    0x0000,
    0x1081,
    0x2102,
    0x3183,
    0x4204,
    0x5285,
    0x6306,
    0x7387,
    0x8408,
    0x9489,
    0xA50A,
    0xB58B,
    0xC60C,
    0xD68D,
    0xE70E,
    0xF78F,
};

#define BEGIN_CRC16 0xFFFF

#endif // my_EEPROM_h