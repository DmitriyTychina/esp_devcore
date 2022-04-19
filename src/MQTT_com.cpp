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

#define def_AP 0xfefe

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

// void MQTT_com_SaveSettings(s_element_Queue_MQTT _element)
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
//         e_IDDirTopic dir_topic[] = {_main_topic, _Settings, _empty, _empty};
//         mqtt_publish(dir_topic, _Save, "NeedSave");
//         return;
//     }
//     else
//         MQTT_pub_Settings_ok(_Save);
//     // debuglog_print_free_memory();
// }

void MQTT_com_Settings(s_element_Queue_MQTT _element)
{
    // EmptyMemorySettingsNTC();
    // debuglog_print_free_memory();
    char *topic = (char *)_element.topic->c_str();
    // _element.payload->toLowerCase();
    char *payload = (char *)_element.payload->c_str();
    uint8_t payload_len = strlen(payload);
    rsdebugInfln("MQTT_com_Settings:%s[%s]", topic, payload);
    // const char ArrVarTopic[_LastElement_e_IDVarTopic][14]
    // const char ArrDirTopic[_LastElement_e_IDDirTopic][10]
    // char arr_dir[10][14] = {"", "", "", "", "", "", "", "", ""};
    char arr_dir[5][16] = {"", "", "", "", ""};
    uint16_t i = 0;
    // do
    // {
    //     char *n = strrchr(topic, '/');
    //     if (n)
    //     {
    //         strcpy(arr_dir[i], n + 1);
    //         strcpy(n, "");
    //     }
    //     else
    //     {
    //         strcpy(arr_dir[i], topic);
    //         strcpy(topic, "");
    //     }
    //     rsdebugDln("dir[%d]: %s", i, arr_dir[i]);
    //     i++;
    // } while (strlen(topic) != 0);
    char *pch = strtok(topic, "/");
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
            e_IDDirTopic dir_topic[] = {_main_topic, _Settings, _empty, _empty};
            // e_IDDirTopic dir_topic_tmp[] = {_main_topic, _Settings, _empty, _empty};
            if (!strcmp(arr_dir[2], ArrDirTopic[_RSdebug]))
            {
                dir_topic[2] = _RSdebug;
                LoadInMemorySettingsSys();
                if (!strcmp(arr_dir[3], ArrVarTopic[_T_task]))
                {
                    if (Modify_task_tTask(dir_topic, payload, &ut_debuglog, 10, 1000, &g_p_sys_settings_ROM->RSDebug_Ttask))
                        NeedSaveSettings.bit.RSDebug = true;
                }
                else if (!strcmp(arr_dir[3], ArrDirTopic[_Sdebug]))
                {
                    dir_topic[3] = _Sdebug;
                    // rsdebugDlnF("***_Sdebug");
                    if (!strcmp(arr_dir[4], ArrVarTopic[_Enable]))
                    {
                        if (strlen(payload) == 0) // публикуем значение по умолчанию если пришла пустая строка
                        {
                            mqtt_publish(dir_topic, _Enable, Debug.isSdebugEnabled());
                        }
                        else if (is_equal_enable(payload))
                        {
                            // rsdebugDlnF("***setSdebugEnabled(true)");
                            if (!Debug.isSdebugEnabled())
                            {
                                Debug.setSdebugEnabled(true);
                                g_p_sys_settings_ROM->RSDebug_SDebug = true;
                                NeedSaveSettings.bit.RSDebug = true;
                                // MQTT_pub_Info_NeedSaveSettings("NeedSave_RSDebug");
                            }
                        }
                        else if (is_equal_disable(payload))
                        {
                            // rsdebugDlnF("***setSdebugEnabled(false)");
                            if (Debug.isSdebugEnabled())
                            {
                                Debug.setSdebugEnabled(false);
                                g_p_sys_settings_ROM->RSDebug_SDebug = false;
                                NeedSaveSettings.bit.RSDebug = true;
                                // MQTT_pub_Info_NeedSaveSettings("NeedSave_RSDebug");
                            }
                        }
                        mqtt_publish(dir_topic, _Enable, Debug.isSdebugEnabled());
                    }
                }
                else if (!strcmp(arr_dir[3], ArrDirTopic[_Rdebug]))
                {
                    dir_topic[3] = _Rdebug;
                    // rsdebugDlnF("***_Rdebug");
                    if (!strcmp(arr_dir[4], ArrVarTopic[_Enable]))
                    {
                        if (strlen(payload) == 0) // публикуем значение по умолчанию если пришла пустая строка
                        {
                            mqtt_publish(dir_topic, _Enable, Debug.isRdebugEnabled());
                        }
                        else if (is_equal_enable(payload))
                        {
                            // rsdebugDlnF("***setRdebugEnabled(true)");
                            if (!Debug.isRdebugEnabled())
                            {
                                // Debug.setRdebugEnabled(true);
                                g_p_sys_settings_ROM->RSDebug_RDebug = true;
                                NeedSaveSettings.bit.RSDebug = true;
                                // MQTT_pub_Info_NeedSaveSettings("NeedSave_RSDebug");
                                init_rdebuglog();
                            }
                            // if (!t_debuglog.isEnabled())
                            // t_debuglog.enable();
                        }
                        else if (is_equal_disable(payload))
                        {
                            // rsdebugDlnF("***setRdebugEnabled(false)");
                            if (Debug.isRdebugEnabled())
                            {
                                // Debug.setRdebugEnabled(false);
                                // Debug.disconnectClient();
                                g_p_sys_settings_ROM->RSDebug_RDebug = false;
                                NeedSaveSettings.bit.RSDebug = true;
                                // MQTT_pub_Info_NeedSaveSettings("NeedSave_RSDebug");
                                // Debug.stop();
                                init_rdebuglog();
                            }
                            // if (t_debuglog.isEnabled())
                            // t_debuglog.disable();
                        }
                        mqtt_publish(dir_topic, _Enable, Debug.isRdebugEnabled());
                    }
                }
            }
            else if (!strcmp(arr_dir[2], ArrDirTopic[_SysMon]))
            {
                dir_topic[2] = _SysMon;
                LoadInMemorySettingsSys();
                if (!strcmp(arr_dir[3], ArrVarTopic[_T_task]))
                {
                    if (Modify_task_tTask(dir_topic, payload, &ut_sysmon, 1000, 3600000, &g_p_sys_settings_ROM->SysMon_Ttask))
                    {
                        NeedSaveSettings.bit.SysMon = true;
                        SysMon_Init();
                    }
                }
            }
            else if (!strcmp(arr_dir[2], ArrDirTopic[_OTA]))
            {
                dir_topic[2] = _OTA;
                LoadInMemorySettingsSys();
                if (!strcmp(arr_dir[3], ArrVarTopic[_T_task]))
                {
                    // if (Modify_task_tTask(dir_topic, payload, &ut_OTA, 1, 5000, &g_p_sys_settings_ROM->OTA_Ttask))
                    //     NeedSaveSettings.bit.OTA = true;
                }
                // EmptyMemorySettingsSys();
            }
            else if (!strcmp(arr_dir[2], ArrDirTopic[_MQTT]))
            {
                dir_topic[2] = _MQTT;
                LoadInMemorySettingsEthernet();
                if (!strcmp(arr_dir[3], ArrVarTopic[_T_task]))
                {
                    if (Modify_task_tTask(dir_topic, payload, &ut_MQTT, 1, 1000, &g_p_ethernet_settings_ROM->MQTT_Ttask))
                        NeedSaveSettings.bit.MQTT = true;
                }
                else if (!strcmp(arr_dir[3], ArrVarTopic[_USER]))
                {
                    if (payload_len == 0)
                        mqtt_publish(dir_topic, _USER, "*****");
                    else if (strcmp(payload, "*****") != 0)
                    {
                        if (strcmp(g_p_ethernet_settings_ROM->MQTT_user, payload) != 0) // не совпадает
                        {
                            strcpy(g_p_ethernet_settings_ROM->MQTT_user, payload);
                            NeedSaveSettings.bit.MQTT = true;
                            // MQTT_pub_Info_NeedSaveSettings("NeedSave_MQTT");
                        }
                    }
                }
                else if (!strcmp(arr_dir[3], ArrVarTopic[_PASS]))
                {
                    if (payload_len == 0)
                        mqtt_publish(dir_topic, _PASS, "*****");
                    else if (strcmp(payload, "*****") != 0)
                    {
                        if (strcmp(g_p_ethernet_settings_ROM->MQTT_pass, payload) != 0) // не совпадает
                        {
                            strcpy(g_p_ethernet_settings_ROM->MQTT_pass, payload);
                            NeedSaveSettings.bit.MQTT = true;
                            // MQTT_pub_Info_NeedSaveSettings("NeedSave_MQTT");
                        }
                    }
                }
                // EmptyMemorySettingsEthernet();
            }
            else if (!strcmp(arr_dir[2], ArrDirTopic[_WIFI]))
            {
                dir_topic[2] = _WIFI;
                LoadInMemorySettingsEthernet();
                if (!strcmp(arr_dir[3], ArrVarTopic[_T_task]))
                {
                    if (Modify_task_tTask(dir_topic, payload, &ut_wifi, 1, 500, &g_p_ethernet_settings_ROM->WiFi_Ttask))
                        NeedSaveSettings.bit.WIFI = true;
                }
                else
                {
                    uint16_t num_AP = def_AP;
                    e_IDDirTopic a_dirs[] = {_AP1, _AP2, _AP3, _AP4, _AP5};
                    for (uint16_t x = 0; x < sizeof(a_dirs) / sizeof(a_dirs[0]); x++)
                    {
                        if (!strcmp(arr_dir[3], ArrDirTopic[a_dirs[x]]))
                        {
                            num_AP = x;
                            break;
                        }
                    }
                    if (num_AP != def_AP)
                    {
                        dir_topic[3] = a_dirs[num_AP];
                        if (!strcmp(arr_dir[4], ArrVarTopic[_SSID]))
                        {
                            if (payload_len == 0)
                            {
                                mqtt_publish(dir_topic, _SSID, g_p_ethernet_settings_ROM->settings_serv[num_AP].SSID);
                                // rsdebugDnfln("payload_len == 0");
                            }
                            else if (strcmp(g_p_ethernet_settings_ROM->settings_serv[num_AP].SSID, payload) != 0) // не совпадает
                            {
                                strcpy(g_p_ethernet_settings_ROM->settings_serv[num_AP].SSID, payload);
                                NeedSaveSettings.bit.WIFI = true;
                                // MQTT_pub_Info_NeedSaveSettings("NeedSave_WiFi");
                                // rsdebugDnfln("_serv[%d].SSID: %s len: %d", num_AP, g_p_ethernet_settings_ROM->settings_serv[num_AP].SSID, strlen(g_p_ethernet_settings_ROM->settings_serv[num_AP].SSID));
                                // rsdebugDnfln("          payload: %s len: %d", payload, payload_len);
                            }
                            // else
                            // {
                            //     mqtt_publish(dir_topic, _SSID, g_p_ethernet_settings_ROM->settings_serv[num_AP].SSID);
                            //     rsdebugDnfln("else");
                            //     rsdebugDnfln("_serv[%d].SSID: %s len: %d", num_AP, g_p_ethernet_settings_ROM->settings_serv[num_AP].SSID, strlen(g_p_ethernet_settings_ROM->settings_serv[num_AP].SSID));
                            //     rsdebugDnfln("          payload: %s len: %d", payload, payload_len);
                            // }
                        }
                        else if (!strcmp(arr_dir[4], ArrVarTopic[_PASS]))
                        {
                            if (payload_len == 0)
                                mqtt_publish(dir_topic, _PASS, "*****");
                            else if (strcmp(payload, "*****") != 0)
                            {
                                if (strcmp(g_p_ethernet_settings_ROM->settings_serv[num_AP].PASS, payload) != 0) // не совпадает
                                {
                                    strcpy(g_p_ethernet_settings_ROM->settings_serv[num_AP].PASS, payload);
                                    NeedSaveSettings.bit.WIFI = true;
                                    // MQTT_pub_Info_NeedSaveSettings("NeedSave_WiFi");
                                }
                                mqtt_publish(dir_topic, _PASS, "*****");
                            }
                        }
                        else if (!strcmp(arr_dir[4], ArrVarTopic[_IP_serv]))
                        {
                            // rsdebugInfln("IP_MQTT_com1:%s[%d]", g_p_ethernet_settings_ROM->settings_serv[num_AP].MQTTip, num_AP);
                            if (payload_len == 0)
                            {
                                mqtt_publish(dir_topic, _IP_serv, g_p_ethernet_settings_ROM->settings_serv[num_AP].MQTTip);
                            }
                            else if (strcmp(g_p_ethernet_settings_ROM->settings_serv[num_AP].MQTTip, payload) != 0) // не совпадает
                            {
                                strcpy(g_p_ethernet_settings_ROM->settings_serv[num_AP].MQTTip, payload);
                                NeedSaveSettings.bit.WIFI = true;
                                // rsdebugInfln("IP_MQTT_com2:%s[%d]", g_p_ethernet_settings_ROM->settings_serv[num_AP].MQTTip, num_AP);
                                // MQTT_pub_Info_NeedSaveSettings("NeedSave_WiFi");
                            }
                            // else
                            // {
                            //     mqtt_publish(dir_topic, _IP_serv, g_p_ethernet_settings_ROM->settings_serv[num_AP].MQTTip);
                            // }
                        }
                        // rsdebugDnfln("_serv[%d].: %s len: %d", num_AP, g_p_ethernet_settings_ROM->settings_serv[num_AP].SSID, strlen(g_p_ethernet_settings_ROM->settings_serv[num_AP].SSID));
                    }
                }
                // EmptyMemorySettingsEthernet();
            }
            else if (!strcmp(arr_dir[2], ArrVarTopic[_Default]))
            {
                // dir_topic[2] = _Default;
                if (is_equal_ok(payload))
                    return;
                else if (is_equal_enable(payload))
                {
                    // NotSaveEmptyMemorySettings();
                    // ROMVerifySettingsElseSaveDefault(true);
                    // all_settings_Default(g_p_all_settings_ROM);
                    MQTT_pub_allSettings(false); // данные по умолчанию
                    mqtt_publish_ok(dir_topic, _Default);
                    // rsdebugDnfln("dir_topic, _Default, ok");
                }
                else
                {
                    mqtt_publish_no(dir_topic, _Default);
                    // rsdebugDnfln("dir_topic_tmp, _Default, no");
                }
            }
            else if (!strcmp(arr_dir[2], ArrVarTopic[_Read]))
            {
                // dir_topic[2] = _Read;
                if (is_equal_ok(payload))
                    return;
                else if (is_equal_enable(payload))
                {
                    MQTT_pub_allSettings();
                    mqtt_publish_ok(dir_topic, _Read);
                }
                else
                    mqtt_publish_no(dir_topic, _Read);
            }
            else if (!strcmp(arr_dir[2], ArrVarTopic[_Save]))
            {
                // dir_topic[2] = _Save;
                if (is_equal_ok(payload))
                    return;
                else if (is_equal_enable(payload))
                {
                    SaveAllSettings();
                    mqtt_publish_ok(dir_topic, _Save);
                }
                else
                    mqtt_publish_no(dir_topic, _Save);
            }
            else if (!strcmp(arr_dir[2], ArrVarTopic[_Debug]))
            {
                if (is_equal_ok(payload))
                    return;
                // else if (!strcmp(payload, ""))
                //     ; // ничего не делаем
                else if (!strcmp(payload, "timereset"))
                {
                    MQTT_pub_Info_TimeReset();
                    mqtt_publish_ok(dir_topic, _Debug);
                }
                // else if (!strcmp(payload, "getipntp"))
                // {
                //     LoadInMemorySettingsNTP();
                //     e_IDDirTopic dir_topic[] = {_main_topic, _Settings, _NTP, _empty};
                //     MQTT_pub_Settings_NTP_IP(dir_topic, _IP1, 0);
                //     MQTT_pub_Settings_NTP_IP(dir_topic, _IP2, 1);
                //     MQTT_pub_Settings_NTP_IP(dir_topic, _IP3, 2);
                //     EmptyMemorySettingsNTP();
                // }
                else if (!strcmp(payload, "reset"))
                {
                    rsdebugWnflnF("Restart ESP");
                    // e_IDDirTopic dir_topic[] = {_main_topic, _Settings, _empty, _empty};
                    mqtt_publish(dir_topic, _Debug, "Restarting...");
                    // os_delay_us(500000);
                    ESP.restart(); // Reset
                }
                else
                {
                    mqtt_publish(dir_topic, _Debug, strcat(payload, "-undefined"));
                }
            }
            dir_topic[2] = _empty;
            dir_topic[3] = _empty;
            if (NeedSaveSettings.value)
                mqtt_publish(dir_topic, _Save, "NeedSave");
            else
                mqtt_publish_ok(dir_topic, _Save);
        }
    }
}

bool Modify_task_tTask(e_IDDirTopic *_dir_topic, const char *_payload, uTask *_task, uint32_t _tMin, uint32_t _tMax, uint32_t *_tTask)
{
    uint16_t tTask = atoi(_payload);
    if ((tTask >= _tMin && tTask <= _tMax) || tTask == 0)
    {
        if (_task->getInterval() != tTask)
        {
            _task->setInterval(tTask);
            // return true;
        }
        if (!_task->isEnabled())
            _task->enable();
    }
    else
    {
        mqtt_publish(_dir_topic, _T_task, (uint32_t)_task->getInterval());
        return false;
    }
    if (*_tTask != tTask)
    {
        *_tTask = tTask;
        return true;
    }
    return false;
}
