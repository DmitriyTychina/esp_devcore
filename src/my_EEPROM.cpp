#include <Arduino.h>
#include <EEPROM.h>

#include "my_EEPROM.h"
#include "my_debuglog.h"

// bool NeedSaveSettings_NTP = false;
union u_NeedSaveSettings NeedSaveSettings;
s_all_settings_ROM *s1 = NULL;

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
void get_p_all_settings_ROM(s_all_settings_ROM *_p_all_settings_ROM)
{
    EEPROM.begin(len_all_settings_ROM);
    _p_all_settings_ROM = (s_all_settings_ROM *)EEPROM.getConstDataPtr();
    if (!_p_all_settings_ROM) // если нет указателя
    {
        rsdebugEnflnF("Нет указателя на данные из ROM");
        EEPROM.end();
    }
}

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
    // s_all_settings_ROM *_p_all_settings_ROM = NULL;
    // get_p_all_settings_ROM(_p_all_settings_ROM);
    // EEPROM.getConstDataPtr();
    // rsdebugDlnF("@3");
    if (!_p_all_settings_ROM)
    {
        // rsdebugDlnF("@33");
        return;
    }
    // rsdebugDlnF("@4");
    if (!force)
    {
        rsdebugInflnF("***Проверяем CRC ROM***");
        if (compareCRC(_p_all_settings_ROM, len_all_settings_ROM)) // если данные в памяти валидны
        {
            rsdebugInflnF("Данные в ROM валидны");
            return;
        }
        else
        {
            rsdebugWnflnF("Данные в ROM не валидны");
            all_settings_Default((s_all_settings_ROM *)_p_all_settings_ROM);
            EEPROM.getDataPtr(); // установить флаг "данные изменились" для сохранения при EEPROM.end()
        }
    }
    else
    {
        // rsdebugDlnF("@5");
        all_settings_Default((s_all_settings_ROM *)_p_all_settings_ROM);
        // rsdebugDlnF("@6");
        EEPROM.getDataPtr(); // установить флаг "данные изменились" для сохранения при EEPROM.end()
                             // rsdebugDlnF("@7");
    }
    // rsdebugDlnF("@8");
    EEPROM.end();
    // rsdebugDlnF("@9");
}

// Инициализируем ROM данными по умолчанию
void all_settings_Default(s_all_settings_ROM *__p_all_settings_ROM)
{
    rsdebugInflnF("Инициализируем ROM по умолчанию");
    s_all_settings_ROM def_all_settings_ROM;
    calcCRC(&def_all_settings_ROM, len_all_settings_ROM);                      // данные по умолчанию не имеют вычесленого CRC !!! - вычисляем
    memcpy(__p_all_settings_ROM, &def_all_settings_ROM, len_all_settings_ROM); // заполняем данными по умолчанию
}

// читаем из ROM настройки и получаем указатель на данные в RAM
void GetSettings(void **_p_settings_ROM, uint16_t addr, uint16_t len)
{
    if (!*_p_settings_ROM) // если нет указателя на структуру
    {
        // rsdebugDlnF("@1");
        *_p_settings_ROM = (uint8_t *)malloc(len); // выделяем память и создаем указатель
                                                   //  rsdebugDlnF("@2");
        if (!*_p_settings_ROM)                     // если нет указателя на структуру
        {
            rsdebugEnfF("Не смогли выделить память для данных из ROM: ");
            rsdebugEnfln("%d байт", len);
            return;
        }
        // rsdebugDlnF("@3");

        EEPROM.begin(len_all_settings_ROM);
        // rsdebugDlnF("@4");
        uint8_t *_p_all_settings_ROM = (uint8_t *)EEPROM.getConstDataPtr();
        // rsdebugDlnF("@5");
        // s_all_settings_ROM *_p_all_settings_ROM = NULL;
        // get_p_all_settings_ROM(_p_all_settings_ROM); // EEPROM.getConstDataPtr();
        if (!_p_all_settings_ROM)
        {
            rsdebugEnflnF("Нет указателя на данные из ROM");
            return;
        }
        //  rsdebugDlnF("@6");
        memcpy(*_p_settings_ROM, _p_all_settings_ROM + addr, len);
        //  rsdebugDlnF("@7");
        EEPROM.end();
        // rsdebugDlnF("@8");
    }
}

// очищаем RAM от настроек
void EmptySettings(void **_p_settings_ROM)
{
    if (*_p_settings_ROM) // если есть указатель на структуру
    {
        // debuglog_print_free_memory();
        delete (uint8_t *)*_p_settings_ROM; // освобождаем память
        *_p_settings_ROM = NULL;
        // debuglog_print_free_memory();
    }
}

void LoadInMemorySettingsSys()
{
    if (!g_p_sys_settings_ROM)
    {
        rsdebugInfF("Загружаем настройки Sys из ROM: ");
        rsdebugInfln("%d байт", len_sys_settings_ROM);
        GetSettingsSys(&g_p_sys_settings_ROM);
    }
}
void EmptyMemorySettingsSys(bool force) // force=true для очистки памяти независимо от NeedSaveSettings.bit
{
    if (NeedSaveSettings.bit.OTA)
        rsdebugInflnF("Перед очищением: настройки OTA не сохранены");
    if (NeedSaveSettings.bit.RSDebug)
        rsdebugInflnF("Перед очищением: настройки RSDebug не сохранены");
    if (NeedSaveSettings.bit.SysMon)
        rsdebugInflnF("Перед очищением: настройки SysMon не сохранены");
    if (force || (!NeedSaveSettings.bit.OTA && !NeedSaveSettings.bit.RSDebug && !NeedSaveSettings.bit.SysMon))
    {
        if (g_p_sys_settings_ROM)
        {
            rsdebugInfF("Очищаем память от настроек Sys: ");
            rsdebugInfln("%d байт", len_sys_settings_ROM);
            // EmptySettingsSys(&g_p_sys_settings_ROM);
            EmptySettings((void **)&g_p_sys_settings_ROM);
            NeedSaveSettings.bit.OTA = false;
            NeedSaveSettings.bit.RSDebug = false;
            NeedSaveSettings.bit.SysMon = false;
        }
    }
}

void LoadInMemorySettingsEthernet()
{
    if (!g_p_ethernet_settings_ROM)
    {
        rsdebugInfF("Загружаем настройки связи из ROM: ");
        rsdebugInfln("%d байт", len_ethernet_settings_ROM);
        GetSettingsEthernet(&g_p_ethernet_settings_ROM);
    }
}

void EmptyMemorySettingsEthernet(bool force) // force=true для очистки памяти независимо от NeedSaveSettings.bit
{
    if (NeedSaveSettings.bit.WIFI)
        rsdebugInflnF("Перед очищением: настройки WIFI не сохранены");
    if (NeedSaveSettings.bit.MQTT)
        rsdebugInflnF("Перед очищением: настройки MQTT не сохранены");
    if (force || (!NeedSaveSettings.bit.WIFI && !NeedSaveSettings.bit.MQTT))
    {
        if (g_p_ethernet_settings_ROM)
        {
            rsdebugInfF("Очищаем память от настроек связи: ");
            rsdebugInfln("%d байт", len_ethernet_settings_ROM);
            // EmptySettingsEthernet(&g_p_ethernet_settings_ROM);
            EmptySettings((void **)&g_p_ethernet_settings_ROM);
            NeedSaveSettings.bit.WIFI = false;
            NeedSaveSettings.bit.MQTT = false;
        }
    }
}

void LoadInMemorySettingsNTP()
{
    if (!g_p_NTP_settings_ROM)
    {
        rsdebugInfF("Загружаем настройки NTP из ROM: ");
        rsdebugInfln("%d байт", len_NTP_settings_ROM);
        GetSettingsNTP(&g_p_NTP_settings_ROM);
    }
}

void EmptyMemorySettingsNTP(bool force) // force=true для очистки памяти независимо от NeedSaveSettings.bit
{
    if (g_p_NTP_settings_ROM)
    {
        if (NeedSaveSettings.bit.NTP)
            rsdebugInflnF("Перед очищением: настройки NTP не сохранены");
        if (force || !NeedSaveSettings.bit.NTP)
        {
            rsdebugInfF("Очищаем память от настроек NTP: ");
            rsdebugInfln("%d байт", len_NTP_settings_ROM);
            // EmptySettingsNTP(&g_p_NTP_settings_ROM);
            EmptySettings((void **)&g_p_NTP_settings_ROM);
            NeedSaveSettings.bit.NTP = false;
        }
    }
}

void verifyErrorNeedSaveSettings(const char *nameBit, bool _bit, void *settings_ROM)
{
    if (_bit && !settings_ROM)
        rsdebugEnfln("NeedSaveSettings.bit.%s=true, но нет указателя на данные", nameBit) else if (!_bit && settings_ROM)
            rsdebugEnfln("NeedSaveSettings.bit.%s=false, но есть указатель на данные", nameBit);
}

void SaveAllSettings(void)
{
    EEPROM.begin(len_all_settings_ROM);
    // uint8_t *_p_all_settings_ROM = (uint8_t *)EEPROM.getConstDataPtr();
    s_all_settings_ROM *_p_all_settings_ROM = (s_all_settings_ROM *)EEPROM.getConstDataPtr();
    // s_all_settings_ROM *_p_all_settings_ROM = NULL;
    // get_p_all_settings_ROM(_p_all_settings_ROM); // EEPROM.getConstDataPtr();
    if (!_p_all_settings_ROM) // если нет указателя
    {
        rsdebugEnflnF("Нет указателя на данные из ROM");
        return;
    }
    rsdebugInflnF("Сохраняем все данные:");

    rsdebugDnfln("bit.NTP: %s", NeedSaveSettings.bit.NTP ? "1" : "0");
    verifyErrorNeedSaveSettings("NTP", NeedSaveSettings.bit.NTP, g_p_NTP_settings_ROM);
    if (NeedSaveSettings.bit.NTP)
    {
        LoadInMemorySettingsNTP();
        rsdebugDnflnF("Сохраняем данные NTP");
        memcpy(&_p_all_settings_ROM->NTP_settings_ROM, g_p_NTP_settings_ROM, len_NTP_settings_ROM);
        EmptyMemorySettingsNTP(true);
    }

    rsdebugDnfln("bit.OTA: %s", NeedSaveSettings.bit.OTA ? "1" : "0");
    rsdebugDnfln("bit.RSDebug: %s", NeedSaveSettings.bit.RSDebug ? "1" : "0");
    rsdebugDnfln("bit.SysMon: %s", NeedSaveSettings.bit.SysMon ? "1" : "0");
    verifyErrorNeedSaveSettings("OTA", NeedSaveSettings.bit.OTA, g_p_sys_settings_ROM);
    verifyErrorNeedSaveSettings("RSDebug", NeedSaveSettings.bit.RSDebug, g_p_sys_settings_ROM);
    verifyErrorNeedSaveSettings("SysMon", NeedSaveSettings.bit.SysMon, g_p_sys_settings_ROM);
    if (NeedSaveSettings.bit.OTA || NeedSaveSettings.bit.RSDebug || NeedSaveSettings.bit.SysMon)
    {
        LoadInMemorySettingsSys();
        rsdebugDnflnF("Сохраняем данные SYS");
        memcpy(&_p_all_settings_ROM->sys_settings_ROM, g_p_sys_settings_ROM, len_sys_settings_ROM);
        EmptyMemorySettingsSys(true);
    }

    rsdebugDnfln("bit.WIFI: %s", NeedSaveSettings.bit.WIFI ? "1" : "0");
    rsdebugDnfln("bit.MQTT: %s", NeedSaveSettings.bit.MQTT ? "1" : "0");
    verifyErrorNeedSaveSettings("WiFi", NeedSaveSettings.bit.WIFI, g_p_ethernet_settings_ROM);
    verifyErrorNeedSaveSettings("MQTT", NeedSaveSettings.bit.MQTT, g_p_ethernet_settings_ROM);
    if (NeedSaveSettings.bit.WIFI || NeedSaveSettings.bit.MQTT)
    {
        LoadInMemorySettingsEthernet();
        rsdebugDnflnF("Сохраняем данные ethernet");
        memcpy(&_p_all_settings_ROM->ethernet_settings_ROM, g_p_ethernet_settings_ROM, len_ethernet_settings_ROM);
        EmptyMemorySettingsEthernet(true);
    }

    calcCRC(_p_all_settings_ROM, len_all_settings_ROM);
    EEPROM.getDataPtr(); // установить флаг "данные изменились" для сохранения при EEPROM.end()
    EEPROM.end();
}

void NotSaveEmptyMemorySettings(void)
{
    EmptyMemorySettingsNTP(true);
    EmptyMemorySettingsSys(true);
    EmptyMemorySettingsEthernet(true);
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

void cb_emptymemory(void)
{
    if (NeedSaveSettings.value)
    {
        rsdebugDnfln("cb_emptymemory: %d", NeedSaveSettings.value);
        rsdebugDnfln("bit.NTP: %s", NeedSaveSettings.bit.NTP ? "1" : "0");
        rsdebugDnfln("bit.OTA: %s", NeedSaveSettings.bit.OTA ? "1" : "0");
        rsdebugDnfln("bit.RSDebug: %s", NeedSaveSettings.bit.RSDebug ? "1" : "0");
        rsdebugDnfln("bit.SysMon: %s", NeedSaveSettings.bit.SysMon ? "1" : "0");
        rsdebugDnfln("bit.WIFI: %s", NeedSaveSettings.bit.WIFI ? "1" : "0");
        rsdebugDnfln("bit.MQTT: %s", NeedSaveSettings.bit.MQTT ? "1" : "0");
        if (u.state_WiFi == _wifi_connected)
            EmptyMemorySettingsEthernet();
        EmptyMemorySettingsSys();
        EmptyMemorySettingsNTP();
        // EmptyMemorySettingsNTC();
    }
}
