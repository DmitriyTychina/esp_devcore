#include <Arduino.h>
// #include <NtpClientLib.h>
#include <EEPROM.h>

#include "my_MQTT.h"
#include "my_EEPROM.h"
#include "my_scheduler.h"
#include "my_debuglog.h"
#include "my_MQTT_pub.h"
#include "my_MQTT_sub.h"

#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
// include
#include "my_door.h"
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

#define not_AP 0xfefe

#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN

// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

#ifdef MQTT_QUEUE
#define size_Queue_MQTT 50 // Максимум 255
// s_element_MQTT *p_element_MQTT;
CirQueue p_Queue_MQTT(size_Queue_MQTT, sizeof(s_element_MQTT));
#else
char last_topic[80];
char last_payload[24];
#endif

const s_SubscribeElement _arr_SubscribeElement[] = {
// если подписаны на топик aaa/bbb/ccc который входит в подписку aaa/bbb/#, то его нужно поместить выше в массиве
#ifdef USER_AREA
    // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
    {{d_main_topic, d_devices, d_door, d_commands}, v_latch, 0, &cb_MQTT_com_Door},
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
    // {{d_main_topic, d_settings, d_empty, d_empty}, _Debug, 0, &cb_MQTT_sub_Settings},
    {{d_main_topic, d_info, d_empty, d_empty}, vc_Read, 0, &cb_MQTT_sub_Info},
    // {{d_main_topic, d_settings, d_empty, d_empty}, _ReadDflt, 0, &cb_MQTT_sub_Settings},
    // {{d_main_topic, d_settings, d_empty, d_empty}, _ReadCrnt, 0, &cb_MQTT_sub_Settings},
    // {{d_main_topic, d_settings, d_empty, d_empty}, _Edit, 0, &cb_MQTT_sub_Settings},
    // {{d_main_topic, d_settings, d_empty, d_empty}, _Save, 0, &cb_MQTT_sub_Settings},
    {{d_main_topic, d_settings, d_empty, d_empty}, v_all, 0, &cb_MQTT_sub_Settings},
};

void onMessageReceived(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
#ifdef MQTT_QUEUE
    char *_topic = new char[strlen(topic)];
    strcpy(_topic, topic);
    char *_payload = new char[len + 1];
    memcpy(_payload, payload, len);
    _payload[len] = 0;
    s_element_MQTT in_element = {_topic, _payload};
    // rsdebugDnfln("1topic[%s]", topic);
    rsdebugDnfln("[%d]MQTT incoming: %s[%s][%d][%d][%d]", p_Queue_MQTT.isCount(), _topic, _payload, len, index, total);
    if (p_Queue_MQTT.isFull())
    {
        rsdebugEnflnF("p_Queue_nCallback is full !!!!");
    }
    else
    {
        if (!p_Queue_MQTT.isEmpty())
        {
            // rsdebugDnflnF("@3");
            s_element_MQTT *_element = (s_element_MQTT *)p_Queue_MQTT.peeklast();
            // rsdebugDnflnF("@31");
            // rsdebugDnfln("[%d]MQTT peeklast: %s[%s]", p_Queue_MQTT->isCount() - 1, _element->topic->c_str(), _element->payload->c_str());
            // rsdebugDnfln("#1[%s][%d][%s][%d]", _element->topic, strlen(_element->topic), in_element.topic, strlen(_topic));
            // rsdebugDnfln("#2[%s][%d][%s][%d]", _element->payload, strlen(_element->payload), in_element.payload, strlen(_payload));
            // rsdebugDnflnF("@32");
            // rsdebugInfln("FreeRAM1: %d", ESP.getFreeHeap());
            if (!strcmp((*_element).topic, _topic) && !strcmp((*_element).payload, _payload))
            {
                rsdebugWnflnF("Duplicate !!!!");
                delete _topic;
                delete _payload;
                // delete in_element;
                return;
            }
            // rsdebugInfln("FreeRAM2: %d", ESP.getFreeHeap());
        }
        // {
        // rsdebugDnflnF("@3");
        // rsdebugInfln("FreeRAM3: %d", ESP.getFreeHeap());
        p_Queue_MQTT.push((uint8_t *)&in_element);
        // rsdebugInfln("FreeRAM4: %d", ESP.getFreeHeap());
        ut_MQTT.forceNextIteration();
    }
#else
    char tmp_payload[len + 1];
    memcpy(tmp_payload, payload, len);
    tmp_payload[len] = 0;
    s_element_MQTT in_element = {topic, tmp_payload};
    rsdebugDnfF("MQTT incoming: ");
    rsdebugDnfln("%s<%s>", topic, tmp_payload);
    if (!strcmp(last_topic, topic) && !strcmp(last_payload, tmp_payload))
    {
        rsdebugWnflnF("Duplicate !!!!");
        return;
    }
    else
    {
        strcpy(last_topic, topic);
        strcpy(last_payload, tmp_payload);
        String topic_in, topic_DB;
        topic_in = String(topic);
        // payload_in = String(tmp_payload);
        // uint8_t i;
        // uint8_t n = sizeof(_arr_SubscribeElement) / sizeof(_arr_SubscribeElement[0]);
        for (uint8_t i = 0; i < (sizeof(_arr_SubscribeElement) / sizeof(_arr_SubscribeElement[0])); i++)
        {
            if (_arr_SubscribeElement[i].IDVarTopic == v_all)
            {
                topic_DB = CreateTopic((e_IDDirTopic *)_arr_SubscribeElement[i].IDDirTopic, v_empty);
                topic_in.remove(topic_DB.length());
                // rsdebugDln("topic_in=%s, topic=%s, topic_DB=%s", topic_in.c_str(), topic.c_str(), topic_DB.c_str());
                // rsdebugDln("i=%d, s1=%s, s2=%s", i, topic_in.c_str(), topic_DB.c_str());
            }
            else
            {
                topic_DB = CreateTopic((e_IDDirTopic *)_arr_SubscribeElement[i].IDDirTopic, _arr_SubscribeElement[i].IDVarTopic);
            }
            if (topic_in.length() == topic_DB.length())
                if (topic_in == topic_DB)
                {
                    // rsdebugDlnF("==:break");
                    _arr_SubscribeElement[i].Callback(in_element);
                    break;
                }
        }
    }
#endif
}

void MQTT_sub_All()
{
    for (uint8 i = 0; i < (sizeof(_arr_SubscribeElement) / sizeof(_arr_SubscribeElement[0])); i++)
    {
        mqtt_subscribe(_arr_SubscribeElement[i]);
    }
}

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

void cb_MQTT_sub_Info(s_element_MQTT _element)
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

void cb_MQTT_sub_Settings(s_element_MQTT _element)
{
    uint8_t payload_len = strlen(_element.payload);
    rsdebugInfF("MQTT in->Settings:");
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
#if defined(EEPROM_C)
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_debuglog, 10, 1000, &g_p_sys_settings_ROM->RSDebug_Ttask))
                        NeedSaveSettings.sys.bit.RSDebug_T_task = true;
#elif defined(EEPROM_CPP)
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_debuglog, 10, 1000, &ram_data.p_SYS_settings()->RSDebug_Ttask))
                        ram_data.unsaved = true;
#endif
                    else
                        return;
                }
                else if (!strcmp(arr_dirs[3], ArrDirTopic[d_s_debug]))
                {
                    dirs_topic[3] = d_s_debug;
                    // rsdebugDlnF("***d_s_debug");
                    if (!strcmp(arr_dirs[4], ArrVarTopic[v_enable]))
                    {
#if defined(EEPROM_C)
                        uint8_t tmp_result = Modify_bool(dirs_topic, v_enable, _element.payload, &g_p_sys_settings_ROM->RSDebug_SDebug, Debug.isSdebugEnabled()); // в минутах: от 1мин до 24ч
#elif defined(EEPROM_CPP)
                        uint8_t tmp_result = Modify_bool(dirs_topic, v_enable, _element.payload, &ram_data.p_SYS_settings()->RSDebug_SDebug, Debug.isSdebugEnabled()); // в минутах: от 1мин до 24ч
#endif
                        if (!tmp_result) // 0 - нет изменеий
                            return;
                        if (tmp_result & 1) // 1 или 3 - payload != mem_data
                        {
#if defined(EEPROM_C)
                            NeedSaveSettings.sys.bit.RSDebug_SD_en = true;
#elif defined(EEPROM_CPP)
                            ram_data.unsaved = true;
#endif
                        }
                        if (tmp_result & 2) // 2 или 3 - payload != real_data
                        {
#if defined(EEPROM_C)
                            Debug.setSdebugEnabled(g_p_sys_settings_ROM->RSDebug_SDebug);
#elif defined(EEPROM_CPP)
                            Debug.setSdebugEnabled(ram_data.p_SYS_settings()->RSDebug_SDebug);
#endif
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
#if defined(EEPROM_C)
                        uint8_t tmp_result = Modify_bool(dirs_topic, v_enable, _element.payload, &g_p_sys_settings_ROM->RSDebug_RDebug, Debug.isRdebugEnabled()); // в минутах: от 1мин до 24ч
#elif defined(EEPROM_CPP)
                        uint8_t tmp_result = Modify_bool(dirs_topic, v_enable, _element.payload, &ram_data.p_SYS_settings()->RSDebug_RDebug, Debug.isRdebugEnabled()); // в минутах: от 1мин до 24ч
#endif
                        if (!tmp_result) // 0 - нет изменеий
                            return;
                        if (tmp_result & 1) // 1 или 3 - payload != mem_data
                        {
#if defined(EEPROM_C)
                            NeedSaveSettings.sys.bit.RSDebug_RD_en = true;
#elif defined(EEPROM_CPP)
                            ram_data.unsaved = true;
#endif
                        }
                        if (tmp_result & 2) // 2 или 3 - payload != real_data
                        {
#if defined(EEPROM_C)
                            Debug.setRdebugEnabled(g_p_sys_settings_ROM->RSDebug_RDebug);
#elif defined(EEPROM_CPP)
                            Debug.setRdebugEnabled(ram_data.p_SYS_settings()->RSDebug_RDebug);
#endif
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
#if defined(EEPROM_C)
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_sysmon, 1000, 3600000, &g_p_sys_settings_ROM->SysMon_Ttask))
                        NeedSaveSettings.sys.bit.SysMon_T_task = true;
#elif defined(EEPROM_CPP)
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_sysmon, 1000, 3600000, &ram_data.p_SYS_settings()->SysMon_Ttask))
                        ram_data.unsaved = true;
#endif
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
#if defined(EEPROM_C)
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_OTA, 1, 5000, &g_p_sys_settings_ROM->OTA_Ttask))
                        NeedSaveSettings.sys.bit.OTA_T_task = true;
#elif defined(EEPROM_CPP)
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_OTA, 1, 5000, &ram_data.p_SYS_settings()->OTA_Ttask))
                        ram_data.unsaved = true;
#endif
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
#if defined(EEPROM_C)
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_MQTT, 1, 1000, &g_p_ethernet_settings_ROM->MQTT_Ttask))
                        NeedSaveSettings.bit.MQTT = true;
#elif defined(EEPROM_CPP)
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_MQTT, 1, 1000, &ram_data.p_NET_settings()->MQTT_Ttask))
                        ram_data.unsaved = true;
#endif
                    else
                        return;
                }
                else if (!strcmp(arr_dirs[3], ArrVarTopic[v_user]))
                {
#if defined(EEPROM_C)
                    if (Modify_pass(dirs_topic, v_user, _element.payload, (char *)&g_p_ethernet_settings_ROM->MQTT_user))
                        NeedSaveSettings.bit.MQTT = true;
#elif defined(EEPROM_CPP)
                    if (Modify_pass(dirs_topic, v_user, _element.payload, (char *)&ram_data.p_NET_settings()->MQTT_user))
                        ram_data.unsaved = true;
#endif
                    else
                        return;
                }
                else if (!strcmp(arr_dirs[3], ArrVarTopic[v_pass]))
                {
#if defined(EEPROM_C)
                    if (Modify_pass(dirs_topic, v_pass, _element.payload, (char *)&g_p_ethernet_settings_ROM->MQTT_pass))
                        NeedSaveSettings.bit.MQTT = true;
#elif defined(EEPROM_CPP)
                    if (Modify_pass(dirs_topic, v_pass, _element.payload, (char *)&ram_data.p_NET_settings()->MQTT_pass))
                        ram_data.unsaved = true;
#endif
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
#if defined(EEPROM_C)
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_wifi, 1, 500, &g_p_ethernet_settings_ROM->WiFi_Ttask))
                        NeedSaveSettings.bit.WIFI = true;
#elif defined(EEPROM_CPP)
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_wifi, 1, 500, &ram_data.p_NET_settings()->WiFi_Ttask))
                        ram_data.unsaved = true;
#endif
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
#if defined(EEPROM_C)
                            if (Modify_string(dirs_topic, v_ssid, _element.payload, (char *)&g_p_ethernet_settings_ROM->settings_serv[num_AP].SSID))
                                NeedSaveSettings.bit.WIFI = true;
#elif defined(EEPROM_CPP)
                            if (Modify_string(dirs_topic, v_ssid, _element.payload, (char *)&ram_data.p_NET_settings()->settings_serv[num_AP].SSID))
                                ram_data.unsaved = true;
#endif
                            else
                                return;
                        }
                        else if (!strcmp(arr_dirs[4], ArrVarTopic[v_pass]))
                        {
#if defined(EEPROM_C)
                            if (Modify_pass(dirs_topic, v_pass, _element.payload, (char *)&g_p_ethernet_settings_ROM->settings_serv[num_AP].PASS))
                                NeedSaveSettings.bit.WIFI = true;
#elif defined(EEPROM_CPP)
                            if (Modify_pass(dirs_topic, v_pass, _element.payload, (char *)&ram_data.p_NET_settings()->settings_serv[num_AP].PASS))
                                ram_data.unsaved = true;
#endif
                            else
                                return;
                        }
                        else if (!strcmp(arr_dirs[4], ArrVarTopic[v_ip_serv]))
                        {
#if defined(EEPROM_C)
                            if (Modify_string(dirs_topic, v_ip_serv, _element.payload, (char *)&g_p_ethernet_settings_ROM->settings_serv[num_AP].MQTTip))
                                NeedSaveSettings.bit.WIFI = true;
#elif defined(EEPROM_CPP)
                            if (Modify_string(dirs_topic, v_ip_serv, _element.payload, (char *)&ram_data.p_NET_settings()->settings_serv[num_AP].MQTTip))
                                ram_data.unsaved = true;
#endif
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
#if defined(EEPROM_C)
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_NTP, 100, 5000, &g_p_NTP_settings_ROM->Ttask))
                        NeedSaveSettings.bit.NTP = true;
#elif defined(EEPROM_CPP)
                    if (Modify_task_tTask(dirs_topic, _element.payload, &ut_NTP, 100, 5000, &ram_data.p_NTP_settings()->Ttask))
                        ram_data.unsaved = true;
#endif
                    else
                        return;
                }
                else if (!strcmp(arr_dirs[3], ArrVarTopic[v_t_sync]))
                {
                    // rsdebugDnfln("@1 d_ntp");
#if defined(EEPROM_C)
                    uint8_t tmp_result = Modify_num(dirs_topic, v_t_sync, _element.payload, 1, 1440, &g_p_NTP_settings_ROM->T_syncNTP, NTP.getInterval() / 60); // в минутах: от 1мин до 24ч
#elif defined(EEPROM_CPP)
                    uint8_t tmp_result = Modify_num(dirs_topic, v_t_sync, _element.payload, 1, 1440, &ram_data.p_NTP_settings()->T_syncNTP, NTP.getInterval() / 60); // в минутах: от 1мин до 24ч
#endif
                    if (!tmp_result) // 0 - нет изменеий
                        return;
                    if (tmp_result & 1) // 1 или 3 - payload != mem_data
                    {
#if defined(EEPROM_C)
                        NeedSaveSettings.bit.NTP = true;
#elif defined(EEPROM_CPP)
                        ram_data.unsaved = true;
#endif
                    }
                    if (tmp_result & 2) // 2 или 3 - payload != real_data
                    {
#if defined(EEPROM_C)
                        NTP.setInterval(g_p_NTP_settings_ROM->T_syncNTP * 60);
#elif defined(EEPROM_CPP)
                        NTP.setInterval(ram_data.p_NTP_settings()->T_syncNTP * 60);
#endif
                    }
                    // rsdebugDnfln("@2 d_ntp");
                }
                else if (!strcmp(arr_dirs[3], ArrVarTopic[v_timezone]))
                {
#if defined(EEPROM_C)
                    uint8_t tmp_result = Modify_num(dirs_topic, v_timezone, _element.payload, -23, 23, &g_p_NTP_settings_ROM->timezone, NTP.getTimeZone()); // в минутах: от 1мин до 24ч
#elif defined(EEPROM_CPP)
                    uint8_t tmp_result = Modify_num(dirs_topic, v_timezone, _element.payload, -23, 23, &ram_data.p_NTP_settings()->timezone, NTP.getTimeZone()); // в минутах: от 1мин до 24ч
#endif
                    if (!tmp_result) // 0 - нет изменеий
                        return;
                    if (tmp_result & 1) // 1 или 3 - payload != mem_data
                    {
#if defined(EEPROM_C)
                        NeedSaveSettings.bit.NTP = true;
#elif defined(EEPROM_CPP)
                        ram_data.unsaved = true;
#endif
                    }
                    if (tmp_result & 2) // 2 или 3 - payload != real_data
                    {
#if defined(EEPROM_C)
                        NTP.setTimeZone(g_p_NTP_settings_ROM->timezone);
#elif defined(EEPROM_CPP)
                        NTP.setTimeZone(ram_data.p_NTP_settings()->timezone);
#endif
                    }
                }
                else if (!strcmp(arr_dirs[3], ArrVarTopic[v_ip1]))
                {
#if defined(EEPROM_C)
                    if (Modify_string(dirs_topic, v_ip1, _element.payload, (char *)&g_p_NTP_settings_ROM->serversNTP[0]))
                        NeedSaveSettings.bit.NTP = true;
#elif defined(EEPROM_CPP)
                    if (Modify_string(dirs_topic, v_ip1, _element.payload, (char *)&ram_data.p_NTP_settings()->serversNTP[0]))
                        ram_data.unsaved = true;
#endif
                    else
                        return;
                }
                else if (!strcmp(arr_dirs[3], ArrVarTopic[v_ip2]))
                {
#if defined(EEPROM_C)
                    if (Modify_string(dirs_topic, v_ip2, _element.payload, (char *)&g_p_NTP_settings_ROM->serversNTP[1]))
                        NeedSaveSettings.bit.NTP = true;
#elif defined(EEPROM_CPP)
                    if (Modify_string(dirs_topic, v_ip2, _element.payload, (char *)&ram_data.p_NTP_settings()->serversNTP[1]))
                        ram_data.unsaved = true;
#endif
                    else
                        return;
                }
                else if (!strcmp(arr_dirs[3], ArrVarTopic[v_ip3]))
                {
#if defined(EEPROM_C)
                    if (Modify_string(dirs_topic, v_ip3, _element.payload, (char *)&g_p_NTP_settings_ROM->serversNTP[2]))
                        NeedSaveSettings.bit.NTP = true;
#elif defined(EEPROM_CPP)
                    if (Modify_string(dirs_topic, v_ip3, _element.payload, (char *)&ram_data.p_NTP_settings()->serversNTP[2]))
                        ram_data.unsaved = true;
#endif
                    else
                        return;
                }
                // rsdebugDnfln("@3 d_ntp");
                init_NTP_with_WiFi();
#if defined(EEPROM_C)
                if (!NeedSaveSettings.bit.NTP)
                    return;
#elif defined(EEPROM_CPP)
                if (!ram_data.unsaved)
                    return;
#endif
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
#if defined(EEPROM_C)
                if (NeedSaveSettings.all)
#elif defined(EEPROM_CPP)
                if (ram_data.unsaved)
#endif
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
#if defined(EEPROM_C)
                        if (NeedSaveSettings.all)
#elif defined(EEPROM_CPP)
                        if (ram_data.unsaved)
#endif
                        {
                            // rsdebugDnflnF("@7");
                            rsdebugInflnF("Не сохраняем, загружаем данные из ROM");
#if defined(EEPROM_C)
                            NotSaveEmptyMemorySettings(); // не сохраняем текущие
#elif defined(EEPROM_CPP)
                            ram_data.unsaved = false; // не сохраняем текущие
#endif
                            MQTT_pub_allSettings(true); // публикуем данные из ROM
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
                    mqtt_publish_no(dirs_topic, vc_Debug);
                }
                return;
            }
            else
                return;
            dirs_topic[2] = d_empty;
            dirs_topic[3] = d_empty;
            // rsdebugDnfln("@ NeedSaveSettings.all=%d", NeedSaveSettings.all);
#if defined(EEPROM_C)
            if (NeedSaveSettings.all)
#elif defined(EEPROM_CPP)
            if (ram_data.unsaved)
#endif
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

void mqtt_subscribe(s_SubscribeElement _sub_element)
{
    String topic = CreateTopic(_sub_element.IDDirTopic, _sub_element.IDVarTopic);
    rsdebugDnfln("sub %s", topic.c_str());
    client.subscribe(topic.c_str(), _sub_element.mqttQOS);
}

void mqtt_unsubscribe(s_SubscribeElement _sub_element)
{
}

bool is_equal_ok(const char *char_str)
{
    return (!strcmp(char_str, "ok"));
}

bool is_equal_no(const char *char_str)
{
    return (!strcmp(char_str, "no"));
}

bool is_equal_enable(const char *char_str)
{
    return (!strcmp(char_str, "enable") ||
            !strcmp(char_str, "true") ||
            !strcmp(char_str, "open") ||
            !strcmp(char_str, "yes") ||
            !strcmp(char_str, "1") ||
            !strcmp(char_str, "on"));
}

bool is_equal_disable(const char *char_str)
{
    return (!strcmp(char_str, "disable") ||
            !strcmp(char_str, "false") ||
            !strcmp(char_str, "close") ||
            !strcmp(char_str, "no") ||
            !strcmp(char_str, "0") ||
            !strcmp(char_str, "off"));
}

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
