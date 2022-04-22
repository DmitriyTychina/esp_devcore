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
//         e_IDDirTopic dir_topic[] = {_main_topic, _Settings, d_empty, d_empty};
//         mqtt_publish(dir_topic, _Save, "NeedSave");
//         return;
//     }
//     else
//         MQTT_pub_Settings_ok(_Save);
//     // debuglog_print_free_memory();
// }

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
    rsdebugInfln("MQTT in->Settings:%s[%s[%d]]", _element.topic, _element.payload, payload_len);
    // const char ArrVarTopic[_LastElement_e_IDVarTopic][14]
    // const char ArrDirTopic[_LastElement_e_IDDirTopic][10]
    // char arr_dir[10][14] = {"", "", "", "", "", "", "", "", ""};
    char arr_dir[8][16] = {"", "", "", "", ""}; // [уровней][символов]
    uint16_t i = 0;
    char *pch = strtok(_element.topic, "/");
    while (pch != NULL)
    {
        strcpy(arr_dir[i], pch);
        // rsdebugDln("dir[%d]: %s", i, arr_dir[i]);
        i++;
        pch = strtok(NULL, "/");
    }
    if (!strcmp(arr_dir[0], ArrDirTopic[_main_topic]))
    {
        if (!strcmp(arr_dir[1], ArrDirTopic[_Settings]))
        {
            LoadInMemorySettingsSys();
            LoadInMemorySettingsEthernet();
            // LoadInMemorySettingsNTP(); // надо добавить настройки NTP
            e_IDDirTopic dir_topic[] = {_main_topic, _Settings, d_empty, d_empty};
            if (!strcmp(arr_dir[2], ArrDirTopic[_RSdebug]))
            {
                dir_topic[2] = _RSdebug;
                if (!strcmp(arr_dir[3], ArrVarTopic[_T_task]))
                {
                    if (Modify_task_tTask(dir_topic, _element.payload, &ut_debuglog, 10, 1000, &g_p_sys_settings_ROM->RSDebug_Ttask))
                        NeedSaveSettings.of.sys.bit.RSDebug_T_task = true;
                    else
                        return;
                }
                else if (!strcmp(arr_dir[3], ArrDirTopic[_Sdebug]))
                {
                    dir_topic[3] = _Sdebug;
                    // rsdebugDlnF("***_Sdebug");
                    if (!strcmp(arr_dir[4], ArrVarTopic[_Enable]))
                    {
                        if (is_equal_enable(_element.payload))
                        {
                            // rsdebugDlnF("***setSdebugEnabled(true)");
                            if (!Debug.isSdebugEnabled())
                            {
                                Debug.setSdebugEnabled(true);
                                g_p_sys_settings_ROM->RSDebug_SDebug = true;
                                NeedSaveSettings.of.sys.bit.RSDebug_SD_en = true;
                                mqtt_publish(dir_topic, _Enable, true);
                                // MQTT_pub_Info_NeedSaveSettings("NeedSave_RSDebug");
                            }
                            else
                                return;
                        }
                        else if (is_equal_disable(_element.payload))
                        {
                            // rsdebugDlnF("***setSdebugEnabled(false)");
                            if (Debug.isSdebugEnabled())
                            {
                                Debug.setSdebugEnabled(false);
                                g_p_sys_settings_ROM->RSDebug_SDebug = false;
                                NeedSaveSettings.of.sys.bit.RSDebug_SD_en = true;
                                mqtt_publish(dir_topic, _Enable, false);
                                // MQTT_pub_Info_NeedSaveSettings("NeedSave_RSDebug");
                            }
                            else
                                return;
                        }
                        else
                        {
                            mqtt_publish(dir_topic, _Enable, Debug.isSdebugEnabled());
                            return;
                        }
                    }
                    else
                        return;
                }
                else if (!strcmp(arr_dir[3], ArrDirTopic[_Rdebug]))
                {
                    dir_topic[3] = _Rdebug;
                    // rsdebugDlnF("***_Rdebug");
                    if (!strcmp(arr_dir[4], ArrVarTopic[_Enable]))
                    {
                        // rsdebugDnflnF("*1");
                        if (is_equal_enable(_element.payload))
                        {
                            // rsdebugDnflnF("*3");
                            // rsdebugDlnF("***setRdebugEnabled(true)");
                            if (!Debug.isRdebugEnabled())
                            {
                                // rsdebugDnflnF("*4");
                                // Debug.setRdebugEnabled(true);
                                g_p_sys_settings_ROM->RSDebug_RDebug = true;
                                NeedSaveSettings.of.sys.bit.RSDebug_RD_en = true;
                                // MQTT_pub_Info_NeedSaveSettings("NeedSave_RSDebug");
                                init_rdebuglog();
                                mqtt_publish(dir_topic, _Enable, true);
                            }
                            else
                                return;
                            // if (!t_debuglog.isEnabled())
                            // t_debuglog.enable();
                        }
                        else if (is_equal_disable(_element.payload))
                        {
                            // rsdebugDnflnF("*5");
                            // rsdebugDlnF("***setRdebugEnabled(false)");
                            if (Debug.isRdebugEnabled())
                            {
                                // rsdebugDnflnF("*6");
                                // Debug.setRdebugEnabled(false);
                                // Debug.disconnectClient();
                                g_p_sys_settings_ROM->RSDebug_RDebug = false;
                                NeedSaveSettings.of.sys.bit.RSDebug_RD_en = true;
                                // MQTT_pub_Info_NeedSaveSettings("NeedSave_RSDebug");
                                // Debug.stop();
                                init_rdebuglog();
                                mqtt_publish(dir_topic, _Enable, false);
                            }
                            else
                                return;
                            // if (t_debuglog.isEnabled())
                            // t_debuglog.disable();
                        }
                        else
                        {
                            // rsdebugDnflnF("*2");
                            mqtt_publish(dir_topic, _Enable, Debug.isRdebugEnabled());
                            return;
                        }
                    }
                    else
                        return;
                }
                else
                    return;
            }
            else if (!strcmp(arr_dir[2], ArrDirTopic[_SysMon]))
            {
                dir_topic[2] = _SysMon;
                if (!strcmp(arr_dir[3], ArrVarTopic[_T_task]))
                {
                    if (Modify_task_tTask(dir_topic, _element.payload, &ut_sysmon, 1000, 3600000, &g_p_sys_settings_ROM->SysMon_Ttask))
                        NeedSaveSettings.of.sys.bit.SysMon_T_task = true;
                    else
                        return;
                }
                else
                    return;
            }
            else if (!strcmp(arr_dir[2], ArrDirTopic[_OTA]))
            {
                dir_topic[2] = _OTA;
                if (!strcmp(arr_dir[3], ArrVarTopic[_T_task]))
                {
                    if (Modify_task_tTask(dir_topic, _element.payload, &ut_OTA, 1, 5000, &g_p_sys_settings_ROM->OTA_Ttask))
                        NeedSaveSettings.of.sys.bit.OTA_T_task = true;
                    else
                        return;
                }
                else
                    return;
            }
            else if (!strcmp(arr_dir[2], ArrDirTopic[_MQTT]))
            {
                dir_topic[2] = _MQTT;
                if (!strcmp(arr_dir[3], ArrVarTopic[_T_task]))
                {
                    if (Modify_task_tTask(dir_topic, _element.payload, &ut_MQTT, 1, 1000, &g_p_ethernet_settings_ROM->MQTT_Ttask))
                        NeedSaveSettings.of.bit.MQTT = true;
                    else
                        return;
                }
                else if (!strcmp(arr_dir[3], ArrVarTopic[_USER]))
                {
                    if (Modify_pass(dir_topic, _USER, _element.payload, (char *)&g_p_ethernet_settings_ROM->MQTT_user))
                        NeedSaveSettings.of.bit.MQTT = true;
                    else
                        return;
                }
                else if (!strcmp(arr_dir[3], ArrVarTopic[_PASS]))
                {
                    if (Modify_pass(dir_topic, _PASS, _element.payload, (char *)&g_p_ethernet_settings_ROM->MQTT_pass))
                        NeedSaveSettings.of.bit.MQTT = true;
                    else
                        return;
                }
                else
                    return;
            }
            else if (!strcmp(arr_dir[2], ArrDirTopic[_WIFI]))
            {
                dir_topic[2] = _WIFI;
                if (!strcmp(arr_dir[3], ArrVarTopic[_T_task]))
                {
                    if (Modify_task_tTask(dir_topic, _element.payload, &ut_wifi, 1, 500, &g_p_ethernet_settings_ROM->WiFi_Ttask))
                        NeedSaveSettings.of.bit.WIFI = true;
                    else
                        return;
                }
                else
                {
                    uint16_t num_AP = not_AP;
                    e_IDDirTopic a_dirs[] = {_AP1, _AP2, _AP3, _AP4, _AP5};
                    for (uint16_t x = 0; x < sizeof(a_dirs) / sizeof(a_dirs[0]); x++)
                    {
                        if (!strcmp(arr_dir[3], ArrDirTopic[a_dirs[x]]))
                        {
                            num_AP = x;
                            break;
                        }
                    }
                    if (num_AP != not_AP)
                    {
                        dir_topic[3] = a_dirs[num_AP];
                        if (!strcmp(arr_dir[4], ArrVarTopic[_SSID]))
                        {
                            if (Modify_string(dir_topic, _SSID, _element.payload, (char *)&g_p_ethernet_settings_ROM->settings_serv[num_AP].SSID))
                                NeedSaveSettings.of.bit.WIFI = true;
                            else
                                return;
                        }
                        else if (!strcmp(arr_dir[4], ArrVarTopic[_PASS]))
                        {
                            if (Modify_pass(dir_topic, _PASS, _element.payload, (char *)&g_p_ethernet_settings_ROM->settings_serv[num_AP].PASS))
                                NeedSaveSettings.of.bit.WIFI = true;
                            else
                                return;
                        }
                        else if (!strcmp(arr_dir[4], ArrVarTopic[_IP_serv]))
                        {
                            if (Modify_string(dir_topic, _IP_serv, _element.payload, (char *)&g_p_ethernet_settings_ROM->settings_serv[num_AP].MQTTip))
                                NeedSaveSettings.of.bit.WIFI = true;
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
            else if (!strcmp(arr_dir[2], ArrVarTopic[_ReadDflt]))
            {
                if (is_equal_ok(_element.payload) || is_equal_no(_element.payload))
                    return;
                else if (is_equal_enable(_element.payload))
                {
                    rsdebugInflnF("Загружаем данные по умолчанию");
                    MQTT_pub_allSettings(false); // данные по умолчанию
                    mqtt_publish_ok(dir_topic, _ReadDflt);
                    mqtt_publish_no(dir_topic, _ReadCrnt);
                }
                else
                {
                    mqtt_publish_no(dir_topic, _ReadDflt);
                }
                return;
            }
            else if (!strcmp(arr_dir[2], ArrVarTopic[_ReadCrnt]))
            {
                // dir_topic[2] = _Read;
                if (is_equal_ok(_element.payload) || is_equal_no(_element.payload))
                    return;
                else if (is_equal_enable(_element.payload))
                {
                    rsdebugInflnF("Загружаем текущие данные");
                    MQTT_pub_allSettings(true); // данные из памяти/ROM
                    mqtt_publish_ok(dir_topic, _ReadCrnt);
                    mqtt_publish_no(dir_topic, _ReadDflt);
                }
                else
                    mqtt_publish_no(dir_topic, _ReadCrnt);
                return;
            }
            else if (!strcmp(arr_dir[2], ArrVarTopic[_Save]))
            {
                // dir_topic[2] = _Save;
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
                            mqtt_publish_ok(dir_topic, _Save);
                        }
                        else
                        {
                            // rsdebugDnflnF("@5");
                            mqtt_publish_no(dir_topic, _Save);
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
                            mqtt_publish_ok(dir_topic, _Save);
                            mqtt_publish_no(dir_topic, _ReadDflt);
                            mqtt_publish_no(dir_topic, _ReadCrnt);
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
            else if (!strcmp(arr_dir[2], ArrVarTopic[_Debug]))
            {
                if (is_equal_ok(_element.payload) || is_equal_no(_element.payload))
                    return;
                else if (!strcmp(_element.payload, "timereset"))
                {
                    MQTT_pub_Info_TimeReset();
                    mqtt_publish_ok(dir_topic, _Debug);
                }
                else if (!strcmp(_element.payload, "reset"))
                {
                    rsdebugWnflnF("Restart ESP");
                    // e_IDDirTopic dir_topic[] = {_main_topic, _Settings, d_empty, d_empty};
                    mqtt_publish(dir_topic, _Debug, "Restarting...");
                    // os_delay_us(500000);
                    ESP.restart(); // Reset
                }
                else
                {
                    mqtt_publish(dir_topic, _Debug, "no");
                }
                return;
            }
            else
                return;
            dir_topic[2] = d_empty;
            dir_topic[3] = d_empty;
            if (NeedSaveSettings.all)
                mqtt_publish(dir_topic, _Save, "NeedSave");
            else
                mqtt_publish_ok(dir_topic, _Save);
        }
    }
}
// else if (!strcmp(payload, "getipntp"))
// {
//     e_IDDirTopic dir_topic[] = {_main_topic, _Settings, _NTP, d_empty};
//     MQTT_pub_Settings_NTP_IP(dir_topic, _IP1, 0);
//     MQTT_pub_Settings_NTP_IP(dir_topic, _IP2, 1);
//     MQTT_pub_Settings_NTP_IP(dir_topic, _IP3, 2);
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

bool Modify_task_tTask(e_IDDirTopic *_dir_topic, const char *_payload, uTask *_task, uint32_t _tMin, uint32_t _tMax, uint32_t *_tTask)
{
    uint16_t tTask = atoi(_payload);
    if ((tTask >= _tMin && tTask <= _tMax) || (tTask == 0 && strlen(_payload) == 1 && _payload[0] == '0')) // в диапазоне, а 0 - число
    {
        if (*_tTask != tTask)
        {
            _task->enable();
            _task->setInterval(tTask);
            *_tTask = tTask;
            return true;
        }
    }
    else
    {
        mqtt_publish(_dir_topic, _T_task, (uint32_t)_task->getInterval());
    }
    return false;
}
