#include <Arduino.h>
// #include <NtpClientLib.h>
#include <EEPROM.h>

#include "MQTT_pub.h"
#include "my_debuglog.h"
#include "my_EEPROM.h"
#include "my_wifi.h"
#include "my_NTP.h"

void MQTT_pub_allSettings(bool fromROM)
{
  s_all_settings_ROM def_all_settings_ROM;
  s_ethernet_settings_ROM *p_ethernet_settings;
  s_sys_settings_ROM *p_sys_settings;
  s_NTP_settings_ROM *p_NTP_settings;
  if (fromROM)
  {
    rsdebugInflnF("Публикуем текущие данные"); // ROM или памяти если не сохранены
    LoadInMemorySettingsEthernet();
    p_ethernet_settings = g_p_ethernet_settings_ROM;
    LoadInMemorySettingsSys();
    p_sys_settings = g_p_sys_settings_ROM;
    LoadInMemorySettingsNTP();
    p_NTP_settings = g_p_NTP_settings_ROM;
  }
  else
  {
    rsdebugInflnF("Публикуем данные по умолчанию");
    p_ethernet_settings = &def_all_settings_ROM.ethernet_settings_ROM;
    p_sys_settings = &def_all_settings_ROM.sys_settings_ROM;
    p_NTP_settings = &def_all_settings_ROM.NTP_settings_ROM;
  }

  // публикуем все сетевые настройки
  e_IDDirTopic dirs_topic[] = {d_main_topic, d_settings, d_empty, d_empty};
  dirs_topic[2] = d_mqtt; // {d_main_topic, d_settings, d_mqtt, d_empty}
  mqtt_publish(dirs_topic, v_user, "*****" /* p_ethernet_settings->MQTT_login */);
  mqtt_publish(dirs_topic, v_pass, "*****" /* p_ethernet_settings->MQTT_pass */);
  mqtt_publish(dirs_topic, v_t_task, p_ethernet_settings->MQTT_Ttask);
  dirs_topic[2] = d_wifi; // {d_main_topic, d_settings, d_wifi, d_empty}
  mqtt_publish(dirs_topic, v_t_task, p_ethernet_settings->WiFi_Ttask);
  e_IDDirTopic a_dirs[] = {d_ap1, d_ap2, d_ap3, d_ap4, d_ap5};
  for (uint8_t w = 0; w < (sizeof(a_dirs) / sizeof(a_dirs[0])); w++)
  {
    dirs_topic[3] = a_dirs[w];
    mqtt_publish(dirs_topic, v_ssid, p_ethernet_settings->settings_serv[w].SSID);
    mqtt_publish(dirs_topic, v_pass, "*****" /* p_ethernet_settings->settings_serv[i].PASS */);
    mqtt_publish(dirs_topic, v_ip_serv, p_ethernet_settings->settings_serv[w].MQTTip);
  }
  dirs_topic[3] = d_empty; // {d_main_topic, d_settings, d_wifi, d_empty}

  // публикуем все системные настройки
  dirs_topic[2] = d_rs_debug; // {d_main_topic, d_settings, d_rs_debug, d_empty}
  mqtt_publish(dirs_topic, v_t_task, p_sys_settings->RSDebug_Ttask);
  dirs_topic[3] = d_s_debug; // {d_main_topic, d_settings, d_settings, d_rs_debug, d_s_debug}
  mqtt_publish(dirs_topic, v_enable, p_sys_settings->RSDebug_SDebug /* ? "yes" : "no"*/);
  dirs_topic[3] = d_r_debug; // {d_main_topic, d_settings, d_rs_debug, d_r_debug}
  mqtt_publish(dirs_topic, v_enable, p_sys_settings->RSDebug_RDebug /* ? "yes" : "no"*/);
  dirs_topic[3] = d_empty; // {d_main_topic, d_settings, d_rs_debug, d_empty}
  dirs_topic[2] = d_sysmon; // {d_main_topic, d_settings, d_sysmon, d_empty}
  mqtt_publish(dirs_topic, v_t_task, p_sys_settings->SysMon_Ttask);
  dirs_topic[2] = d_ota; // {d_main_topic, d_settings, d_ota, d_empty}
  mqtt_publish(dirs_topic, v_t_task, p_sys_settings->OTA_Ttask);

  // публикуем все настройки NTP
  dirs_topic[2] = d_ntp; // {d_main_topic, d_settings, d_ntp, d_empty}
  mqtt_publish(dirs_topic, v_t_task, p_NTP_settings->Ttask);
  // mqtt_publish(dirs_topic, v_enable, p_NTP_settings->Ttask /* ? "yes" : "no"*/);
  mqtt_publish(dirs_topic, v_t_sync, p_NTP_settings->T_syncNTP);
  mqtt_publish(dirs_topic, v_ip1, p_NTP_settings->serversNTP[0]);
  mqtt_publish(dirs_topic, v_ip2, p_NTP_settings->serversNTP[1]);
  mqtt_publish(dirs_topic, v_ip3, p_NTP_settings->serversNTP[2]);
  mqtt_publish(dirs_topic, v_timezone, p_NTP_settings->timezone);
  // EmptyMemorySettingsNTP(true);

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
       // NeedSaveSettings.value = 0;
       // MQTT_pub_Settings_ok(_Save);
}

// void MQTT_pub_Commands_ok(e_IDVarTopic _IDVarTopic)
// {
//   e_IDDirTopic dirs_topic[] = {d_main_topic, d_commands, d_empty, d_empty};
//   // mqtt_publish(dirs_topic, _IDVarTopic, "ok");
//   mqtt_publish_ok(dirs_topic, _IDVarTopic);
// }

// void MQTT_pub_Settings_ok(e_IDVarTopic _IDVarTopic)
// {
//   e_IDDirTopic dirs_topic[] = {d_main_topic, d_settings, d_empty, d_empty};
//   mqtt_publish(dirs_topic, _IDVarTopic, "ok");
// }

void MQTT_pub_Info_TimeReset(void)
{
  e_IDDirTopic dirs_topic[] = {d_main_topic, d_info, d_empty, d_empty};
  mqtt_publish(dirs_topic, v_time_reset, NTP.getTimeDateString(NTP.getLastBootTime()).c_str());
}

void MQTT_pub_allInfo(bool reset)
{
  e_IDDirTopic dirs_topic[] = {d_main_topic, d_info, d_empty, d_empty};
  mqtt_publish_no(dirs_topic, vc_Read);
  mqtt_publish(dirs_topic, v_current_wifi_ap, WiFi.SSID().c_str());
  mqtt_publish(dirs_topic, v_currentip, WiFi.localIP().toString().c_str());
  mqtt_publish(dirs_topic, v_cnt_reconn_wifi, (uint32_t)wifi_count_conn);
  mqtt_publish(dirs_topic, v_cnt_reconn_mqtt, (uint32_t)mqtt_count_conn);

#ifdef USER_AREA
  // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
  onStartMQTT_pub_door();
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

  // только после сброса
  if (reset)
  {
    mqtt_publish(dirs_topic, v_reason_reset, get_glob_reason(false).c_str());
    dirs_topic[1] = d_settings;
    mqtt_publish_no(dirs_topic, vc_ReadCrnt);
    mqtt_publish_no(dirs_topic, vc_ReadDflt);
    mqtt_publish_ok(dirs_topic, vc_Save);
    mqtt_publish_ok(dirs_topic, vc_Debug);
#ifdef USER_AREA
    // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
  }

  MQTT_pub_Info_TimeReset();
  cb_ut_sysmon();
}
