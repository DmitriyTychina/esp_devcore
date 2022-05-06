#pragma once

#include <Arduino.h>
#include <EEPROM.h>
#include <typeinfo>

#include "main.h"
#include "my_wifi.h"
#include "my_NTP.h"
#include "my_sysmon.h"
#include "my_debuglog.h"
#include "my_ota.h"

// #define EEPROM_C
#define EEPROM_CPP

#if defined(EEPROM_C)
#define emptymemory_TtaskDefault 2000
#elif defined(EEPROM_CPP)
#define emptymemory_TtaskDefault 2000
extern uint32_t g_last_request_EEPROM;
#endif

#define Data_lifetime 20000

enum e_settings_from
{
    from_def,
    from_rom,
    from_ram
};

struct s_settings_serv
{
    char SSID[16];          // Имя точки WiFi
    char PASS[24];          // Пароль точки WiFi
    char MQTTip[lenMQTTip]; // IP сервера MQTT в этой сети
};

struct s_ethernet_settings_ROM
{
    // uint16_t crc;
    uint32_t WiFi_Ttask = WiFi_TtaskDefault;
    s_settings_serv settings_serv[5] = {{WIFI_SSID1, WIFI_PASS1, IPmqttWIFI1},
                                        {WIFI_SSID3, WIFI_PASS3, IPmqttWIFI3},
                                        {WIFI_SSID2, WIFI_PASS2, IPmqttWIFI2},
                                        {WIFI_SSID4, WIFI_PASS4, IPmqttWIFI4},
                                        {WIFI_SSID5, WIFI_PASS5, IPmqttWIFI5}}; // Параметры точкек WiFi и серверов MQTT в этих сетях
    uint32_t MQTT_Ttask = MQTT_TtaskDefault;
    char MQTT_user[lenMQTTuser] = mqtt_user; // Логин сервера MQTT
    char MQTT_pass[lenMQTTpass] = mqtt_pass; // Пароль сервера MQTT
    // bool stawebserver = false;                                                                                          // Разрешить web-сервер в режиме клиента
    //    char apName[16] = my_AP_NAME;                                                             // Имя точки доступа WiFi
    //    char apPass[24] = my_AP_PASS;                                                             // Пароль точки доступа WiFi
    //    uint8_t apIP[4] = IPAP;                                                                   // IP web-cервера точки доступа WiFi
    // uint16_t Tms = 500; // Период, мс --- для шедулера -не нужен -меняем динамически
};

struct s_sys_settings_ROM
{
    // uint16_t crc;
    uint32_t RSDebug_Ttask = RSDebug_TtaskDefault;
    uint32_t OTA_Ttask = OTA_TtaskDefault;
    uint32_t SysMon_Ttask = SysMon_TtaskDefault;
    uint32_t FreeUpRAM_Ttask = emptymemory_TtaskDefault;
    // uint32_t StatusLED = StatusLED_TtaskDefault;
    bool RSDebug_SDebug = true;
    bool RSDebug_RDebug = true;
    bool SysMon_info_to_rsdebug = false;
    bool SysMon_info_to_mqtt = true;
    // e_microprogramm programm_autonomy; // программа автономного режима, работет не зависимо от связи с сервером
    // e_microprogramm programm_remote;   // программа дистанционного режима, работет при отсутствии связи с сервером
    // e_microprogramm programm_arm;      // программа ручного режима, работет при отсутствии связи с сервером
    // char namesystem[16] = "Water"; // Имя системы // Один раз по MQTT
    // uint8_t ver[6] = "00.20";      // Версия ПО // Один раз по MQTT
};

struct s_NTP_settings_ROM
{
    char serversNTP[3][24] = {"132.163.96.1", "ntp2.stratum2.ru", "pool.ntp.org"}; // NTP-сервера времени
    int8_t timezone = 3;                                                           // timezone // Часовой пояс (+3 - Москва)
    uint16_t PeriodSyncNTP = 60;                                                   // Период синхронизации времени с NTP-сервером в минутах
    uint32_t Ttask = NTP_TtaskDefault;
    bool astro = true;
};

struct s_all_settings_ROM
{
    uint16_t crc;
    // uint16_t len;
    s_ethernet_settings_ROM ethernet_settings_ROM;
    s_sys_settings_ROM sys_settings_ROM;
    s_NTP_settings_ROM NTP_settings_ROM;
    // uint16_t crc3=1;
    // uint16_t crc2=0;
};

// соблюдаем порядок вычисления адреса - не нужно
// #define addr_all_settings_ROM 0
// #define len_all_settings_ROM sizeof(*g_p_EEPROM) // всегда первая CRC
#define len_all_settings_ROM sizeof(s_all_settings_ROM) // всегда первая CRC
// #define len_crc sizeof(g_p_EEPROM->crc)
#define len_crc sizeof(s_all_settings_ROM::crc)

// #define addr_ethernet_settings_ROM (addr_all_settings_ROM+len_crc) // первая после CRC
// #define addr_ethernet_settings_ROM ((uint8_t *)&g_p_EEPROM->ethernet_settings_ROM - (uint8_t *)g_p_EEPROM) // первая после CRC
// #define addr_ethernet_settings_ROM (uint32_t)(&((struct s_all_settings_ROM *)0)->ethernet_settings_ROM)
#define addr_ethernet_settings_ROM offsetof(s_all_settings_ROM, ethernet_settings_ROM)
// #define len_ethernet_settings_ROM sizeof(g_p_EEPROM->ethernet_settings_ROM)
#define len_ethernet_settings_ROM sizeof(s_all_settings_ROM::ethernet_settings_ROM)

// #define addr_NTP_settings_ROM (addr_ethernet_settings_ROM + len_ethernet_settings_ROM) // такая запись давала адрес на 2 байта меньше (???)
// #define addr_NTP_settings_ROM (((uint8_t *)&g_p_EEPROM->NTP_settings_ROM) - (uint8_t *)g_p_EEPROM) // только такая запись давала реальный адрес
// #define addr_NTP_settings_ROM (uint32_t)(&((struct s_all_settings_ROM*)0)->NTP_settings_ROM)
#define addr_NTP_settings_ROM offsetof(s_all_settings_ROM, NTP_settings_ROM)
// #define len_NTP_settings_ROM sizeof(g_p_EEPROM->NTP_settings_ROM)
#define len_NTP_settings_ROM sizeof(s_all_settings_ROM::NTP_settings_ROM)

// // #define addr_NTC_settings_ROM (addr_NTP_settings_ROM + len_NTP_settings_ROM)
// #define addr_NTC_settings_ROM ((char *)&g_p_EEPROM->NTC_settings_ROM - (char *)g_p_EEPROM)
// #define len_NTC_settings_ROM sizeof(g_p_EEPROM->NTC_settings_ROM)

// // #define addr_sys_settings_ROM (addr_NTC_settings_ROM + len_NTC_settings_ROM)
// #define addr_sys_settings_ROM const_cast<uint32_t>((uint8_t *)&g_p_EEPROM->sys_settings_ROM - (uint8_t *)g_p_EEPROM)
// #define addr_sys_settings_ROM (uint32_t)(&((struct s_all_settings_ROM *)0)->sys_settings_ROM)
#define addr_sys_settings_ROM offsetof(s_all_settings_ROM, sys_settings_ROM)
// #define len_sys_settings_ROM sizeof(g_p_EEPROM->sys_settings_ROM)
#define len_sys_settings_ROM sizeof(s_all_settings_ROM::sys_settings_ROM)

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

void calcCRC(void *p_struct, uint16_t len);
uint16_t CRC16(void *pData, uint16_t nCount);
bool compareCRC(void *p_struct, uint16_t len);
void ROMVerifySettingsElseSaveDefault(bool force = false);
bool SaveAllSettings(void);
void cb_ut_emptymemory(void);

#if defined(EEPROM_C)
extern s_sys_settings_ROM *g_p_sys_settings_ROM;
extern s_NTP_settings_ROM *g_p_NTP_settings_ROM;
extern s_ethernet_settings_ROM *g_p_ethernet_settings_ROM;

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

void NotSaveEmptyMemorySettings(void);
void LoadInMemorySettings(char *_name, void **p_MemorySettings, uint16_t _addr, uint16_t _size);

#define LoadInMemorySettingsSys() LoadInMemorySettings("Sys", (void **)&g_p_sys_settings_ROM, addr_sys_settings_ROM, len_sys_settings_ROM)
#define LoadInMemorySettingsEthernet() LoadInMemorySettings("связи", (void **)&g_p_ethernet_settings_ROM, addr_ethernet_settings_ROM, len_ethernet_settings_ROM)
#define LoadInMemorySettingsNTP() LoadInMemorySettings("NTP", (void **)&g_p_NTP_settings_ROM, addr_NTP_settings_ROM, len_NTP_settings_ROM)

union u_NeedSaveSettings
{
    uint32_t all = 0;
    union u_sys
    {
        uint8_t all;
        struct s_bit
        {
            unsigned RSDebug_T_task : 1;
            unsigned RSDebug_RD_en : 1;
            unsigned RSDebug_SD_en : 1;
            unsigned OTA_T_task : 1;
            unsigned SysMon_T_task : 1;
        } bit;
        struct s_of
        {
            unsigned RSDebug : 3;
            unsigned OTA : 1;
            unsigned SysMon : 1;
        } of;
    } sys;
    struct s_bit
    {
        unsigned WIFI : 1;
        unsigned MQTT : 1;
        unsigned NTP : 1;
        // unsigned NTC : 1;
    } bit;
};

// extern bool NeedSaveSettings_NTP;
extern u_NeedSaveSettings NeedSaveSettings;

#elif defined(EEPROM_CPP)

#define LoadInMemorySettingsSys()
#define LoadInMemorySettingsEthernet()
#define LoadInMemorySettingsNTP()

template <typename T>
class c_settings_base // шаблон
{
protected:
    T *p_s = NULL;
    uint32_t last_request;
    // virtual void new_get();
    // virtual void post_get();
    virtual bool condition_del() { return p_s != NULL; };
    // virtual void del();
    virtual void new_get()
    {
        // rsdebugDnfln("new_unitDEF"); // :%s", typeid(T).name());
        this->p_s = new T;
    };
    virtual void post_get()
    {
        // rsdebugDnfln("get_unitDEF"); // :%s", typeid(T).name());
        this->last_request = millis();
    };
    virtual bool del()
    {
        // rsdebugDnfln("del_unitDEF"); // :%s", typeid(T).name());
        delete this->p_s;
        this->p_s = NULL;
        return true;
    };

public:
    // c_settings_base()
    // {
    //     // rsdebugDnfln("base"); // :%s", typeid(T).name());
    //     /* this-> */p_s = NULL;
    // };
    // ~c_settings_base()
    // {
    //     rsdebugDnfln("~base"); // :%s", typeid(T).name());
    //     del_p_s(true);
    // };
    T *operator()() // getter
    {
        // rsdebugDnfln("get"); // :%s", typeid(*this).name());
        if (/* this-> */ p_s == NULL)
        {
            // rsdebugDnfln("new"); // :%s", typeid(*this).name());
            // p_s = new T;
            new_get();
        }
        post_get();
        // rsdebugDnfln("@3");
        return /* this-> */ p_s;
    };
    bool del_p_s(bool force = false)
    {
        // rsdebugDnfln("del_p_s-%d-%d---", this->p_s, millis() - /* this-> */ last_request); // :%s", typeid(*this).name());
        // rsdebugDnfln("del_p_s"); // :%s", typeid(*this).name());
        if (condition_del())
        {
            if (force || ((millis() - /* this-> */ last_request) > Data_lifetime))
            {
                // rsdebugDnfln("del"); // :%s", typeid(*this).name());
                return del();
            }
        }
        return false; // (this->p_s == NULL);
    };
};

template <typename T>
class c_unit_settings_def : public c_settings_base<T>
{
    // protected:
    //     void new_get() override
    //     {
    //         rsdebugDnfln("new_unitDEF"); // :%s", typeid(T).name());
    //         this->p_s = new T;
    //     };
    //     void post_get() override
    //     {
    //         // rsdebugDnfln("get_unitDEF"); // :%s", typeid(T).name());
    //         this->last_request = millis();
    //     };
    //     void del() override
    //     {
    //         rsdebugDnfln("del_unitDEF"); // :%s", typeid(T).name());
    //         delete this->p_s;
    //         this->p_s = NULL;
    //     };
};

template <typename T, long long ADDR>
class c_unit_settings_rom : public c_settings_base<T>
{
protected:
    void new_get() override
    {
        // rsdebugDnfln("new_get_unitROM"); // :%s", typeid(*this).name());
        // rsdebugDnfln("new_unitROM-%d-%d-%d", &EEPROM.getConstDataPtr()[0], EEPROM.getConstDataPtr(), millis() - g_last_request_EEPROM); // :%s", typeid(*this).name());
        if (&EEPROM.getConstDataPtr()[0] == NULL)
        {
            // rsdebugDnfln("new_unitROM"); // :%s", typeid(*this).name());
            // EEPROM.begin(len_all_settings_ROM);
            EEPROM.begin(sizeof(s_all_settings_ROM));
            // g_p_EEPROM = &EEPROM.getConstDataPtr()[0];
        }
        this->p_s = (T *)(&EEPROM.getConstDataPtr()[ADDR]);
        // rsdebugDnfln("new_unitROM[%d]-[%d]", EEPROM.getConstDataPtr(), this->p_s); // :%s", typeid(*this).name());
    };
    void post_get() override
    {
        this->last_request = millis();
        // rsdebugDnfln("@1");
        g_last_request_EEPROM = this->last_request;
        // rsdebugDnfln("@2");
        // rsdebugDnfln("get_unitROM-%d-%d", this->last_request, g_last_request_EEPROM); // :%s", typeid(*this).name());
    };
    bool condition_del() override
    {
        return ((this->p_s != NULL) || (EEPROM.getConstDataPtr() != NULL));
    };
    bool del() override
    {
        // rsdebugDnfln("del_unitROM-%d-%d-%d", &EEPROM.getConstDataPtr()[0], EEPROM.getConstDataPtr(), millis() - g_last_request_EEPROM); // :%s", typeid(*this).name());
        if (EEPROM.getConstDataPtr() != NULL)
        {
            if (millis() - g_last_request_EEPROM > Data_lifetime)
            {
                // rsdebugDnfln("del_unitROM"); // :%s", typeid(*this).name());
                // rsdebugDnfln("del g_p_EEPROM"); // :%s", typeid(*this).name());
                EEPROM.end();
                // g_p_EEPROM = NULL;
                this->p_s = NULL;
                return true;
            }
        }
        return false;
    };
};

template <typename T, uint32_t ADDR /* , uint32_t LEN */>
class c_unit_settings_ram : public c_settings_base<T>
{
protected:
    void new_get() override
    {
        if (this->p_s == NULL)
        {
            // rsdebugDnfln("new_unitRAM");
            c_unit_settings_rom<T, ADDR> c_unit_settings_tmp;
            this->p_s = new T;
            memcpy(this->p_s, c_unit_settings_tmp(), sizeof(T));
            // rsdebugDnfln("new_get_unitROM[%d]-[%d] %p _ %p", ADDR, EEPROM.getConstDataPtr(), g_p_EEPROM, this->p_s); // :%s", typeid(*this).name());
        }
    };
    // void post_get() override
    // {
    //     // rsdebugDnfln("get_unitRAM");
    //     this->last_request = millis();
    // };
    bool del() override
    {
        // rsdebugDnfln("del_unitRAM");
        delete this->p_s;
        this->p_s = NULL;
        return true;
    };
};

void print_free_up_net(bool _flag, String _pre_str);
void print_free_up_sys(bool _flag, String _pre_str);
void print_free_up_NTP(bool _flag, String _pre_str);

class c_all_settings_def
{
public:
    c_unit_settings_def<s_ethernet_settings_ROM> p_NET_settings;
    c_unit_settings_def<s_sys_settings_ROM> p_SYS_settings;
    c_unit_settings_def<s_NTP_settings_ROM> p_NTP_settings;
    bool is_all_del(bool force = false)
    {
        // bool ret = false;
        bool b_NET = p_NET_settings.del_p_s(force);
        bool b_SYS = p_SYS_settings.del_p_s(force);
        bool b_NTP = p_NTP_settings.del_p_s(force);
        // const char* _str = "DEF";
        String _str = F("DEF");
        print_free_up_net(b_NET, _str);
        print_free_up_sys(b_SYS, _str);
        print_free_up_NTP(b_NTP, _str);
        return b_NET & b_SYS & b_NTP;
    };
};

class c_all_settings_rom
{
public:
    c_unit_settings_rom<s_ethernet_settings_ROM, addr_ethernet_settings_ROM> p_NET_settings;
    c_unit_settings_rom<s_sys_settings_ROM, addr_sys_settings_ROM> p_SYS_settings;
    c_unit_settings_rom<s_NTP_settings_ROM, addr_NTP_settings_ROM> p_NTP_settings;

    bool is_all_del(bool force = false)
    {
        // bool ret = false;
        bool b_NET = p_NET_settings.del_p_s(force);
        bool b_SYS = p_SYS_settings.del_p_s(force);
        bool b_NTP = p_NTP_settings.del_p_s(force);
        // const char* _str = "ROM";
        bool ret = b_NET | b_SYS | b_NTP;
        if (ret)
        {
            rsdebugInfF("ROM: Очищаем память");
            rsdebugInfln("$%d", len_all_settings_ROM);
        }
        return ret;
    };
};

class c_all_settings_ram
{
public:
    bool unsaved = false;

    c_unit_settings_ram<s_ethernet_settings_ROM, addr_ethernet_settings_ROM> p_NET_settings;
    c_unit_settings_ram<s_sys_settings_ROM, addr_sys_settings_ROM> p_SYS_settings;
    c_unit_settings_ram<s_NTP_settings_ROM, addr_NTP_settings_ROM> p_NTP_settings;

    bool is_all_del(bool force = false)
    {
        // bool ret = false;
        bool b_NET = p_NET_settings.del_p_s(force);
        bool b_SYS = p_SYS_settings.del_p_s(force);
        bool b_NTP = p_NTP_settings.del_p_s(force);
        // const char* _str = "RAM";
        String _str = F("RAM");
        print_free_up_net(b_NET, _str);
        print_free_up_sys(b_SYS, _str);
        print_free_up_NTP(b_NTP, _str);
        return b_NET & b_SYS & b_NTP;
    };
};

extern c_all_settings_def def_data;
extern c_all_settings_rom rom_data;
extern c_all_settings_ram ram_data;
// #else
#endif

#if defined(EEPROM_C)
#elif defined(EEPROM_CPP)
#endif
