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
    {{d_main_topic, d_devices, d_door, d_commands, d_empty, d_empty}, v_latch, 0, &cb_MQTT_com_Door},
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
    {{d_main_topic, d_system, d_chip_esp, d_empty, d_empty, d_empty}, vc_Debug, 0, &cb_MQTT_com_Debug},
    {{d_main_topic, d_system, d_empty, d_empty, d_empty, d_empty}, vc_ReadAll, 0, &cb_MQTT_com_Info},
    {{d_main_topic, d_system, d_rom, d_empty, d_empty, d_empty}, vc_ReadDef, 0, &cb_MQTT_com_ReadDef},
    {{d_main_topic, d_system, d_rom, d_empty, d_empty, d_empty}, vc_ReadROM, 0, &cb_MQTT_com_ReadROM},
    {{d_main_topic, d_system, d_rom, d_empty, d_empty, d_empty}, vc_ReadRAM, 0, &cb_MQTT_com_ReadRAM},
    {{d_main_topic, d_system, d_rom, d_empty, d_empty, d_empty}, vc_Save, 0, &cb_MQTT_com_Save},
    {{d_main_topic, d_net, d_wifi, d_settings, d_empty, d_empty}, v_all, 0, &cb_MQTT_com_set_WiFi},
    {{d_main_topic, d_net, d_mqtt, d_settings, d_empty, d_empty}, v_all, 0, &cb_MQTT_com_set_MQTT},
#ifdef CORE_NTP
    {{d_main_topic, d_net, d_ntp, d_settings, d_empty, d_empty}, v_all, 0, &cb_MQTT_com_set_NTP},
#endif
    {{d_main_topic, d_net, d_ota, d_settings, d_empty, d_empty}, v_all, 0, &cb_MQTT_com_set_OTA},
    {{d_main_topic, d_net, d_rs_debug, d_settings, d_empty, d_empty}, v_all, 0, &cb_MQTT_com_set_RSDebug},
    {{d_main_topic, d_system, d_sysmon, d_settings, d_empty, d_empty}, v_all, 0, &cb_MQTT_com_set_SysMon},
    {{d_main_topic, d_system, d_rom, d_settings, d_empty, d_empty}, v_all, 0, &cb_MQTT_com_set_ROM},
    // {{d_main_topic, d_settings, d_empty, d_empty, d_empty, d_empty}, _Edit, 0, &cb_MQTT_sub_Settings},
    // {{d_main_topic, d_settings, d_empty, d_empty, d_empty, d_empty}, _Save, 0, &cb_MQTT_sub_Settings},
    // {{d_main_topic, d_settings, d_empty, d_empty, d_empty, d_empty}, v_all, 0, &cb_MQTT_sub_Settings},
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
    rsdebugInfF("MQTT incoming: ");
    rsdebugInfln("%s<%s>", topic, tmp_payload);
    if (!strcmp(last_topic, topic) && !strcmp(last_payload, tmp_payload))
    {
        rsdebugInflnF("Duplicate !!!!");
        return;
    }
    else
    {
        strcpy(last_topic, topic);
        strcpy(last_payload, tmp_payload);
        String topic_in, topic_DB;
        // payload_in = String(tmp_payload);
        // uint8_t i;
        uint32_t n = sizeof(_arr_SubscribeElement[0].IDDirTopic) / sizeof(_arr_SubscribeElement[0].IDDirTopic[0]);
        for (uint32_t i = 0; i < (sizeof(_arr_SubscribeElement) / sizeof(_arr_SubscribeElement[0])); i++)
        {
            topic_in = String(topic);
            o_IDDirTopic tmp_o = {(e_IDDirTopic *)_arr_SubscribeElement[i].IDDirTopic, n};
            if (_arr_SubscribeElement[i].IDVarTopic == v_all)
            {
                topic_DB = CreateTopic(&tmp_o, v_empty);
                topic_in.remove(topic_DB.length());
            }
            else
            {
                topic_DB = CreateTopic(&tmp_o, _arr_SubscribeElement[i].IDVarTopic);
            }
            // rsdebugDnfln("topic_in=%s, topic=%s, topic_DB=%s", topic_in.c_str(), topic, topic_DB.c_str());
            // rsdebugDnfln("i=%d, s1=%s, s2=%s", i, topic_in.c_str(), topic_DB.c_str());
            if (topic_in.length() == topic_DB.length())
                if (topic_in == topic_DB)
                {
                    // rsdebugDlnF("==:()");
                    _arr_SubscribeElement[i].Callback(in_element);
                    break;
                }
        }
    }
#endif
}

void MQTT_sub_All()
{
    for (uint8_t i = 0; i < (sizeof(_arr_SubscribeElement) / sizeof(_arr_SubscribeElement[0])); i++)
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

void my_strtok(s_element_MQTT _element, char (*_arr_dirs)[MQTT_MAX_SYMBOLS])
{
    uint8_t payload_len = strlen(_element.payload);
    // rsdebugDnfF("MQTT in->");
    // rsdebugDnfln("%s<%s$%d>", _element.topic, _element.payload, payload_len);
    uint16_t i = 0;
    char *pch = strtok(_element.topic, "/");
    while (pch != NULL)
    {
        strcpy(_arr_dirs[i], pch);
        // rsdebugDln("dir[%d]: %s", i, arr_dirs[i]);
        i++;
        pch = strtok(NULL, "/");
    }
}

// bool is_equal_topic_var(char (*_arr_dirs)[MQTT_MAX_SYMBOLS], e_IDDirTopic *IDDirTopic, e_IDVarTopic IDVarTopic)
// {
//     if (!strcmp(arr_dirs[0], ArrDirTopic[d_main_topic]))
//     {
//         if (!strcmp(arr_dirs[1], ArrDirTopic[d_info]))
//         {
//             if (!strcmp(arr_dirs[2], ArrVarTopic[vc_ReadROM]))
//             {
//             }
//         }
//     }
// }

void cb_MQTT_com_Debug(s_element_MQTT _element)
{
    // e_IDDirTopic dirs_topic[] = {d_main_topic, d_system, d_chip_esp};
    // o_IDDirTopic o_dirs_topic = {dirs_topic, sizeof(dirs_topic) / sizeof(dirs_topic[0])};
    if (is_equal_ok(_element.payload) || is_equal_no(_element.payload))
        return;
    // else if (!strcmp(_element.payload, "timereset"))
    // {
    //     MQTT_pub_Info_TimeReset();
    //     mqtt_publish_ok(dirs_topic, vc_Debug);
    // }
    else if (!strcmp(_element.payload, "reset"))
    {
        rsdebugWnflnF("Restart ESP");
        // mqtt_publish(&o_dirs_topic, vc_Debug, "Restarting...");
        mqtt_publish(_element.topic, "Restarting...");
        // os_delay_us(500000);
        ESP.restart(); // Reset
    }
    else
    {
        // // mqtt_publish_no(dirs_topic, vc_Debug);
        // const char *tmp = strcat(_element.payload, "-ok");
        // mqtt_publish(_element.topic, tmp);
        mqtt_publish_ok(_element.topic);
    }
}

void cb_MQTT_com_Info(s_element_MQTT _element)
{
    if (is_equal_ok(_element.payload) || is_equal_no(_element.payload))
    {
    }
    else if (is_equal_enable(_element.payload))
    {
        MQTT_pub_allInfo(false);
        mqtt_publish_ok(_element.topic);
    }
    else if (!strcmp(_element.payload, "all"))
    {
        MQTT_pub_allInfo(true);
        mqtt_publish_ok(_element.topic);
    }
    else // if (is_equal_disable(_element.payload))
    {
        mqtt_publish_no(_element.topic);
    }
}

void cb_MQTT_com_ReadDef(s_element_MQTT _element)
{
    if (is_equal_ok(_element.payload) || is_equal_no(_element.payload))
    {
    }
    else if (is_equal_enable(_element.payload))
    {
        ram_data.unsaved = false;
        MQTT_pub_allSettings(from_def);
        mqtt_publish_ok(_element.topic);
    }
    else // if (is_equal_disable(_element.payload))
    {
        mqtt_publish_no(_element.topic);
    }
    mqtt_publish_save_settings();
}

void cb_MQTT_com_ReadROM(s_element_MQTT _element)
{
    if (is_equal_ok(_element.payload) || is_equal_no(_element.payload))
    {
    }
    else if (is_equal_enable(_element.payload))
    {
        ram_data.unsaved = false;
        MQTT_pub_allSettings(from_rom);
        mqtt_publish_ok(_element.topic);
    }
    else // if (is_equal_disable(_element.payload))
    {
        mqtt_publish_no(_element.topic);
    }
    mqtt_publish_save_settings();
}

void cb_MQTT_com_ReadRAM(s_element_MQTT _element)
{
    if (is_equal_ok(_element.payload) || is_equal_no(_element.payload))
    {
    }
    else if (is_equal_enable(_element.payload))
    {
        ram_data.unsaved = false;
        MQTT_pub_allSettings(from_ram);
        mqtt_publish_ok(_element.topic);
    }
    else // if (is_equal_disable(_element.payload))
    {
        mqtt_publish_no(_element.topic);
    }
    mqtt_publish_save_settings();
}

void cb_MQTT_com_Save(s_element_MQTT _element)
{
    if (is_equal_ok(_element.payload))
    {
    }
    else if (is_equal_enable(_element.payload))
    {
        SaveAllSettings();
        mqtt_publish_ok(_element.topic);
    }
    else if (is_equal_disable(_element.payload))
    {
        def_data.is_all_del(true);
        ram_data.is_all_del(true);
        rom_data.is_all_del(true);
        ram_data.unsaved = false;
        mqtt_publish_ok(_element.topic);
    }
}

void cb_MQTT_com_set_WiFi(s_element_MQTT _element)
{
    char arr_dirs[MQTT_MAX_LEVEL][MQTT_MAX_SYMBOLS]; // [уровней][символов]
    my_strtok(_element, &arr_dirs[0]);
    e_IDDirTopic dirs_topic[] = {d_main_topic, d_net, d_wifi, d_settings};
    o_IDDirTopic o_dirs_topic = {dirs_topic, sizeof(dirs_topic) / sizeof(dirs_topic[0])};
    if (!strcmp(arr_dirs[4], ArrVarTopic[v_t_task]))
    {
#if defined(EEPROM_C)
        if (Modify_task_tTask(&o_dirs_topic, _element.payload, &ut_wifi, 1, 500, &g_p_ethernet_settings_ROM->WiFi_Ttask))
            NeedSaveSettings.bit.WIFI = true;
#elif defined(EEPROM_CPP)
        if (Modify_task_tTask(&o_dirs_topic, _element.payload, &ut_wifi, 1, 500, &ram_data.p_NET_settings()->WiFi_Ttask))
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
            if (!strcmp(arr_dirs[4], ArrDirTopic[a_dirs[x]]))
            {
                num_AP = x;
                break;
            }
        }
        if (num_AP != not_AP)
        {
            o_dirs_topic._IDDirTopic[4] = a_dirs[num_AP];
            if (!strcmp(arr_dirs[5], ArrVarTopic[v_ssid]))
            {
#if defined(EEPROM_C)
                if (Modify_string(dirs_topic, v_ssid, _element.payload, (char *)&g_p_ethernet_settings_ROM->settings_serv[num_AP].SSID))
                    NeedSaveSettings.bit.WIFI = true;
#elif defined(EEPROM_CPP)
                if (Modify_string(&o_dirs_topic, v_ssid, _element.payload, (char *)&ram_data.p_NET_settings()->settings_serv[num_AP].SSID))
                    ram_data.unsaved = true;
#endif
                else
                    return;
            }
            else if (!strcmp(arr_dirs[5], ArrVarTopic[v_pass]))
            {
#if defined(EEPROM_C)
                if (Modify_pass(dirs_topic, v_pass, _element.payload, (char *)&g_p_ethernet_settings_ROM->settings_serv[num_AP].PASS))
                    NeedSaveSettings.bit.WIFI = true;
#elif defined(EEPROM_CPP)
                if (Modify_pass(&o_dirs_topic, v_pass, _element.payload, (char *)&ram_data.p_NET_settings()->settings_serv[num_AP].PASS))
                    ram_data.unsaved = true;
#endif
                else
                    return;
            }
            else if (!strcmp(arr_dirs[5], ArrVarTopic[v_server]))
            {
#if defined(EEPROM_C)
                if (Modify_string(dirs_topic, v_server, _element.payload, (char *)&g_p_ethernet_settings_ROM->settings_serv[num_AP].MQTTip))
                    NeedSaveSettings.bit.WIFI = true;
#elif defined(EEPROM_CPP)
                if (Modify_string(&o_dirs_topic, v_server, _element.payload, (char *)&ram_data.p_NET_settings()->settings_serv[num_AP].MQTTip))
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
    mqtt_publish_save_settings();
}

void cb_MQTT_com_set_MQTT(s_element_MQTT _element)
{
    char arr_dirs[MQTT_MAX_LEVEL][MQTT_MAX_SYMBOLS]; // [уровней][символов]
    my_strtok(_element, &arr_dirs[0]);
    e_IDDirTopic dirs_topic[] = {d_main_topic, d_net, d_mqtt, d_settings};
    o_IDDirTopic o_dirs_topic = {dirs_topic, sizeof(dirs_topic) / sizeof(dirs_topic[0])};
    if (!strcmp(arr_dirs[4], ArrVarTopic[v_t_task]))
    {
#if defined(EEPROM_C)
        if (Modify_task_tTask(dirs_topic, _element.payload, &ut_MQTT, 1, 1000, &g_p_ethernet_settings_ROM->MQTT_Ttask))
            NeedSaveSettings.bit.MQTT = true;
#elif defined(EEPROM_CPP)
        if (Modify_task_tTask(&o_dirs_topic, _element.payload, &ut_MQTT, 1, 1000, &ram_data.p_NET_settings()->MQTT_Ttask))
            ram_data.unsaved = true;
#endif
        else
            return;
    }
    else if (!strcmp(arr_dirs[4], ArrVarTopic[v_user]))
    {
#if defined(EEPROM_C)
        if (Modify_pass(dirs_topic, v_user, _element.payload, (char *)&g_p_ethernet_settings_ROM->MQTT_user))
            NeedSaveSettings.bit.MQTT = true;
#elif defined(EEPROM_CPP)
        if (Modify_pass(&o_dirs_topic, v_user, _element.payload, (char *)&ram_data.p_NET_settings()->MQTT_user))
            ram_data.unsaved = true;
#endif
        else
            return;
    }
    else if (!strcmp(arr_dirs[4], ArrVarTopic[v_pass]))
    {
#if defined(EEPROM_C)
        if (Modify_pass(dirs_topic, v_pass, _element.payload, (char *)&g_p_ethernet_settings_ROM->MQTT_pass))
            NeedSaveSettings.bit.MQTT = true;
#elif defined(EEPROM_CPP)
        if (Modify_pass(&o_dirs_topic, v_pass, _element.payload, (char *)&ram_data.p_NET_settings()->MQTT_pass))
            ram_data.unsaved = true;
#endif
        else
            return;
    }
    else
        return;
    mqtt_publish_save_settings();
}

#ifdef CORE_NTP
void cb_MQTT_com_set_NTP(s_element_MQTT _element)
{
    char arr_dirs[MQTT_MAX_LEVEL][MQTT_MAX_SYMBOLS]; // [уровней][символов]
    my_strtok(_element, &arr_dirs[0]);
    e_IDDirTopic dirs_topic[] = {d_main_topic, d_net, d_ntp, d_settings};
    o_IDDirTopic o_dirs_topic = {dirs_topic, sizeof(dirs_topic) / sizeof(dirs_topic[0])};
    if (!strcmp(arr_dirs[4], ArrVarTopic[v_t_task]))
    {
#if defined(EEPROM_C)
        if (Modify_task_tTask(&o_dirs_topic, _element.payload, &ut_NTP, 100, 5000, &g_p_NTP_settings_ROM->Ttask))
            NeedSaveSettings.bit.NTP = true;
#elif defined(EEPROM_CPP)
        if (Modify_task_tTask(&o_dirs_topic, _element.payload, &ut_NTP, 100, 5000, &ram_data.p_NTP_settings()->Ttask))
            ram_data.unsaved = true;
#endif
        else
            return;
    }
    else if (!strcmp(arr_dirs[4], ArrVarTopic[v_t_sync]))
    {
        // rsdebugDnfln("@1 d_ntp");
#if defined(EEPROM_C)
        uint8_t tmp_result = Modify_num(&o_dirs_topic, v_t_sync, _element.payload, 1, 1440, &g_p_NTP_settings_ROM->PeriodSyncNTP, NTP.getInterval() / 60); // в минутах: от 1мин до 24ч
#elif defined(EEPROM_CPP)
        uint8_t tmp_result = Modify_num(&o_dirs_topic, v_t_sync, _element.payload, 1, 1440, &ram_data.p_NTP_settings()->PeriodSyncNTP, NTP.getInterval() / 60); // в минутах: от 1мин до 24ч
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
            NTP.setInterval(g_p_NTP_settings_ROM->PeriodSyncNTP * 60);
#elif defined(EEPROM_CPP)
            NTP.setInterval(ram_data.p_NTP_settings()->PeriodSyncNTP * 60);
#endif
        }
        // rsdebugDnfln("@2 d_ntp");
    }
    else if (!strcmp(arr_dirs[4], ArrVarTopic[v_timezone]))
    {
#if defined(EEPROM_C)
        uint8_t tmp_result = Modify_num(&o_dirs_topic, v_timezone, _element.payload, -23, 23, &g_p_NTP_settings_ROM->timezone, NTP.getTimeZone()); // в минутах: от 1мин до 24ч
#elif defined(EEPROM_CPP)
        uint8_t tmp_result = Modify_num(&o_dirs_topic, v_timezone, _element.payload, -23, 23, &ram_data.p_NTP_settings()->timezone, NTP.getTimeZone()); // в минутах: от 1мин до 24ч
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
    else if (!strcmp(arr_dirs[4], ArrVarTopic[v_ip1]))
    {
#if defined(EEPROM_C)
        if (Modify_string(&o_dirs_topic, v_ip1, _element.payload, (char *)&g_p_NTP_settings_ROM->serversNTP[0]))
            NeedSaveSettings.bit.NTP = true;
#elif defined(EEPROM_CPP)
        if (Modify_string(&o_dirs_topic, v_ip1, _element.payload, (char *)&ram_data.p_NTP_settings()->serversNTP[0]))
            ram_data.unsaved = true;
#endif
        else
            return;
    }
    else if (!strcmp(arr_dirs[4], ArrVarTopic[v_ip2]))
    {
#if defined(EEPROM_C)
        if (Modify_string(&o_dirs_topic, v_ip2, _element.payload, (char *)&g_p_NTP_settings_ROM->serversNTP[1]))
            NeedSaveSettings.bit.NTP = true;
#elif defined(EEPROM_CPP)
        if (Modify_string(&o_dirs_topic, v_ip2, _element.payload, (char *)&ram_data.p_NTP_settings()->serversNTP[1]))
            ram_data.unsaved = true;
#endif
        else
            return;
    }
    else if (!strcmp(arr_dirs[4], ArrVarTopic[v_ip3]))
    {
#if defined(EEPROM_C)
        if (Modify_string(&o_dirs_topic, v_ip3, _element.payload, (char *)&g_p_NTP_settings_ROM->serversNTP[2]))
            NeedSaveSettings.bit.NTP = true;
#elif defined(EEPROM_CPP)
        if (Modify_string(&o_dirs_topic, v_ip3, _element.payload, (char *)&ram_data.p_NTP_settings()->serversNTP[2]))
            ram_data.unsaved = true;
#endif
        else
            return;
    }
    // rsdebugDnfln("@3 d_ntp");
    init_NTP_with_WiFi();
#if defined(EEPROM_C)
    if (!NeedSaveSettings.bit.NTP)
#elif defined(EEPROM_CPP)
    if (!ram_data.unsaved)
#endif
        return;
    mqtt_publish_save_settings();
}
#endif

void cb_MQTT_com_set_OTA(s_element_MQTT _element)
{
    char arr_dirs[MQTT_MAX_LEVEL][MQTT_MAX_SYMBOLS]; // [уровней][символов]
    my_strtok(_element, &arr_dirs[0]);
    e_IDDirTopic dirs_topic[] = {d_main_topic, d_net, d_ota, d_settings};
    o_IDDirTopic o_dirs_topic = {dirs_topic, sizeof(dirs_topic) / sizeof(dirs_topic[0])};
    if (!strcmp(arr_dirs[4], ArrVarTopic[v_t_task]))
    {
#if defined(EEPROM_C)
        if (Modify_task_tTask(&o_dirs_topic, _element.payload, &ut_OTA, 1, 5000, &g_p_sys_settings_ROM->OTA_Ttask))
            NeedSaveSettings.sys.bit.OTA_T_task = true;
#elif defined(EEPROM_CPP)
        if (Modify_task_tTask(&o_dirs_topic, _element.payload, &ut_OTA, 1, 5000, &ram_data.p_SYS_settings()->OTA_Ttask))
            ram_data.unsaved = true;
#endif
        else
            return;
    }
    else
        return;
    mqtt_publish_save_settings();
}

void cb_MQTT_com_set_RSDebug(s_element_MQTT _element)
{
    char arr_dirs[MQTT_MAX_LEVEL][MQTT_MAX_SYMBOLS]; // [уровней][символов]
    my_strtok(_element, &arr_dirs[0]);
    e_IDDirTopic dirs_topic[] = {d_main_topic, d_net, d_rs_debug, d_settings, d_empty}; // ok
    o_IDDirTopic o_dirs_topic = {dirs_topic, sizeof(dirs_topic) / sizeof(dirs_topic[0])};
    if (!strcmp(arr_dirs[4], ArrVarTopic[v_t_task]))
    {
#if defined(EEPROM_C)
        if (Modify_task_tTask(&o_dirs_topic, _element.payload, &ut_debuglog, 10, 1000, &g_p_sys_settings_ROM->RSDebug_Ttask))
            NeedSaveSettings.sys.bit.RSDebug_T_task = true;
#elif defined(EEPROM_CPP)
        if (Modify_task_tTask(&o_dirs_topic, _element.payload, &ut_debuglog, 10, 1000, &ram_data.p_SYS_settings()->RSDebug_Ttask))
            ram_data.unsaved = true;
#endif
        else
            return;
    }
    else if (!strcmp(arr_dirs[4], ArrDirTopic[d_s_debug]))
    {
        o_dirs_topic._IDDirTopic[4] = d_s_debug;
        // rsdebugDlnF("***d_s_debug");
        if (!strcmp(arr_dirs[5], ArrVarTopic[v_enable]))
        {
#if defined(EEPROM_C)
            uint8_t tmp_result = Modify_bool(dirs_topic, v_enable, _element.payload, &g_p_sys_settings_ROM->RSDebug_SDebug, Debug.isSdebugEnabled()); // в минутах: от 1мин до 24ч
#elif defined(EEPROM_CPP)
            uint8_t tmp_result = Modify_bool(&o_dirs_topic, v_enable, _element.payload, &ram_data.p_SYS_settings()->RSDebug_SDebug, Debug.isSdebugEnabled()); // в минутах: от 1мин до 24ч
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
    else if (!strcmp(arr_dirs[4], ArrDirTopic[d_r_debug]))
    {
        o_dirs_topic._IDDirTopic[4] = d_r_debug;
        // rsdebugDlnF("***d_r_debug");
        if (!strcmp(arr_dirs[5], ArrVarTopic[v_enable]))
        {
#if defined(EEPROM_C)
            uint8_t tmp_result = Modify_bool(&o_dirs_topic, v_enable, _element.payload, &g_p_sys_settings_ROM->RSDebug_RDebug, Debug.isRdebugEnabled()); // в минутах: от 1мин до 24ч
#elif defined(EEPROM_CPP)
            uint8_t tmp_result = Modify_bool(&o_dirs_topic, v_enable, _element.payload, &ram_data.p_SYS_settings()->RSDebug_RDebug, Debug.isRdebugEnabled()); // в минутах: от 1мин до 24ч
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
    mqtt_publish_save_settings();
}

void cb_MQTT_com_set_SysMon(s_element_MQTT _element)
{
    char arr_dirs[MQTT_MAX_LEVEL][MQTT_MAX_SYMBOLS]; // [уровней][символов]
    my_strtok(_element, &arr_dirs[0]);
    e_IDDirTopic dirs_topic[] = {d_main_topic, d_system, d_sysmon, d_settings, d_empty}; // ok
    o_IDDirTopic o_dirs_topic = {dirs_topic, sizeof(dirs_topic) / sizeof(dirs_topic[0])};
    if (!strcmp(arr_dirs[4], ArrVarTopic[v_t_task]))
    {
#if defined(EEPROM_C)
        if (Modify_task_tTask(&o_dirs_topic, _element.payload, &ut_debuglog, 1000, 3600000, &g_p_sys_settings_ROM->SysMon_Ttask))
            NeedSaveSettings.sys.bit.RSDebug_T_task = true;
#elif defined(EEPROM_CPP)
        if (Modify_task_tTask(&o_dirs_topic, _element.payload, &ut_sysmon, 1000, 3600000, &ram_data.p_SYS_settings()->SysMon_Ttask))
            ram_data.unsaved = true;
#endif
        else
            return;
    }
    else if (!strcmp(arr_dirs[4], ArrDirTopic[d_info_mqtt]))
    {
        o_dirs_topic._IDDirTopic[4] = d_info_mqtt;
        if (!strcmp(arr_dirs[5], ArrVarTopic[v_enable]))
        {
#if defined(EEPROM_C)
            uint8_t tmp_result = Modify_bool(dirs_topic, v_enable, _element.payload, &g_p_sys_settings_ROM->SysMon_info_to_mqtt, v_b_SysMon_info_to_mqtt);
#elif defined(EEPROM_CPP)
            uint8_t tmp_result = Modify_bool(&o_dirs_topic, v_enable, _element.payload, &ram_data.p_SYS_settings()->SysMon_info_to_mqtt, v_b_SysMon_info_to_mqtt);
#endif
            if (!tmp_result) // 0 - нет изменеий
                return;
            if (tmp_result & 1) // 1 или 3 - payload != mem_data
            {
#if defined(EEPROM_C)
                не менял NeedSaveSettings.sys.bit.RSDebug_SD_en = true;
#elif defined(EEPROM_CPP)
                ram_data.unsaved = true;
#endif
            }
            if (tmp_result & 2) // 2 или 3 - payload != real_data
            {
#if defined(EEPROM_C)
                не менял Debug.setSdebugEnabled(g_p_sys_settings_ROM->SysMon_info_to_mqtt);
#elif defined(EEPROM_CPP)
                v_b_SysMon_info_to_mqtt = ram_data.p_SYS_settings()->SysMon_info_to_mqtt;
#endif
            }
        }
        else
            return;
    }
    else if (!strcmp(arr_dirs[4], ArrDirTopic[d_info_rsdebug]))
    {
        o_dirs_topic._IDDirTopic[4] = d_info_rsdebug;
        // rsdebugDlnF("***d_r_debug");
        if (!strcmp(arr_dirs[5], ArrVarTopic[v_enable]))
        {
#if defined(EEPROM_C)
            uint8_t tmp_result = Modify_bool(&o_dirs_topic, v_enable, _element.payload, &g_p_sys_settings_ROM->SysMon_info_to_rsdebug, v_b_SysMon_info_to_rsdebug);
#elif defined(EEPROM_CPP)
            uint8_t tmp_result = Modify_bool(&o_dirs_topic, v_enable, _element.payload, &ram_data.p_SYS_settings()->SysMon_info_to_rsdebug, v_b_SysMon_info_to_rsdebug);
#endif
            if (!tmp_result) // 0 - нет изменеий
                return;
            if (tmp_result & 1) // 1 или 3 - payload != mem_data
            {
#if defined(EEPROM_C)
                не менял NeedSaveSettings.sys.bit.RSDebug_RD_en = true;
#elif defined(EEPROM_CPP)
                ram_data.unsaved = true;
#endif
            }
            if (tmp_result & 2) // 2 или 3 - payload != real_data
            {
#if defined(EEPROM_C)
                не менял Debug.setRdebugEnabled(g_p_sys_settings_ROM->SysMon_info_to_rsdebug);
#elif defined(EEPROM_CPP)
                v_b_SysMon_info_to_rsdebug = ram_data.p_SYS_settings()->SysMon_info_to_rsdebug;
#endif
                // init_rdebuglog();
            }
        }
        else
            return;
    }
    else
        return;
    mqtt_publish_save_settings();
}

void cb_MQTT_com_set_ROM(s_element_MQTT _element)
{
    char arr_dirs[MQTT_MAX_LEVEL][MQTT_MAX_SYMBOLS]; // [уровней][символов]
    my_strtok(_element, &arr_dirs[0]);
    e_IDDirTopic dirs_topic[] = {d_main_topic, d_net, d_rs_debug, d_settings, d_free_up_ram}; // пока одна
    o_IDDirTopic o_dirs_topic = {dirs_topic, sizeof(dirs_topic) / sizeof(dirs_topic[0])};
    if (!strcmp(arr_dirs[6], ArrVarTopic[v_t_task]))
    {
#if defined(EEPROM_C)
        if (Modify_task_tTask(&o_dirs_topic, _element.payload, &ut_emptymemory, 1000, 10000, &g_p_sys_settings_ROM->FreeUpRAM_Ttask))
            NeedSaveSettings.sys.bit.RSDebug_T_task = true;
#elif defined(EEPROM_CPP)
        if (Modify_task_tTask(&o_dirs_topic, _element.payload, &ut_emptymemory, 1000, 10000, &ram_data.p_SYS_settings()->FreeUpRAM_Ttask))
            ram_data.unsaved = true;
#endif
        else
            return;
    }
    else
        return;
    mqtt_publish_save_settings();
}

void mqtt_subscribe(s_SubscribeElement _sub_element)
{
    o_IDDirTopic o_dirs_topic = {_sub_element.IDDirTopic, sizeof(_sub_element.IDDirTopic) / sizeof(_sub_element.IDDirTopic[0])};
    String topic = CreateTopic(&o_dirs_topic, _sub_element.IDVarTopic);
    rsdebugDnfln("sub %s", topic.c_str());
    client.subscribe(topic.c_str(), _sub_element.mqttQOS);
}

void mqtt_unsubscribe(s_SubscribeElement _sub_element)
{
    // на будущее
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

bool Modify_string(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, char _data[])
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

bool Modify_pass(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, char _data[])
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

bool Modify_task_tTask(o_IDDirTopic *_dir_topic, const char *_payload, uTask *_task, uint32_t _tMin, uint32_t _tMax, uint32_t *p_T_task)
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

uint8_t Modify_num(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, int32_t _tMin, int32_t _tMax, int32_t *_mem_num, int32_t _real_val)
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

uint8_t Modify_num(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, uint32_t _tMin, uint32_t _tMax, uint32_t *_mem_num, uint32_t _real_val)
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

uint8_t Modify_num(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, uint16_t _tMin, uint16_t _tMax, uint16_t *_mem_num, uint16_t _real_val)
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
        mqtt_publish(_dir_topic, _IDVarTopic, (uint32_t)*_mem_num);
    }
    if (*_mem_num != _real_val)
        ret |= 2;
    return ret;
}

uint8_t Modify_num(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, int8_t _tMin, int8_t _tMax, int8_t *_mem_num, int8_t _real_val)
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
        mqtt_publish(_dir_topic, _IDVarTopic, (int32_t)*_mem_num);
    }
    if (*_mem_num != _real_val)
        ret |= 2;
    return ret;
}

uint8_t Modify_num(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, uint8_t _tMin, uint8_t _tMax, uint8_t *_mem_num, uint8_t _real_val)
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
        mqtt_publish(_dir_topic, _IDVarTopic, (uint32_t)*_mem_num);
    }
    if (*_mem_num != _real_val)
        ret |= 2;
    return ret;
}

// 0 - нет изменеий
// 1 - payload != mem_data
// 2 - payload != real_data
// 3 - payload != mem_data и payload != real_data
uint8_t Modify_bool(o_IDDirTopic *_dir_topic, e_IDVarTopic _IDVarTopic, const char *_payload, bool *_mem_bool, bool _real_val)
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
