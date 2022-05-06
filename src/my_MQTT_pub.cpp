#include <Arduino.h>
// #include <NtpClientLib.h>
#include <EEPROM.h>

#include "my_MQTT_pub.h"
#include "my_debuglog.h"
#include "my_wifi.h"
#include "my_NTP.h"

void MQTT_pub_allSettings(e_settings_from from)
{
  s_ethernet_settings_ROM *p_ethernet_settings;
  s_sys_settings_ROM *p_sys_settings;
  s_NTP_settings_ROM *p_NTP_settings;
#if defined(EEPROM_C)
  s_all_settings_ROM def_all_settings_ROM;
#elif defined(EEPROM_CPP)
#endif

  switch (from)
  {
  case from_def:
    rsdebugInflnF("Публикуем данные по умолчанию");
#if defined(EEPROM_C) // не переделано
    p_ethernet_settings = &def_all_settings_ROM.ethernet_settings_ROM;
    p_sys_settings = &def_all_settings_ROM.sys_settings_ROM;
    p_NTP_settings = &def_all_settings_ROM.NTP_settings_ROM;
#elif defined(EEPROM_CPP)
    p_ethernet_settings = def_data.p_NET_settings();
    p_sys_settings = def_data.p_SYS_settings();
    p_NTP_settings = def_data.p_NTP_settings();
#endif
    break;
  case from_rom:
    rsdebugInflnF("Публикуем сохраненные данные");
#if defined(EEPROM_C) // не переделано
    hjkl;
#elif defined(EEPROM_CPP)
    p_ethernet_settings = rom_data.p_NET_settings();
    p_sys_settings = rom_data.p_SYS_settings();
    p_NTP_settings = rom_data.p_NTP_settings();
#endif
    break;
  case from_ram:
    rsdebugInflnF("Публикуем текущие данные"); // ROM или памяти если не сохранены
#if defined(EEPROM_C)                          // не переделано
    LoadInMemorySettingsEthernet();
    p_ethernet_settings = g_p_ethernet_settings_ROM;
    LoadInMemorySettingsSys();
    p_sys_settings = g_p_sys_settings_ROM;
    LoadInMemorySettingsNTP();
    p_NTP_settings = g_p_NTP_settings_ROM;
#elif defined(EEPROM_CPP)
    p_ethernet_settings = ram_data.p_NET_settings();
    p_sys_settings = ram_data.p_SYS_settings();
    p_NTP_settings = ram_data.p_NTP_settings();
#endif
    break;
  default:
    break;
  }

  // публикуем все сетевые настройки
  // NET/WiFi
  e_IDDirTopic dirs_topic[] = {d_main_topic, d_net, d_wifi, d_settings, d_empty}; // ok
  o_IDDirTopic o_dirs_topic = {dirs_topic, (uint32_t)(sizeof(dirs_topic) / sizeof(dirs_topic[0]))};
  mqtt_publish(&o_dirs_topic, v_t_task, p_ethernet_settings->WiFi_Ttask);
  e_IDDirTopic tmp_dirs[] = {d_ap1, d_ap2, d_ap3, d_ap4, d_ap5};
  for (uint8_t w = 0; w < (sizeof(tmp_dirs) / sizeof(tmp_dirs[0])); w++)
  {
    o_dirs_topic._IDDirTopic[4] = tmp_dirs[w];
    mqtt_publish(&o_dirs_topic, v_ssid, p_ethernet_settings->settings_serv[w].SSID);
    mqtt_publish(&o_dirs_topic, v_pass, "*****" /* p_ethernet_settings->settings_serv[i].PASS */);
    mqtt_publish(&o_dirs_topic, v_server, p_ethernet_settings->settings_serv[w].MQTTip);
  }
  // NET/MQTT
  o_dirs_topic._IDDirTopic[4] = d_empty;
  o_dirs_topic._IDDirTopic[2] = d_mqtt; // {d_main_topic, d_net, d_mqtt, d_settings, d_empty, d_empty}
  mqtt_publish(&o_dirs_topic, v_t_task, p_ethernet_settings->MQTT_Ttask);
  mqtt_publish(&o_dirs_topic, v_user, "*****" /* p_ethernet_settings->MQTT_login */);
  mqtt_publish(&o_dirs_topic, v_pass, "*****" /* p_ethernet_settings->MQTT_pass */);
  // NET/NTP
  o_dirs_topic._IDDirTopic[2] = d_ntp; // {d_main_topic, d_net, d_ntp, d_settings, d_empty, d_empty}
  mqtt_publish(&o_dirs_topic, v_t_task, p_NTP_settings->Ttask);
  mqtt_publish(&o_dirs_topic, v_t_sync, p_NTP_settings->PeriodSyncNTP);
  mqtt_publish(&o_dirs_topic, v_ip1, p_NTP_settings->serversNTP[0]);
  mqtt_publish(&o_dirs_topic, v_ip2, p_NTP_settings->serversNTP[1]);
  mqtt_publish(&o_dirs_topic, v_ip3, p_NTP_settings->serversNTP[2]);
  mqtt_publish(&o_dirs_topic, v_timezone, p_NTP_settings->timezone);
  o_dirs_topic._IDDirTopic[4] = d_astro; // {d_main_topic, d_net, d_ntp, d_settings, d_astro, d_empty}
  mqtt_publish(&o_dirs_topic, v_enable, p_NTP_settings->timezone);
  o_dirs_topic._IDDirTopic[4] = d_empty; // {d_main_topic, d_net, d_ntp, d_settings, d_empty, d_empty}
  // NET/OTA
  o_dirs_topic._IDDirTopic[2] = d_ota; // {d_main_topic, d_net, d_ota, d_settings, d_empty, d_empty}
  mqtt_publish(&o_dirs_topic, v_t_task, p_sys_settings->OTA_Ttask);
  // NET/RSdebug
  o_dirs_topic._IDDirTopic[2] = d_rs_debug; // {d_main_topic, d_net, d_rs_debug, d_settings, d_empty, d_empty}
  mqtt_publish(&o_dirs_topic, v_t_task, p_sys_settings->RSDebug_Ttask);
  o_dirs_topic._IDDirTopic[4] = d_s_debug; // {d_main_topic, d_net, d_rs_debug, d_settings, d_s_debug, d_empty}
  mqtt_publish(&o_dirs_topic, v_enable, p_sys_settings->RSDebug_SDebug /* ? "yes" : "no"*/);
  o_dirs_topic._IDDirTopic[4] = d_r_debug; // {d_main_topic, d_net, d_rs_debug, d_settings, d_r_debug, d_empty}
  mqtt_publish(&o_dirs_topic, v_enable, p_sys_settings->RSDebug_RDebug /* ? "yes" : "no"*/);
  // System/Sysmon
  o_dirs_topic._IDDirTopic[1] = d_system;
  o_dirs_topic._IDDirTopic[2] = d_sysmon;
  o_dirs_topic._IDDirTopic[4] = d_empty; // {d_main_topic, d_system, d_sysmon, d_settings, d_empty, d_empty}
  mqtt_publish(&o_dirs_topic, v_t_task, p_sys_settings->SysMon_Ttask);
  o_dirs_topic._IDDirTopic[4] = d_info_rsdebug; // {d_main_topic, d_system, d_sysmon, d_settings, d_info_rsdebug, d_empty}
  mqtt_publish(&o_dirs_topic, v_enable, p_sys_settings->SysMon_info_to_rsdebug);
  o_dirs_topic._IDDirTopic[4] = d_info_mqtt; // {d_main_topic, d_system, d_sysmon, d_settings, d_info_rsdebug, d_empty}
  mqtt_publish(&o_dirs_topic, v_enable, p_sys_settings->SysMon_info_to_mqtt);
  // System/ROM
  o_dirs_topic._IDDirTopic[2] = d_rom;
  o_dirs_topic._IDDirTopic[4] = d_free_up_ram; // {d_main_topic, d_system, d_rom, d_settings, d_free_up_ram, d_empty}
  mqtt_publish(&o_dirs_topic, v_t_task, p_sys_settings->FreeUpRAM_Ttask);
  // System/StatusLED

#ifdef USER_AREA
  // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN

  //   mqtt_publish(dirs_topic, v_t_task, g_p  dirs_topic[1] = d_devices;
  //   dirs_topic[2] = _NTC;
  //   dirs_topic[3] = d_settings;
  //   LoadInMemorySettingsNTC();
  // _NTC_settings_ROM->Ttask);
  //   mqtt_publish(dirs_topic, _T_MQTT, g_p_NTC_settings_ROM->T_MQTT);
  //   mqtt_publish(dirs_topic, v_enable, g_p_NTC_settings_ROM->data_NTC_const.enable /*  ? "yes" : "no" */);
  //   mqtt_publish(dirs_topic, _DataDefault, g_p_NTC_settings_ROM->data_NTC_const.NTC_t_def);
  //   mqtt_publish(dirs_topic, _DataMin, g_p_NTC_settings_ROM->data_NTC_const.NTC_min_t);
  //   mqtt_publish(dirs_topic, _DataMax, g_p_NTC_settings_ROM->data_NTC_const.NTC_max_t);
  //   mqtt_publish(dirs_topic, _DataAge, g_p_NTC_settings_ROM->data_NTC_const.NTC_max_age);
  //   mqtt_publish(dirs_topic, _Kfiltr, g_p_NTC_settings_ROM->data_NTC_const.NTC_Kfiltr);
  //   mqtt_publish(dirs_topic, _Vcc, g_p_NTC_settings_ROM->data_NTC_const.NTC_vcc, "%.5g");
  //   mqtt_publish(dirs_topic, _Rs, g_p_NTC_settings_ROM->data_NTC_const.NTC_Rs);
  //   mqtt_publish(dirs_topic, _Ka, g_p_NTC_settings_ROM->data_NTC_const.NTC_Ka, "%.5g");
  //   mqtt_publish(dirs_topic, _Kb, g_p_NTC_settings_ROM->data_NTC_const.NTC_Kb, "%.5g");
  //   mqtt_publish(dirs_topic, _Kc, g_p_NTC_settings_ROM->data_NTC_const.NTC_Kc, "%.5g");
  // // EmptyMemorySettingsNTC();

// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
}

void MQTT_pub_Info_NTP(String _str = "")
{
  e_IDDirTopic dirs_topic[] = {d_main_topic, d_system, d_chip_esp}; // ok
  o_IDDirTopic o_dirs_topic = {dirs_topic, (uint32_t)(sizeof(dirs_topic) / sizeof(dirs_topic[0]))};
  mqtt_publish(&o_dirs_topic, v_time_reset, NTP.getTimeDateString(NTP.getLastBootTime()).c_str());
  o_dirs_topic._IDDirTopic[1] = d_net;
  o_dirs_topic._IDDirTopic[2] = d_ntp;
  mqtt_publish(&o_dirs_topic, v_server, NTP.getNtpServerName().c_str());
  if (_str.length() > 0)
    mqtt_publish(&o_dirs_topic, v_error, _str.c_str());
  // Astro
}

void MQTT_pub_SysInfo(uint32_t _FreeRAM, uint32_t _RAMFragmentation, uint32_t _MaxFreeBlockSize, float _CPUload, float _CPUCore)
{
  e_IDDirTopic dirs_topic[] = {d_main_topic, d_system, d_sysmon}; // ok
  o_IDDirTopic o_dirs_topic = {dirs_topic, (uint32_t)(sizeof(dirs_topic) / sizeof(dirs_topic[0]))};
  mqtt_publish(&o_dirs_topic, v_current_wifi_rssi, WiFi.RSSI());
  // o_dirs_topic._IDDirTopic[1] = d_system;
  // o_dirs_topic._IDDirTopic[2] = d_ram;
  mqtt_publish(&o_dirs_topic, v_free_ram, _FreeRAM);
  mqtt_publish(&o_dirs_topic, v_ram_fragm, _RAMFragmentation);
  mqtt_publish(&o_dirs_topic, v_ram_max_free_block, _MaxFreeBlockSize);
  // o_dirs_topic._IDDirTopic[2] = d_chip_esp;
  mqtt_publish(&o_dirs_topic, v_cpu_load_work, _CPUload, "%.3f");
  mqtt_publish(&o_dirs_topic, v_cpu_load_core, _CPUCore, "%.3f");
  // e_IDDirTopic dirs_topic[] = {d_main_topic, d_net, d_wifi, d_empty, d_empty, d_empty};
  // o_IDDirTopic o_dirs_topic = {dirs_topic, (uint32_t)(sizeof(dirs_topic) / sizeof(dirs_topic[0]))};
  // mqtt_publish(&o_dirs_topic, v_current_wifi_rssi, WiFi.RSSI());
  // o_dirs_topic._IDDirTopic[1] = d_system;
  // o_dirs_topic._IDDirTopic[2] = d_ram;
  // mqtt_publish(&o_dirs_topic, v_free_ram, _FreeRAM);
  // mqtt_publish(&o_dirs_topic, v_ram_fragm, _RAMFragmentation);
  // mqtt_publish(&o_dirs_topic, v_ram_max_free_block, _MaxFreeBlockSize);
  // o_dirs_topic._IDDirTopic[2] = d_chip_esp;
  // mqtt_publish(&o_dirs_topic, v_cpu_load_work, _CPUload, "%.3f");
  // mqtt_publish(&o_dirs_topic, v_cpu_load_core, _CPUCore, "%.3f");
}

void MQTT_pub_allInfo(bool _all)
{
  // NET/WiFi
  e_IDDirTopic dirs_topic[] = {d_main_topic, d_net, d_wifi}; // ok
  o_IDDirTopic o_dirs_topic = {dirs_topic, (uint32_t)(sizeof(dirs_topic) / sizeof(dirs_topic[0]))};
  mqtt_publish(&o_dirs_topic, v_current_wifi_ssid, WiFi.SSID().c_str());
  mqtt_publish(&o_dirs_topic, v_currentip, WiFi.localIP().toString().c_str());
  mqtt_publish(&o_dirs_topic, v_cnt_reconn_wifi, wifi_count_conn);
  // NET/MQTT
  o_dirs_topic._IDDirTopic[2] = d_mqtt; // {d_main_topic, d_net, d_mqtt, d_empty, d_empty, d_empty}
  mqtt_publish(&o_dirs_topic, v_cnt_reconn_mqtt, mqtt_count_conn);
  // NET/NTP
  MQTT_pub_Info_NTP();
  // // только после сброса
  // if (_all)
  // {
  // System
  o_dirs_topic._IDDirTopic[1] = d_system;
  o_dirs_topic._IDDirTopic[2] = d_empty; // {d_main_topic, d_system, d_empty, d_empty, d_empty, d_empty}
  mqtt_publish_no(&o_dirs_topic, vc_ReadAll);
  // System/Flash
  o_dirs_topic._IDDirTopic[2] = d_flash; // {d_main_topic, d_system, d_flash, d_empty, d_empty, d_empty}
  mqtt_publish(&o_dirs_topic, v_total_size, ESP.getFlashChipRealSize());
  mqtt_publish(&o_dirs_topic, v_size_sketch, ESP.getSketchSize());
  mqtt_publish(&o_dirs_topic, v_free_size_sketh, ESP.getFreeSketchSpace());
  mqtt_publish(&o_dirs_topic, v_flash_freq, ESP.getFlashChipSpeed() / 1000000);
  // System/RAM
  o_dirs_topic._IDDirTopic[2] = d_ram; // {d_main_topic, d_system, d_ram, d_empty, d_empty, d_empty}
  mqtt_publish(&o_dirs_topic, v_total_size, 81920);
  // System/ROM
  o_dirs_topic._IDDirTopic[2] = d_rom; // {d_main_topic, d_system, d_rom, d_empty, d_empty, d_empty}
  mqtt_publish(&o_dirs_topic, v_total_size, 4096);
  mqtt_publish(&o_dirs_topic, v_used, (uint32_t)len_all_settings_ROM);
  mqtt_publish_no(&o_dirs_topic, vc_ReadDef);
  mqtt_publish_no(&o_dirs_topic, vc_ReadROM);
  mqtt_publish_no(&o_dirs_topic, vc_ReadRAM);
  mqtt_publish_save_settings();
  // System/ChipESP
  o_dirs_topic._IDDirTopic[2] = d_chip_esp; // {d_main_topic, d_system, d_chip_esp, d_empty, d_empty, d_empty}
  mqtt_publish(&o_dirs_topic, v_reason_reset, get_glob_reason().c_str());
  mqtt_publish(&o_dirs_topic, v_cpu_freq, ESP.getCpuFreqMHz());
  mqtt_publish(&o_dirs_topic, v_ver_core, ESP.getCoreVersion().c_str());
  mqtt_publish(&o_dirs_topic, v_ver_sdk, ESP.getSdkVersion());
  mqtt_publish_ok(&o_dirs_topic, vc_Debug);
  // System/StatusLED

  // }

#ifdef USER_AREA
  // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
  onStartMQTT_pub_door();
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

  cb_ut_sysmon(); // чтоб вывелась системная информация
}

// void onMessagePublish(uint16_t packetId)
// {
// }

void mqtt_publish(o_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, bool _payload)
{
  // if (!client.connected())
  //   return;
  mqtt_publish(_IDDirTopic, _IDVarTopic, _payload ? "yes" : "no");
}

void mqtt_publish(o_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, int32_t _payload)
{
  if (!client.connected())
    return;
  char msg[16];
  sprintf(msg, "%d", _payload);
  mqtt_publish(_IDDirTopic, _IDVarTopic, msg);
}

void mqtt_publish(o_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, uint32_t _payload)
{
  if (!client.connected())
    return;
  char msg[16];
  sprintf(msg, "%d", _payload);
  mqtt_publish(_IDDirTopic, _IDVarTopic, msg);
}

void mqtt_publish(o_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, float _payload, const char *_format)
{
  if (!client.connected())
    return;
  char msg[16];
  sprintf(msg, _format, _payload);
  mqtt_publish(_IDDirTopic, _IDVarTopic, msg);
}

// void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, float payload)
// {
//   if (!client.connected())
//     return;
//   char msg[16];
//   sprintf(msg, "%.2f", payload);
//   mqtt_publish(_IDDirTopic, _IDVarTopic, msg);
// }

void mqtt_publish(o_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, const char *_payload)
{
  if (!client.connected())
    return;
  String topic = CreateTopic(_IDDirTopic, _IDVarTopic);
  mqtt_publish(topic.c_str(), _payload);
}

void mqtt_publish(const char *_topic, const char *_payload)
{
  if (!client.connected())
    return;
  // LedOn;
  client.publish(_topic, 0, false, _payload);
  rsdebugInfF("MQTT pub: ");
  rsdebugInfln("%s<%s>", _topic, _payload);
  // LedOff;
}

void mqtt_publish_ok(o_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic)
{
  mqtt_publish(_IDDirTopic, _IDVarTopic, "ok");
}

void mqtt_publish_no(o_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic)
{
  mqtt_publish(_IDDirTopic, _IDVarTopic, "no");
}

void mqtt_publish_ok(const char *_topic)
{
  mqtt_publish(_topic, "ok");
}

void mqtt_publish_no(const char *_topic)
{
  mqtt_publish(_topic, "no");
}

void mqtt_publish_save_settings()
{
  e_IDDirTopic dirs_topic[] = {d_main_topic, d_system, d_rom};
  o_IDDirTopic o_dirs_topic = {dirs_topic, (uint32_t)(sizeof(dirs_topic) / sizeof(dirs_topic[0]))};
  mqtt_publish(&o_dirs_topic, vc_Save, ram_data.unsaved ? "NeedSave" : "ok");
}
