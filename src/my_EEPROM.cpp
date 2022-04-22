#include <Arduino.h>
#include <EEPROM.h>

#include "my_EEPROM.h"
#include "my_debuglog.h"

// bool NeedSaveSettings_NTP = false;
union u_NeedSaveSettings NeedSaveSettings;
s_all_settings_ROM *s1;

// ********************************************************************************
// Расчет CRC16
// ********************************************************************************
uint16_t CRC16(void *pData, uint16_t nCount)
{
    uint8_t *s = (uint8_t *)pData;
    uint8_t tmp;
    uint16_t Ind;
    uint16_t CRC = BEGIN_CRC16;
    for (uint16_t i = 0; i < nCount; i++, s++)
    {
        tmp = *s;
        Ind = ((CRC ^ tmp) & 0xf);
        CRC = ((CRC >> 4) & 0xfff) ^ pgm_read_word_aligned(crc_tabl + Ind);
        tmp >>= 4;
        Ind = ((CRC ^ tmp) & 0xf);
        CRC = ((CRC >> 4) & 0xfff) ^ pgm_read_word_aligned(crc_tabl + Ind);
    }
    return (CRC);
}

// получаем указателя на данные из ROM (с EEPROM.begin)
// void get_p_all_settings_ROM(s_all_settings_ROM *_p_all_settings_ROM)
// {
//     EEPROM.begin(len_all_settings_ROM);
//     _p_all_settings_ROM = (s_all_settings_ROM *)EEPROM.getConstDataPtr();
//     if (!_p_all_settings_ROM) // если нет указателя
//     {
//         rsdebugEnflnF("Нет указателя на данные из ROM");
//         EEPROM.end();
//     }
// }

// void Print_s_NTP_settings(s_NTP_settings_ROM *_s)
// {
//     rsdebugDln("serversNTP[0]: %s", _s->serversNTP[0]);
//     rsdebugDln("serversNTP[1]: %s", _s->serversNTP[1]);
//     rsdebugDln("serversNTP[2]: %s", _s->serversNTP[2]);
//     rsdebugDln("T_syncNTP: %d", _s->T_syncNTP);
//     rsdebugDln("timezone: %d", _s->timezone); //timezone
//     rsdebugDln("Ttask: %d", _s->Ttask);
// };

// если данные в ROM не валидны - записываем "по-умолчанию"
void ROMVerifySettingsElseSaveDefault(bool force)
{
    // uint16_t len_struct = sizeof(s_all_settings_ROM);
    // rsdebugDlnF("@1");
    EEPROM.begin(len_all_settings_ROM);
    // rsdebugDlnF("@2");
    s_all_settings_ROM *_p_all_settings_ROM = (s_all_settings_ROM *)EEPROM.getConstDataPtr();
    // s1 = (s_all_settings_ROM *)EEPROM.getConstDataPtr();
    // s_all_settings_ROM *_p_all_settings_ROM = NULL;
    // get_p_all_settings_ROM(_p_all_settings_ROM);
    // EEPROM.getConstDataPtr();
    // rsdebugDlnF("@3");
    if (_p_all_settings_ROM)
    {
        // rsdebugDlnF("@4");
        rsdebugInfF("Проверяем CRC ROM");
        rsdebugInfln("[%d]", len_all_settings_ROM);
        // rsdebugInfln("ROM[%d]:%d", s1->len, len_all_settings_ROM);
        if (!force && compareCRC(_p_all_settings_ROM, len_all_settings_ROM)) // если данные в памяти валидны
        {
            rsdebugInflnF("Данные в ROM ok");
        }
        else
        {
            rsdebugWnflnF("Данные в ROM not ok");
            all_settings_Default(_p_all_settings_ROM);
            EEPROM.getDataPtr(); // установить флаг "данные изменились" для сохранения при EEPROM.end()
        }
    }
    else
        rsdebugEnflnF("Нет указателя на данные из ROM");
    EEPROM.end();
}

void all_settings_Default(s_all_settings_ROM *__p_all_settings_ROM)
{
    rsdebugInflnF("Устанавливаем настройки по умолчанию");
    s_all_settings_ROM def_all_settings_ROM;
    // def_all_settings_ROM.len = len_all_settings_ROM;
    calcCRC(&def_all_settings_ROM, len_all_settings_ROM);                      // данные по умолчанию не имеют вычесленого CRC !!! - вычисляем
    memcpy(__p_all_settings_ROM, &def_all_settings_ROM, len_all_settings_ROM); // заполняем данными по умолчанию
}

// читаем из ROM настройки и получаем указатель на данные в RAM
// void GetSettings(void *_p_settings_ROM, uint16_t addr, uint16_t len)
// {
//     if (!_p_settings_ROM) // если нет указателя на структуру
//     {
//         rsdebugDnflnF("@1");
//         _p_settings_ROM = malloc(len); // выделяем память и создаем указатель
//         rsdebugDnflnF("@2");
//         if (!_p_settings_ROM) // если нет указателя на структуру
//         {
//             rsdebugEnfF("Не смогли выделить память для данных из ROM: ");
//             rsdebugEnfln("%d байт", len);
//             return;
//         }
//         rsdebugDlnF("@3");

//         EEPROM.begin(len_all_settings_ROM);
//         rsdebugDlnF("@4");
//         uint8_t *_p_all_settings_ROM = (uint8_t *)EEPROM.getConstDataPtr();
//         rsdebugDlnF("@5");
//         // s_all_settings_ROM *_p_all_settings_ROM = NULL;
//         // get_p_all_settings_ROM(_p_all_settings_ROM); // EEPROM.getConstDataPtr();
//         if (!_p_all_settings_ROM)
//         {
//             rsdebugEnflnF("Нет указателя на данные из ROM");
//             return;
//         }
//         rsdebugDlnF("@6");
//         memcpy(_p_settings_ROM, _p_all_settings_ROM + addr, len);
//         rsdebugDlnF("@7");
//         EEPROM.end();
//         rsdebugDlnF("@8");
//     }
// }

// // очищаем RAM от настроек
// void EmptySettings(T _p_settings_ROM)
// {
//     if (_p_settings_ROM) // если есть указатель на структуру
//     {
//         // debuglog_print_free_memory();
//         delete _p_settings_ROM; // освобождаем память
//         _p_settings_ROM = NULL;
//         // debuglog_print_free_memory();
//     }
// }

void LoadInMemorySettings(char *_name, void **p_MemorySettings, uint16_t _addr, uint16_t _size)
{
    if (!*p_MemorySettings)
    {
        rsdebugInfF("Загружаем настройки ");
        rsdebugInfln("%s из ROM: %d байт", _name, _size);
        // GetSettings(p_MemorySettings, _addr, _size);
        if (!*p_MemorySettings) // если нет указателя на структуру
        {
            // rsdebugDnflnF("@1");
            *p_MemorySettings = malloc(_size); // выделяем память и создаем указатель
            // rsdebugDnflnF("@2");
            if (!*p_MemorySettings) // если нет указателя на структуру
            {
                rsdebugEnfF("Не смогли выделить память для данных из ROM: ");
                rsdebugEnfln("%d байт", _size);
                return;
            }
            // rsdebugDnflnF("@3");

            EEPROM.begin(len_all_settings_ROM);
            // rsdebugDnflnF("@4");
            uint8_t *_p_all_settings_ROM = (uint8_t *)EEPROM.getConstDataPtr();
            // rsdebugDnflnF("@5");
            // s_all_settings_ROM *_p_all_settings_ROM = NULL;
            // get_p_all_settings_ROM(_p_all_settings_ROM); // EEPROM.getConstDataPtr();
            if (!_p_all_settings_ROM)
            {
                EEPROM.end();
                rsdebugEnflnF("Нет указателя на данные из ROM");
                return;
            }
            // rsdebugDnflnF("@6");
            memcpy(*p_MemorySettings, _p_all_settings_ROM + _addr, _size);
            // rsdebugDnflnF("@7");
            EEPROM.end();
            // rsdebugDnflnF("@8");
        }
        // rsdebugDnflnF("@9");
    }
    // rsdebugDnflnF("@10");
    ut_emptymemory.suspendNextCall(emptymemory_TtaskDefault);
    // rsdebugDnflnF("@11");
}

// void LoadInMemorySettingsSys()
// {
//     if (!g_p_sys_settings_ROM)
//     {
//         rsdebugInfF("Загружаем настройки Sys из ROM: ");
//         rsdebugInfln("%d байт", len_sys_settings_ROM);
//         GetSettingsSys(&g_p_sys_settings_ROM);
//     }
//     ut_emptymemory.suspendNextCall(emptymemory_TtaskDefault);
// }

void EmptyMemorySettingsSys(bool force) // force=true для очистки памяти независимо от NeedSaveSettings.bit
{
    if (NeedSaveSettings.of.sys.of.OTA)
        rsdebugInflnF("Перед очищением Sys: OTA не сохранены");
    if (NeedSaveSettings.of.sys.of.RSDebug)
        rsdebugInflnF("Перед очищением Sys: RSDebug не сохранены");
    if (NeedSaveSettings.of.sys.of.SysMon)
        rsdebugInflnF("Перед очищением Sys: SysMon не сохранены");
    if (force || !NeedSaveSettings.of.sys.all)
    {
        if (g_p_sys_settings_ROM)
        {
            rsdebugInfF("Очищаем память от настроек Sys: ");
            rsdebugInfln("%d байт", len_sys_settings_ROM);
            // EmptySettingsSys(&g_p_sys_settings_ROM);
            // EmptySettings(g_p_sys_settings_ROM);
            delete g_p_sys_settings_ROM;
            g_p_sys_settings_ROM = NULL;
            NeedSaveSettings.of.sys.all = 0;
            // NeedSaveSettings.bit.OTA = false;
            // NeedSaveSettings.bit.RSDebug_T_task = false;
            // NeedSaveSettings.bit.SysMon = false;
        }
    }
}

// void LoadInMemorySettingsEthernet()
// {
//     if (!g_p_ethernet_settings_ROM)
//     {
//         rsdebugInfF("Загружаем настройки связи из ROM: ");
//         rsdebugInfln("%d байт", len_ethernet_settings_ROM);
//         GetSettingsEthernet(&g_p_ethernet_settings_ROM);
//     }
//     ut_emptymemory.suspendNextCall(emptymemory_TtaskDefault);
// }

void EmptyMemorySettingsEthernet(bool force) // force=true для очистки памяти независимо от NeedSaveSettings.bit
{
    if (NeedSaveSettings.of.bit.WIFI)
        rsdebugInflnF("Перед очищением настроек связи: WIFI не сохранены");
    if (NeedSaveSettings.of.bit.MQTT)
        rsdebugInflnF("Перед очищением настроек связи: MQTT не сохранены");
    if (force || (!NeedSaveSettings.of.bit.WIFI && !NeedSaveSettings.of.bit.MQTT))
    {
        if (g_p_ethernet_settings_ROM)
        {
            rsdebugInfF("Очищаем память от настроек связи: ");
            rsdebugInfln("%d байт", len_ethernet_settings_ROM);
            // EmptySettingsEthernet(&g_p_ethernet_settings_ROM);
            // EmptySettings(g_p_ethernet_settings_ROM);
            delete g_p_ethernet_settings_ROM;
            g_p_ethernet_settings_ROM = NULL;

            NeedSaveSettings.of.bit.WIFI = false;
            NeedSaveSettings.of.bit.MQTT = false;
        }
    }
}

// void LoadInMemorySettingsNTP()
// {
//     if (!g_p_NTP_settings_ROM)
//     {
//         rsdebugInfF("Загружаем настройки NTP из ROM: ");
//         rsdebugInfln("%d байт", len_NTP_settings_ROM);
//         GetSettingsNTP(&g_p_NTP_settings_ROM);
//     }
//     ut_emptymemory.suspendNextCall(emptymemory_TtaskDefault);
// }

void EmptyMemorySettingsNTP(bool force) // force=true для очистки памяти независимо от NeedSaveSettings.bit
{
    if (g_p_NTP_settings_ROM)
    {
        if (NeedSaveSettings.of.bit.NTP)
            rsdebugInflnF("Перед очищением NTP: NTP не сохранены");
        if (force || !NeedSaveSettings.of.bit.NTP)
        {
            rsdebugInfF("Очищаем память от настроек NTP: ");
            rsdebugInfln("%d байт", len_NTP_settings_ROM);
            // EmptySettingsNTP(&g_p_NTP_settings_ROM);
            // EmptySettings(g_p_NTP_settings_ROM);
            delete g_p_NTP_settings_ROM;
            g_p_NTP_settings_ROM = NULL;
            NeedSaveSettings.of.bit.NTP = false;
        }
    }
}

void verifyErrorNeedSaveSettings(const char *nameBit, bool _bit, void *settings_ROM)
{
    if (_bit && !settings_ROM)
    {
        rsdebugEnfln("NeedSaveSettings.bit.%s=true, но нет указателя на данные", nameBit)
    }
    // else if (!_bit && settings_ROM) // не нужно
    // {
    //     rsdebugEnfln("NeedSaveSettings.bit.%s=false, но есть указатель на данные", nameBit);
    // }
}

bool SaveAllSettings(void)
{
    if (NeedSaveSettings.all)
    {
        EEPROM.begin(len_all_settings_ROM);
        // uint8_t *_p_all_settings_ROM = (uint8_t *)EEPROM.getConstDataPtr();
        s_all_settings_ROM *_p_all_settings_ROM = (s_all_settings_ROM *)EEPROM.getConstDataPtr();
        // s_all_settings_ROM *_p_all_settings_ROM = NULL;
        // get_p_all_settings_ROM(_p_all_settings_ROM); // EEPROM.getConstDataPtr();
        if (_p_all_settings_ROM) // если нет указателя
        {
            rsdebugInflnF("Сохраняем все данные:");
            rsdebugDnfln("bit.RSDebug: %d", NeedSaveSettings.of.sys.of.RSDebug);
            rsdebugDnfln("bit.OTA: %d", NeedSaveSettings.of.sys.of.OTA);
            rsdebugDnfln("bit.SysMon: %d", NeedSaveSettings.of.sys.of.SysMon);
            verifyErrorNeedSaveSettings("RSDebug", NeedSaveSettings.of.sys.of.RSDebug, g_p_sys_settings_ROM);
            verifyErrorNeedSaveSettings("OTA", NeedSaveSettings.of.sys.of.OTA, g_p_sys_settings_ROM);
            verifyErrorNeedSaveSettings("SysMon", NeedSaveSettings.of.sys.of.SysMon, g_p_sys_settings_ROM);
            if (NeedSaveSettings.of.sys.all)
            {
                LoadInMemorySettingsSys();
                rsdebugDnflnF("Сохраняем данные SYS");
                memcpy(&_p_all_settings_ROM->sys_settings_ROM, g_p_sys_settings_ROM, len_sys_settings_ROM);
                EmptyMemorySettingsSys(true);
            }
            rsdebugDnfln("bit.WIFI: %s", NeedSaveSettings.of.bit.WIFI ? "1" : "0");
            rsdebugDnfln("bit.MQTT: %s", NeedSaveSettings.of.bit.MQTT ? "1" : "0");
            verifyErrorNeedSaveSettings("WiFi", NeedSaveSettings.of.bit.WIFI, g_p_ethernet_settings_ROM);
            verifyErrorNeedSaveSettings("MQTT", NeedSaveSettings.of.bit.MQTT, g_p_ethernet_settings_ROM);
            if (NeedSaveSettings.of.bit.WIFI || NeedSaveSettings.of.bit.MQTT)
            {
                LoadInMemorySettingsEthernet();
                rsdebugDnflnF("Сохраняем данные ethernet");
                memcpy(&_p_all_settings_ROM->ethernet_settings_ROM, g_p_ethernet_settings_ROM, len_ethernet_settings_ROM);
                EmptyMemorySettingsEthernet(true);
            }
            rsdebugDnfln("bit.NTP: %s", NeedSaveSettings.of.bit.NTP ? "1" : "0");
            verifyErrorNeedSaveSettings("NTP", NeedSaveSettings.of.bit.NTP, g_p_NTP_settings_ROM);
            if (NeedSaveSettings.of.bit.NTP)
            {
                LoadInMemorySettingsNTP();
                rsdebugDnflnF("Сохраняем данные NTP");
                memcpy(&_p_all_settings_ROM->NTP_settings_ROM, g_p_NTP_settings_ROM, len_NTP_settings_ROM);
                EmptyMemorySettingsNTP(true);
            }
            calcCRC(_p_all_settings_ROM, len_all_settings_ROM);
            EEPROM.getDataPtr(); // установить флаг "данные изменились" для сохранения при EEPROM.end()
        }
        else
            rsdebugEnflnF("Нет указателя на данные из ROM");
        EEPROM.end();
        return true;
    }
    else
    {
        rsdebugWnflnF("Нет изменений - не сохраняем");
        return false;
    }
}

void NotSaveEmptyMemorySettings(void)
{
    EmptyMemorySettingsSys(true);
    EmptyMemorySettingsEthernet(true);
    EmptyMemorySettingsNTP(true);
    // EmptyMemorySettingsNTC(true);
}

bool compareCRC(void *p_struct, uint16_t len) // Вычисляем crc: true - crc ok
{
    return (*(uint16_t *)p_struct == CRC16(((uint8_t *)p_struct) + len_crc, len - len_crc));
}

void calcCRC(void *p_struct, uint16_t len) // Вычисляем crc
{
    *(uint16_t *)p_struct = CRC16(((uint8_t *)p_struct) + len_crc, len - len_crc);
}

void cb_ut_emptymemory(void)
{
    // if (NeedSaveSettings.all)
    // {
    //     rsdebugDnfln("NeedSaveSettings: %d", NeedSaveSettings.all);
    // }
    // if (g_p_sys_settings_ROM)
    // {
    //     rsdebugDnflnF("g_p_sys_settings_ROM");
    // }
    // if (g_p_ethernet_settings_ROM)
    // {
    //     rsdebugDnflnF("g_p_ethernet_settings_ROM");
    // }
    // if (g_p_NTP_settings_ROM)
    // {
    //     rsdebugDnflnF("g_p_ethernet_settings_ROM");
    // }
    // rsdebugDnfln("bit.NTP: %s", NeedSaveSettings.bit.NTP ? "1" : "0");
    // rsdebugDnfln("bit.OTA: %s", NeedSaveSettings.bit.OTA ? "1" : "0");
    // rsdebugDnfln("bit.RSDebug: %s", isNSS_allRSDebug ? "1" : "0");
    // rsdebugDnfln("bit.SysMon: %s", NeedSaveSettings.bit.SysMon ? "1" : "0");
    // rsdebugDnfln("bit.WIFI: %s", NeedSaveSettings.bit.WIFI ? "1" : "0");
    // rsdebugDnfln("bit.MQTT: %s", NeedSaveSettings.bit.MQTT ? "1" : "0");
    if (wifi_state == _wifi_connected)
        EmptyMemorySettingsEthernet();
    EmptyMemorySettingsSys();
    EmptyMemorySettingsNTP();
    // EmptyMemorySettingsNTC();
}
