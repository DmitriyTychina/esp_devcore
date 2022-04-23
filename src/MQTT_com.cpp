#include <Arduino.h>
// #include <NtpClientLib.h>
#include <EEPROM.h>

#include "my_MQTT.h"
#include "my_EEPROM.h"
#include "my_scheduler.h"
#include "my_debuglog.h"
#include "MQTT_pub.h"
#include "MQTT_com.h"

#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
// include
#include "my_door.h"
// #include "my_NTP.h"
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

#define not_AP 0xfefe

#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN

// void Modify_NTP_IP(const char *char_str, int _numIP)
// {
//     if (strcmp(char_str, g_p_NTP_settings_ROM->serversNTP[_numIP])) // если не совпадает
//     {
//         strcpy(g_p_NTP_settings_ROM->serversNTP[_numIP], char_str);
//         NeedSaveSettings.bit.NTP = true;
//         MQTT_pub_Info_NeedSaveSettings("NeedSave_NTP");
//         init_NTP_with_WiFi();
//     }
// }

// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

// void MQTT_com_SaveSettings(s_element_MQTT _element)
// {
//     // debuglog_print_free_memory();
//     const char *payload = _element.payload->c_str();
//     if (!strcmp(payload, "ok"))
//         return;
//     else if (!strncmp(payload, "NeedSave", 8))
//         return;
//     else if (!strcmp(payload, "save"))
//     {
//         if (NeedSaveSettings.value)
//         {
//             SaveAllSettings();
//             NeedSaveSettings.value = 0;
//         }
//     }
//     else if (!strcmp(payload, "notsave"))
//     {
//         NotSaveEmptyMemorySettings();
//         NeedSaveSettings.value = 0;
//     }
//     // else
//     //     return;
//     if (NeedSaveSettings.value)
//     {
//         e_IDDirTopic dirs_topic[] = {d_main_topic, d_settings, d_empty, d_empty};
//         mqtt_publish(dirs_topic, _Save, "NeedSave");
//         return;
//     }
//     else
//         MQTT_pub_Settings_ok(_Save);
//     // debuglog_print_free_memory();
// }
void cb_MQTT_com_Info(s_element_MQTT _element)
{
    uint8_t payload_len = strlen(_element.payload);
    rsdebugInfF("MQTT in->Info:");
    rsdebugInfln("%s<%s$%d>", _element.topic, _element.payload, payload_len);
    char arr_dirs[8][16] = {"", "", "", "", ""}; // [уровней][символов]
    uint16_t i = 0;
    char *pch = strtok(_element.topic, "/");
    while (pch != NULL)
    {
        strcpy(arr_dirs[i], pch);
        // rsdebugDln("dir[%d]: %s", i, arr_dirs[i]);
        i++;
        pch = strtok(NULL, "/");
    }
    if (!strcmp(arr_dirs[0], ArrDirTopic[d_main_topic]))
    {
        if (!strcmp(arr_dirs[1], ArrDirTopic[d_info]))
        {
            e_IDDirTopic dirs_topic[] = {d_main_topic, d_info, d_empty, d_empty};
            if (!strcmp(arr_dirs[2], ArrVarTopic[vc_Read]))
            {
                // dirs_topic[2] = _Read;
                if (is_equal_ok(_element.payload) || is_equal_no(_element.payload))
                    return;
                else if (is_equal_enable(_element.payload))
                {
                    rsdebugInflnF("Публикуем инфо");
                    MQTT_pub_allInfo(true);
                    mqtt_publish_ok(dirs_topic, vc_Read);
                }
                else
                    mqtt_publish_no(dirs_topic, vc_Read);
                return;
            }
        }
    }
}
void cb_MQTT_com_Settings(s_element_MQTT _element)
{
    // EmptyMemorySettingsNTC();
    // debuglog_print_free_memory();
    // char *topic = (char *)_element.topic->c_str();
    // char *topic = _element.topic;
    // _element.payload->toLowerCase();
    // char *payload = (char *)_element.payload->c_str();
    // char *payload = _element.payload;
    uint8_t payload_len = strlen(_element.payload);
    rsdebugInfF("MQTT in->Settings:");
    rsdebugInfln("%s<%s$%d>", _element.topic, _element.payload, payload_len);
    // const char ArrVarTopic[_LastElement_e_IDVarTopic][14]
    // const char ArrDirTopic[_LastElement_e_IDDirTopic][10]
    // char arr_dirs[10][14] = {"", "", "", "", "", "", "", "", ""};
    char arr_dirs[8][16] = {"", "", "", "", ""}; // [уровней][символов]
    uint16_t i = 0;
    char *pch = strtok(_element.topic, "/");
    while (pch != NULL)
    {
        strcpy(arr_dirs[i], pch);
        // rsdebugDln("dir[%d]: %s", i, arr_dirs[i]);
        i++;
        pch = strtok(NULL, "/");
    }
    if (!strcmp(arr_dirs[0], ArrDirTopic[d_main_topic]))
    {
        if (!strcmp(arr_dirs[1], ArrDirTopic[d_settings]))
        {
            LoadInMemorySettingsSys();
            LoadInMemorySettingsEthernet();
            LoadInMemorySettingsNTP();
            e_IDDirTopic dirs_topic[] = {d_main_topic, d_settings, d_empty, d_empty};
            if (!strcmp(arr_dirs[2], ArrDirTopic[d_rs_debug]))
            {
                dirs_topic[2] = d_rs_debug;
                if (!strcmp(arr_dirs[3], ArrVarTopic[v_t_task]))
                {
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_debuglog, 10, 1000, &g_p_sys_settings_ROM->RSDebug_Ttask))
                        NeedSaveSettings.sys.bit.RSDebug_T_task = true;
                    else
                        return;
                }
                else if (!strcmp(arr_dirs[3], ArrDirTopic[d_s_debug]))
                {
                    dirs_topic[3] = d_s_debug;
                    // rsdebugDlnF("***d_s_debug");
                    if (!strcmp(arr_dirs[4], ArrVarTopic[v_enable]))
                    {
                        uint8_t tmp_result = Modify_bool(dirs_topic, v_enable, _element.payload, &g_p_sys_settings_ROM->RSDebug_SDebug, Debug.isSdebugEnabled()); // в минутах: от 1мин до 24ч
                        if (!tmp_result)                                                                                                                          // 0 - нет изменеий
                            return;
                        if (tmp_result & 1) // 1 или 3 - payload != mem_data
                        {
                            NeedSaveSettings.sys.bit.RSDebug_SD_en = true;
                        }
                        if (tmp_result & 2) // 2 или 3 - payload != real_data
                        {
                            Debug.setSdebugEnabled(g_p_sys_settings_ROM->RSDebug_SDebug);
                        }
                    }
                    else
                        return;
                }
                else if (!strcmp(arr_dirs[3], ArrDirTopic[d_r_debug]))
                {
                    dirs_topic[3] = d_r_debug;
                    // rsdebugDlnF("***d_r_debug");
                    if (!strcmp(arr_dirs[4], ArrVarTopic[v_enable]))
                    {
                        uint8_t tmp_result = Modify_bool(dirs_topic, v_enable, _element.payload, &g_p_sys_settings_ROM->RSDebug_RDebug, Debug.isRdebugEnabled()); // в минутах: от 1мин до 24ч
                        if (!tmp_result)                                                                                                                          // 0 - нет изменеий
                            return;
                        if (tmp_result & 1) // 1 или 3 - payload != mem_data
                        {
                            NeedSaveSettings.sys.bit.RSDebug_RD_en = true;
                        }
                        if (tmp_result & 2) // 2 или 3 - payload != real_data
                        {
                            Debug.setRdebugEnabled(g_p_sys_settings_ROM->RSDebug_RDebug);
                                // init_rdebuglog();
                        }
                    }
                    else
                        return;
                }
                else
                    return;
            }
            else if (!strcmp(arr_dirs[2], ArrDirTopic[d_sysmon]))
            {
                dirs_topic[2] = d_sysmon;
                if (!strcmp(arr_dirs[3], ArrVarTopic[v_t_task]))
                {
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_sysmon, 1000, 3600000, &g_p_sys_settings_ROM->SysMon_Ttask))
                        NeedSaveSettings.sys.bit.SysMon_T_task = true;
                    else
                        return;
                }
                else
                    return;
            }
            else if (!strcmp(arr_dirs[2], ArrDirTopic[d_ota]))
            {
                dirs_topic[2] = d_ota;
                if (!strcmp(arr_dirs[3], ArrVarTopic[v_t_task]))
                {
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_OTA, 1, 5000, &g_p_sys_settings_ROM->OTA_Ttask))
                        NeedSaveSettings.sys.bit.OTA_T_task = true;
                    else
                        return;
                }
                else
                    return;
            }
            else if (!strcmp(arr_dirs[2], ArrDirTopic[d_mqtt]))
            {
                dirs_topic[2] = d_mqtt;
                if (!strcmp(arr_dirs[3], ArrVarTopic[v_t_task]))
                {
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_MQTT, 1, 1000, &g_p_ethernet_settings_ROM->MQTT_Ttask))
                        NeedSaveSettings.bit.MQTT = true;
                    else
                        return;
                }
                else if (!strcmp(arr_dirs[3], ArrVarTopic[v_user]))
                {
                    if (Modify_pass(dirs_topic, v_user, _element.payload, (char *)&g_p_ethernet_settings_ROM->MQTT_user))
                        NeedSaveSettings.bit.MQTT = true;
                    else
                        return;
                }
                else if (!strcmp(arr_dirs[3], ArrVarTopic[v_pass]))
                {
                    if (Modify_pass(dirs_topic, v_pass, _element.payload, (char *)&g_p_ethernet_settings_ROM->MQTT_pass))
                        NeedSaveSettings.bit.MQTT = true;
                    else
                        return;
                }
                else
                    return;
            }
            else if (!strcmp(arr_dirs[2], ArrDirTopic[d_wifi]))
            {
                dirs_topic[2] = d_wifi;
                if (!strcmp(arr_dirs[3], ArrVarTopic[v_t_task]))
                {
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_wifi, 1, 500, &g_p_ethernet_settings_ROM->WiFi_Ttask))
                        NeedSaveSettings.bit.WIFI = true;
                    else
                        return;
                }
                else
                {
                    uint16_t num_AP = not_AP;
                    e_IDDirTopic a_dirs[] = {d_ap1, d_ap2, d_ap3, d_ap4, d_ap5};
                    for (uint16_t x = 0; x < sizeof(a_dirs) / sizeof(a_dirs[0]); x++)
                    {
                        if (!strcmp(arr_dirs[3], ArrDirTopic[a_dirs[x]]))
                        {
                            num_AP = x;
                            break;
                        }
                    }
                    if (num_AP != not_AP)
                    {
                        dirs_topic[3] = a_dirs[num_AP];
                        if (!strcmp(arr_dirs[4], ArrVarTopic[v_ssid]))
                        {
                            if (Modify_string(dirs_topic, v_ssid, _element.payload, (char *)&g_p_ethernet_settings_ROM->settings_serv[num_AP].SSID))
                                NeedSaveSettings.bit.WIFI = true;
                            else
                                return;
                        }
                        else if (!strcmp(arr_dirs[4], ArrVarTopic[v_pass]))
                        {
                            if (Modify_pass(dirs_topic, v_pass, _element.payload, (char *)&g_p_ethernet_settings_ROM->settings_serv[num_AP].PASS))
                                NeedSaveSettings.bit.WIFI = true;
                            else
                                return;
                        }
                        else if (!strcmp(arr_dirs[4], ArrVarTopic[v_ip_serv]))
                        {
                            if (Modify_string(dirs_topic, v_ip_serv, _element.payload, (char *)&g_p_ethernet_settings_ROM->settings_serv[num_AP].MQTTip))
                                NeedSaveSettings.bit.WIFI = true;
                            else
                                return;
                        }
                        else
                            return;
                    }
                    else
                        return;
                }
                // EmptyMemorySettingsEthernet();
            }
            else if (!strcmp(arr_dirs[2], ArrDirTopic[d_ntp]))
            {
                // rsdebugDnfln("@0 d_ntp");
                dirs_topic[2] = d_ntp;
                if (!strcmp(arr_dirs[3], ArrVarTopic[v_t_task]))
                {
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_NTP, 100, 5000, &g_p_NTP_settings_ROM->Ttask))
                        NeedSaveSettings.bit.NTP = true;
                    else
                        return;
                }
                else if (!strcmp(arr_dirs[3], ArrVarTopic[v_t_sync]))
                {
                    // rsdebugDnfln("@1 d_ntp");
                    uint8_t tmp_result = Modify_num(dirs_topic, v_t_sync, _element.payload, 1, 1440, &g_p_NTP_settings_ROM->T_syncNTP, NTP.getInterval() / 60); // в минутах: от 1мин до 24ч
                    if (!tmp_result)                                                                                                                            // 0 - нет изменеий
                        return;
                    if (tmp_result & 1) // 1 или 3 - payload != mem_data
                    {
                        NeedSaveSettings.bit.NTP = true;
                    }
                    if (tmp_result & 2) // 2 или 3 - payload != real_data
                    {
                        NTP.setInterval(g_p_NTP_settings_ROM->T_syncNTP * 60);
                    }
                    // rsdebugDnfln("@2 d_ntp");
                }
                else if (!strcmp(arr_dirs[3], ArrVarTopic[v_timezone]))
                {
                    uint8_t tmp_result = Modify_num(dirs_topic, v_timezone, _element.payload, -23, 23, &g_p_NTP_settings_ROM->timezone, NTP.getTimeZone()); // в минутах: от 1мин до 24ч
                    if (!tmp_result)                                                                                                                        // 0 - нет изменеий
                        return;
                    if (tmp_result & 1) // 1 или 3 - payload != mem_data
                    {
                        NeedSaveSettings.bit.NTP = true;
                    }
                    if (tmp_result & 2) // 2 или 3 - payload != real_data
                    {
                        NTP.setTimeZone(g_p_NTP_settings_ROM->timezone);
                    }
                }
                else if (!strcmp(arr_dirs[3], ArrVarTopic[v_ip1]))
                {
                    if (Modify_string(dirs_topic, v_ip1, _element.payload, (char *)&g_p_NTP_settings_ROM->serversNTP[0]))
                        NeedSaveSettings.bit.NTP = true;
                    else
                        return;
                }
                else if (!strcmp(arr_dirs[3], ArrVarTopic[v_ip2]))
                {
                    if (Modify_string(dirs_topic, v_ip2, _element.payload, (char *)&g_p_NTP_settings_ROM->serversNTP[1]))
                        NeedSaveSettings.bit.NTP = true;
                    else
                        return;
                }
                else if (!strcmp(arr_dirs[3], ArrVarTopic[v_ip3]))
                {
                    if (Modify_string(dirs_topic, v_ip3, _element.payload, (char *)&g_p_NTP_settings_ROM->serversNTP[2]))
                        NeedSaveSettings.bit.NTP = true;
                    else
                        return;
                }
                // rsdebugDnfln("@3 d_ntp");
                init_NTP_with_WiFi();
                if (!NeedSaveSettings.bit.NTP)
                    return;
            }
            else if (!strcmp(arr_dirs[2], ArrVarTopic[vc_ReadDflt]))
            {
                if (is_equal_ok(_element.payload) || is_equal_no(_element.payload))
                    return;
                else if (is_equal_enable(_element.payload))
                {
                    rsdebugInflnF("Загружаем данные по умолчанию");
                    MQTT_pub_allSettings(false); // данные по умолчанию
                    mqtt_publish_ok(dirs_topic, vc_ReadDflt);
                    mqtt_publish_no(dirs_topic, vc_ReadCrnt);
                }
                else
                {
                    mqtt_publish_no(dirs_topic, vc_ReadDflt);
                }
                return;
            }
            else if (!strcmp(arr_dirs[2], ArrVarTopic[vc_ReadCrnt]))
            {
                // dirs_topic[2] = _Read;
                if (is_equal_ok(_element.payload) || is_equal_no(_element.payload))
                    return;
                else if (is_equal_enable(_element.payload))
                {
                    rsdebugInflnF("Загружаем текущие данные");
                    MQTT_pub_allSettings(true); // данные из памяти/ROM
                    mqtt_publish_ok(dirs_topic, vc_ReadCrnt);
                    mqtt_publish_no(dirs_topic, vc_ReadDflt);
                }
                else
                    mqtt_publish_no(dirs_topic, vc_ReadCrnt);
                return;
            }
            else if (!strcmp(arr_dirs[2], ArrVarTopic[vc_Save]))
            {
                // dirs_topic[2] = _Save;
                // rsdebugDnflnF("@1");
                if (NeedSaveSettings.all)
                {
                    if (!strcmp(_element.payload, "NeedSave"))
                    {
                        // rsdebugDnflnF("@2");
                        return;
                    }
                    else if (is_equal_enable(_element.payload))
                    {
                        // rsdebugDnflnF("@3");
                        if (SaveAllSettings())
                        {
                            // rsdebugDnflnF("@4");
                            mqtt_publish_ok(dirs_topic, vc_Save);
                        }
                        else
                        {
                            // rsdebugDnflnF("@5");
                            mqtt_publish_no(dirs_topic, vc_Save);
                        }
                        return;
                    }
                    else if (is_equal_disable(_element.payload))
                    {
                        // rsdebugDnflnF("@6");
                        if (NeedSaveSettings.all)
                        {
                            // rsdebugDnflnF("@7");
                            rsdebugInflnF("Не сохраняем, загружаем данные из ROM");
                            NotSaveEmptyMemorySettings(); // не сохраняем текущие
                            MQTT_pub_allSettings(true);   // публикуем данные из ROM
                            mqtt_publish_ok(dirs_topic, vc_Save);
                            mqtt_publish_no(dirs_topic, vc_ReadDflt);
                            mqtt_publish_no(dirs_topic, vc_ReadCrnt);
                        }
                        return;
                    }
                }
                else if (is_equal_ok(_element.payload))
                {
                    // rsdebugDnflnF("@8");
                    return;
                }
                // rsdebugDnflnF("@9");
            }
            else if (!strcmp(arr_dirs[2], ArrVarTopic[vc_Debug]))
            {
                if (is_equal_ok(_element.payload) || is_equal_no(_element.payload))
                    return;
                else if (!strcmp(_element.payload, "timereset"))
                {
                    MQTT_pub_Info_TimeReset();
                    mqtt_publish_ok(dirs_topic, vc_Debug);
                }
                else if (!strcmp(_element.payload, "reset"))
                {
                    rsdebugWnflnF("Restart ESP");
                    // e_IDDirTopic dirs_topic[] = {d_main_topic, d_settings, d_empty, d_empty};
                    mqtt_publish(dirs_topic, vc_Debug, "Restarting...");
                    // os_delay_us(500000);
                    ESP.restart(); // Reset
                }
                else
                {
                    mqtt_publish(dirs_topic, vc_Debug, "no");
                }
                return;
            }
            else
                return;
            dirs_topic[2] = d_empty;
            dirs_topic[3] = d_empty;
            // rsdebugDnfln("@ NeedSaveSettings.all=%d", NeedSaveSettings.all);
            if (NeedSaveSettings.all)
            {
                // rsdebugDnfln("@ mqtt_publish NeedSave vc_Save");
                mqtt_publish(dirs_topic, vc_Save, "NeedSave");
            }
            else
            {
                // rsdebugDnfln("@ mqtt_publish_ok vc_Save");
                mqtt_publish_ok(dirs_topic, vc_Save);
            }
        }
    }
}
// else if (!strcmp(payload, "getipntp"))
// {
//     e_IDDirTopic dirs_topic[] = {d_main_topic, d_settings, d_ntp, d_empty};
//     MQTT_pub_Settings_NTP_IP(dirs_topic, v_ip1, 0);
//     MQTT_pub_Settings_NTP_IP(dirs_topic, v_ip2, 1);
//     MQTT_pub_Settings_NTP_IP(dirs_topic, v_ip3, 2);
//     EmptyMemorySettingsNTP();
// }

bool Modify_string(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, char _data[])
{
    if (strlen(_payload) == 0)
        mqtt_publish(_dir_topic, _IDVarTopic, _data);
    else if (strcmp(_data, _payload) != 0) // не совпадает
    {
        strcpy(_data, _payload);
        return true;
    }
    return false;
}

bool Modify_pass(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, char _data[])
{
    if (strlen(_payload) == 0)
        mqtt_publish(_dir_topic, _IDVarTopic, "*****");
    else if (strcmp(_payload, "*****") != 0) // не совпадает
    {
        if (strcmp(_data, _payload) != 0) // не совпадает
        {
            strcpy(_data, _payload);
            return true;
        }
    }
    return false;
}

bool Modify_task_tTask(e_IDDirTopic *_dir_topic, const char *_payload, uTask *_task, uint32_t _tMin, uint32_t _tMax, uint32_t *p_T_task)
{
    uint32_t _payload_T_task = atoi(_payload);
    if ((_payload_T_task >= _tMin && _payload_T_task <= _tMax) || (_payload_T_task == 0 && strlen(_payload) == 1 && _payload[0] == '0')) // в диапазоне, а 0 - число
    {
        if (_task->getInterval() != _payload_T_task)
        {
            _task->enable();
            _task->setInterval(_payload_T_task);
            *p_T_task = _payload_T_task;
            return true;
        }
    }
    else
    {
        mqtt_publish(_dir_topic, v_t_task, (uint32_t)_task->getInterval());
    }
    return false;
}

uint8_t Modify_num(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, int32_t _tMin, int32_t _tMax, int32_t *_mem_num, int32_t _real_val)
{
    int32_t num = atoi(_payload);
    uint8_t ret = 0;
    if (num >= _tMin && num <= _tMax)
    {
        if (*_mem_num != num)
        {
            *_mem_num = num;
            ret = 1;
        }
    }
    else
    {
        mqtt_publish(_dir_topic, _IDVarTopic, *_mem_num);
    }
    if (*_mem_num != _real_val)
        ret |= 2;
    return ret;
}
uint8_t Modify_num(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, uint32_t _tMin, uint32_t _tMax, uint32_t *_mem_num, uint32_t _real_val)
{
    uint32_t num = atoi(_payload);
    uint8_t ret = 0;
    if (num >= _tMin && num <= _tMax)
    {
        if (*_mem_num != num)
        {
            *_mem_num = num;
            ret = 1;
        }
    }
    else
    {
        mqtt_publish(_dir_topic, _IDVarTopic, *_mem_num);
    }
    if (*_mem_num != _real_val)
        ret |= 2;
    return ret;
}
uint8_t Modify_num(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, uint16_t _tMin, uint16_t _tMax, uint16_t *_mem_num, uint16_t _real_val)
{
    uint16_t num = atoi(_payload);
    uint8_t ret = 0;
    if (num >= _tMin && num <= _tMax)
    {
        if (*_mem_num != num)
        {
            *_mem_num = num;
            ret = 1;
        }
    }
    else
    {
        mqtt_publish(_dir_topic, _IDVarTopic, *_mem_num);
    }
    if (*_mem_num != _real_val)
        ret |= 2;
    return ret;
}
uint8_t Modify_num(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, int8_t _tMin, int8_t _tMax, int8_t *_mem_num, int8_t _real_val)
{
    int8_t num = atoi(_payload);
    uint8_t ret = 0;
    if (num >= _tMin && num <= _tMax)
    {
        if (*_mem_num != num)
        {
            *_mem_num = num;
            ret = 1;
        }
    }
    else
    {
        mqtt_publish(_dir_topic, _IDVarTopic, *_mem_num);
    }
    if (*_mem_num != _real_val)
        ret |= 2;
    return ret;
}
uint8_t Modify_num(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, uint8_t _tMin, uint8_t _tMax, uint8_t *_mem_num, uint8_t _real_val)
{
    uint8_t num = atoi(_payload);
    uint8_t ret = 0;
    if (num >= _tMin && num <= _tMax)
    {
        if (*_mem_num != num)
        {
            *_mem_num = num;
            ret = 1;
        }
    }
    else
    {
        mqtt_publish(_dir_topic, _IDVarTopic, *_mem_num);
    }
    if (*_mem_num != _real_val)
        ret |= 2;
    return ret;
}
// 0 - нет изменеий
// 1 - payload != mem_data
// 2 - payload != real_data
// 3 - payload != mem_data и payload != real_data
uint8_t Modify_bool(e_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, bool *_mem_bool, bool _real_val)
{
    uint8_t ret = 0;
    if (is_equal_enable(_payload))
    {
        if (*_mem_bool != true)
        {
            *_mem_bool = true;
            ret = 1;
        }
    }
    else if (is_equal_disable(_payload))
    {
        if (*_mem_bool != false)
        {
            *_mem_bool = false;
            ret = 1;
        }
    }
    else
    {
        mqtt_publish(_dir_topic, _IDVarTopic, *_mem_bool);
    }
    if (*_mem_bool != _real_val)
        ret |= 2;
    return ret;
}
