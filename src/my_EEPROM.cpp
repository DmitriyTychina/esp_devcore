#include <Arduino.h>

#include "my_EEPROM.h"
#include "my_debuglog.h"

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
        CRC = ((CRC >> 4) & 0xfff) ^ pgm_read_word/*_aligned*/(crc_tabl + Ind);
        tmp >>= 4;
        Ind = ((CRC ^ tmp) & 0xf);
        CRC = ((CRC >> 4) & 0xfff) ^ pgm_read_word/*_aligned*/(crc_tabl + Ind);
    }
    return (CRC);
}

#if defined(EEPROM_C)
u_NeedSaveSettings NeedSaveSettings;
void LoadInMemorySettings(char *_name, void **p_MemorySettings, uint16_t _addr, uint16_t _size)
{
    if (!*p_MemorySettings)
    {
        rsdebugInfF("Загружаем настройки ");
        rsdebugInfln("%s из ROM$%d_of_%d", _name, _size, _addr);
        if (!*p_MemorySettings) // если нет указателя на структуру
        {
            *p_MemorySettings = malloc(_size); // выделяем память и создаем указатель
            if (!*p_MemorySettings)            // если нет указателя на структуру
            {
                rsdebugEnfF("Не смогли выделить память для данных из ROM: ");
                rsdebugEnfln("%d байт", _size);
                return;
            }
            EEPROM.begin(len_all_settings_ROM);
            uint8_t *_p_all_settings_ROM = (uint8_t *)EEPROM.getConstDataPtr();
            if (!_p_all_settings_ROM)
            {
                EEPROM.end();
                rsdebugEnflnF("Нет указателя на данные из ROM");
                return;
            }
            memcpy(*p_MemorySettings, _p_all_settings_ROM + _addr, _size);
            EEPROM.end();
        }
    }
    ut_emptymemory.suspendNextCall(emptymemory_TtaskDefault);
}

void EmptyMemorySettingsSys(bool force) // force=true для очистки памяти независимо от NeedSaveSettings.bit
{
    if (NeedSaveSettings.sys.of.OTA)
        rsdebugInflnF("Перед очищением Sys: OTA не сохранены");
    if (NeedSaveSettings.sys.of.RSDebug)
        rsdebugInflnF("Перед очищением Sys: RSDebug не сохранены");
    if (NeedSaveSettings.sys.of.SysMon)
        rsdebugInflnF("Перед очищением Sys: SysMon не сохранены");
    if (force || !NeedSaveSettings.sys.all)
    {
        if (g_p_sys_settings_ROM)
        {
            rsdebugInfF("Очищаем память от настроек Sys");
            rsdebugInfln("$%d", len_sys_settings_ROM);
            delete g_p_sys_settings_ROM;
            g_p_sys_settings_ROM = NULL;
            NeedSaveSettings.sys.all = 0;
        }
    }
}

void EmptyMemorySettingsEthernet(bool force) // force=true для очистки памяти независимо от NeedSaveSettings.bit
{
    if (NeedSaveSettings.bit.WIFI)
        rsdebugInflnF("Перед очищением настроек связи: WIFI не сохранены");
    if (NeedSaveSettings.bit.MQTT)
        rsdebugInflnF("Перед очищением настроек связи: MQTT не сохранены");
    if (force || (!NeedSaveSettings.bit.WIFI && !NeedSaveSettings.bit.MQTT))
    {
        if (g_p_ethernet_settings_ROM)
        {
            rsdebugInfF("Очищаем память от настроек связи");
            rsdebugInfln("$%d", len_ethernet_settings_ROM);
            // EmptySettingsEthernet(&g_p_ethernet_settings_ROM);
            // EmptySettings(g_p_ethernet_settings_ROM);
            delete g_p_ethernet_settings_ROM;
            g_p_ethernet_settings_ROM = NULL;

            NeedSaveSettings.bit.WIFI = false;
            NeedSaveSettings.bit.MQTT = false;
        }
    }
}

void EmptyMemorySettingsNTP(bool force) // force=true для очистки памяти независимо от NeedSaveSettings.bit
{
    if (g_p_NTP_settings_ROM)
    {
        if (NeedSaveSettings.bit.NTP)
            rsdebugInflnF("Перед очищением NTP: NTP не сохранены");
        if (force || !NeedSaveSettings.bit.NTP)
        {
            rsdebugInfF("Очищаем память от настроек NTP");
            rsdebugInfln("$%d", len_NTP_settings_ROM);
            delete g_p_NTP_settings_ROM;
            g_p_NTP_settings_ROM = NULL;
            NeedSaveSettings.bit.NTP = false;
        }
    }
}

void verifyErrorNeedSaveSettings(const char *nameBit, bool _bit, void *settings_ROM)
{
    if (_bit && !settings_ROM)
    {
        rsdebugEnfln("NeedSaveSettings.bit.%s=true, но нет указателя на данные", nameBit)
    }
}

bool SaveAllSettings(void)
{
    if (NeedSaveSettings.all)
    {
        EEPROM.begin(len_all_settings_ROM);
        s_all_settings_ROM *_p_all_settings_ROM = (s_all_settings_ROM *)EEPROM.getConstDataPtr();
        if (_p_all_settings_ROM) // если нет указателя
        {
            rsdebugInflnF("Сохраняем все данные:");
            rsdebugDnfln("bit.RSDebug: %d", NeedSaveSettings.sys.of.RSDebug);
            rsdebugDnfln("bit.OTA: %d", NeedSaveSettings.sys.of.OTA);
            rsdebugDnfln("bit.SysMon: %d", NeedSaveSettings.sys.of.SysMon);
            verifyErrorNeedSaveSettings("RSDebug", NeedSaveSettings.sys.of.RSDebug, g_p_sys_settings_ROM);
            verifyErrorNeedSaveSettings("OTA", NeedSaveSettings.sys.of.OTA, g_p_sys_settings_ROM);
            verifyErrorNeedSaveSettings("SysMon", NeedSaveSettings.sys.of.SysMon, g_p_sys_settings_ROM);
            if (NeedSaveSettings.sys.all)
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
            rsdebugDnfln("bit.NTP: %s", NeedSaveSettings.bit.NTP ? "1" : "0");
            verifyErrorNeedSaveSettings("NTP", NeedSaveSettings.bit.NTP, g_p_NTP_settings_ROM);
            if (NeedSaveSettings.bit.NTP)
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

#elif defined(EEPROM_CPP)

uint32_t g_last_request_EEPROM;
c_all_settings_def def_data;
c_all_settings_rom rom_data;
c_all_settings_ram ram_data;

bool SaveAllSettings(void)
{
    if (ram_data.unsaved)
    {
        if (!EEPROM.getConstDataPtr())
            EEPROM.begin(len_all_settings_ROM);
        s_all_settings_ROM *_p_all_settings_ROM = (s_all_settings_ROM *)EEPROM.getConstDataPtr();
        if (_p_all_settings_ROM) // если есть указатель
        {
            rsdebugInflnF("Сохраняем все данные:");
            rsdebugDnflnF("Сохраняем данные SYS");
            memcpy(&_p_all_settings_ROM->sys_settings_ROM, ram_data.p_SYS_settings(), len_sys_settings_ROM);
            rsdebugDnflnF("Сохраняем данные ethernet");
            memcpy(&_p_all_settings_ROM->ethernet_settings_ROM, ram_data.p_NET_settings(), len_ethernet_settings_ROM);
#ifdef CORE_NTP
            rsdebugDnflnF("Сохраняем данные NTP");
            memcpy(&_p_all_settings_ROM->NTP_settings_ROM, ram_data.p_NTP_settings(), len_NTP_settings_ROM);
#endif
            calcCRC(_p_all_settings_ROM, len_all_settings_ROM); // вычисляем CRC
            EEPROM.getDataPtr();                                // установить флаг "данные изменились" для сохранения при EEPROM.end()
            ram_data.unsaved = false;
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

void print_free_up_net(bool _flag, String _pre_str)
{
    if (_flag)
    {
        rsdebugInf(_pre_str.c_str());
        rsdebugInfF(": Очищаем память от сетевых настроек");
        rsdebugInfln("$%d", len_ethernet_settings_ROM);
    }
};

void print_free_up_sys(bool _flag, String _pre_str)
{
    if (_flag)
    {
        rsdebugInf(_pre_str.c_str());
        rsdebugInfF(": Очищаем память от системных настроек");
        rsdebugInfln("$%d", len_sys_settings_ROM);
    }
};

#ifdef CORE_NTP
void print_free_up_NTP(bool _flag, String _pre_str)
{
    if (_flag)
    {
        rsdebugInf(_pre_str.c_str());
        rsdebugInfF(": Очищаем память от настроек NTP");
        rsdebugInfln("$%d", len_NTP_settings_ROM);
    }
};
#endif

#endif

// если данные в ROM не валидны - записываем "по-умолчанию"
void ROMVerifySettingsElseSaveDefault(bool force)
{
    EEPROM.begin(len_all_settings_ROM);
    s_all_settings_ROM *_p_all_settings_ROM = (s_all_settings_ROM *)EEPROM.getConstDataPtr();
    if (_p_all_settings_ROM)
    {
        rsdebugInfF("Проверяем CRC ROM");
        rsdebugInfln("$%d", len_all_settings_ROM);
        if (!force && compareCRC(_p_all_settings_ROM, len_all_settings_ROM)) // если данные в памяти валидны
        {
            rsdebugInflnF("Данные в ROM ok");
        }
        else
        {
            rsdebugWnflnF("Данные в ROM not ok");
            s_all_settings_ROM def_all_settings_ROM;
            calcCRC(&def_all_settings_ROM, len_all_settings_ROM);                     // данные по умолчанию не имеют вычесленого CRC !!! - вычисляем
            memcpy(_p_all_settings_ROM, &def_all_settings_ROM, len_all_settings_ROM); // заполняем данными по умолчанию
            rsdebugInflnF("Устанавливаем настройки по умолчанию");
            EEPROM.getDataPtr(); // установить флаг "данные изменились" для сохранения при EEPROM.end()
        }
    }
    else
        rsdebugEnflnF("Нет указателя на данные из ROM");
    EEPROM.end();
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
#if defined(EEPROM_C)
    EmptyMemorySettingsEthernet();
    EmptyMemorySettingsSys();
    EmptyMemorySettingsNTP();
#elif defined(EEPROM_CPP)
    def_data.is_all_del();
    if (!ram_data.unsaved)
        ram_data.is_all_del();
    rom_data.is_all_del();
#endif
}
