#include <Arduino.h>
// #include <NtpClientLib.h>
#include <EEPROM.h>

#include "MQTT_pub.h"
#include "my_debuglog.h"
#include "my_EEPROM.h"
#include "my_wifi.h"
#include "my_NTP.h"

void MQTT_pub_allSettings(bool fromROMorDefault)
{
  s_all_settings_ROM def_all_settings_ROM;
  if(fromROMorDefault)
    rsdebugInflnF("Публикуем данные из ROM")
  else
    rsdebugInflnF("Публикуем данные по умолчанию")
  e_IDDirTopic dir_topic[] = {_main_topic, _Settings, _empty, _empty};

  s_ethernet_settings_ROM *p_ethernet_settings;
  if(fromROMorDefault)
  {
    LoadInMemorySettingsEthernet();
    p_ethernet_settings = g_p_ethernet_settings_ROM;
  }
  else
  {
    p_ethernet_settings = &def_all_settings_ROM.ethernet_settings_ROM;
  }
  dir_topic[2] = _MQTT; // {_main_topic, _Settings, _MQTT, _empty}
  mqtt_publish(dir_topic, _USER, "*****" /* p_ethernet_settings->MQTT_login */);
  mqtt_publish(dir_topic, _PASS, "*****" /* p_ethernet_settings->MQTT_pass */);
  mqtt_publish(dir_topic, _T_task, p_ethernet_settings->MQTT_Ttask);
  dir_topic[2] = _WIFI; // {_main_topic, _Settings, _WIFI, _empty}
  mqtt_publish(dir_topic, _T_task, p_ethernet_settings->WiFi_Ttask);
  e_IDDirTopic a_dirs[] = {_AP1, _AP2, _AP3, _AP4, _AP5};
  for (uint8_t w = 0; w < (sizeof(a_dirs) / sizeof(a_dirs[0])); w++)
  {
    dir_topic[3] = a_dirs[w];
    mqtt_publish(dir_topic, _SSID, p_ethernet_settings->settings_serv[w].SSID);
    mqtt_publish(dir_topic, _PASS, "*****" /* p_ethernet_settings->settings_serv[i].PASS */);
    mqtt_publish(dir_topic, _IP_serv, p_ethernet_settings->settings_serv[w].MQTTip);
    // rsdebugInfln("IP_MQTT_com0:%s[%d]", p_ethernet_settings->settings_serv[w].MQTTip, w);
  }
  dir_topic[3] = _empty; // {_main_topic, _Settings, _WIFI, _empty}
  // EmptyMemorySettingsEthernet(true);

  s_sys_settings_ROM *p_sys_settings;
  if(fromROMorDefault)
  {
    LoadInMemorySettingsSys();
    p_sys_settings = g_p_sys_settings_ROM;
  }
  else
  {
    p_sys_settings = &def_all_settings_ROM.sys_settings_ROM;
  }
  dir_topic[2] = _RSdebug; // {_main_topic, _Settings, _RSdebug, _empty}
  mqtt_publish(dir_topic, _T_task, p_sys_settings->RSDebug_Ttask);
  dir_topic[3] = _Sdebug; // {_main_topic, _Settings, _Settings, _RSdebug, _Sdebug}
  mqtt_publish(dir_topic, _Enable, p_sys_settings->RSDebug_SDebug /* ? "yes" : "no"*/);
  dir_topic[3] = _Rdebug; // {_main_topic, _Settings, _RSdebug, _Rdebug}
  mqtt_publish(dir_topic, _Enable, p_sys_settings->RSDebug_RDebug /* ? "yes" : "no"*/);
  dir_topic[3] = _empty;  // {_main_topic, _Settings, _RSdebug, _empty}
  dir_topic[2] = _SysMon; // {_main_topic, _Settings, _SysMon, _empty}
  mqtt_publish(dir_topic, _T_task, p_sys_settings->SysMon_Ttask);
  // mqtt_publish(dir_topic, _Enable, p_sys_settings->SysMon_Ttask != 0 /* ? "yes" : "no"*/);
  dir_topic[2] = _OTA; // {_main_topic, _Settings, _OTA, _empty}
  mqtt_publish(dir_topic, _T_task, p_sys_settings->OTA_Ttask);
  // EmptyMemorySettingsSys(true);

  s_NTP_settings_ROM *p_NTP_settings;
  if(fromROMorDefault)
  {
    LoadInMemorySettingsNTP();
    p_NTP_settings = g_p_NTP_settings_ROM;
  }
  else
  {
    p_NTP_settings = &def_all_settings_ROM.NTP_settings_ROM;
  }
  dir_topic[2] = _NTP; // {_main_topic, _Settings, _NTP, _empty}
  mqtt_publish(dir_topic, _T_task, p_NTP_settings->Ttask);
  // mqtt_publish(dir_topic, _Enable, p_NTP_settings->Ttask /* ? "yes" : "no"*/);
  mqtt_publish(dir_topic, _Tsync, p_NTP_settings->T_syncNTP);
  mqtt_publish(dir_topic, _IP1, p_NTP_settings->serversNTP[0]);
  mqtt_publish(dir_topic, _IP2, p_NTP_settings->serversNTP[1]);
  mqtt_publish(dir_topic, _IP3, p_NTP_settings->serversNTP[2]);
  mqtt_publish(dir_topic, _Timezone, p_NTP_settings->timezone);
  // EmptyMemorySettingsNTP(true);

#ifdef USER_AREA
// ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN

  //   mqtt_publish(dir_topic, _T_task, g_p  dir_topic[1] = _Devices;
  //   dir_topic[2] = _NTC;
  //   dir_topic[3] = _Settings;
  //   LoadInMemorySettingsNTC();
  // _NTC_settings_ROM->Ttask);
  //   mqtt_publish(dir_topic, _T_MQTT, g_p_NTC_settings_ROM->T_MQTT);
  //   mqtt_publish(dir_topic, _Enable, g_p_NTC_settings_ROM->data_NTC_const.enable /*  ? "yes" : "no" */);
  //   mqtt_publish(dir_topic, _DataDefault, g_p_NTC_settings_ROM->data_NTC_const.NTC_t_def);
  //   mqtt_publish(dir_topic, _DataMin, g_p_NTC_settings_ROM->data_NTC_const.NTC_min_t);
  //   mqtt_publish(dir_topic, _DataMax, g_p_NTC_settings_ROM->data_NTC_const.NTC_max_t);
  //   mqtt_publish(dir_topic, _DataAge, g_p_NTC_settings_ROM->data_NTC_const.NTC_max_age);
  //   mqtt_publish(dir_topic, _Kfiltr, g_p_NTC_settings_ROM->data_NTC_const.NTC_Kfiltr);
  //   mqtt_publish(dir_topic, _Vcc, g_p_NTC_settings_ROM->data_NTC_const.NTC_vcc, "%.5g");
  //   mqtt_publish(dir_topic, _Rs, g_p_NTC_settings_ROM->data_NTC_const.NTC_Rs);
  //   mqtt_publish(dir_topic, _Ka, g_p_NTC_settings_ROM->data_NTC_const.NTC_Ka, "%.5g");
  //   mqtt_publish(dir_topic, _Kb, g_p_NTC_settings_ROM->data_NTC_const.NTC_Kb, "%.5g");
  //   mqtt_publish(dir_topic, _Kc, g_p_NTC_settings_ROM->data_NTC_const.NTC_Kc, "%.5g");
  // // EmptyMemorySettingsNTC();

// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
  // NeedSaveSettings.value = 0;
  // MQTT_pub_Settings_ok(_Save);
}

// void MQTT_pub_Commands_ok(e_IDVarTopic _IDVarTopic)
// {
//   e_IDDirTopic dir_topic[] = {_main_topic, _Commands, _empty, _empty};
//   // mqtt_publish(dir_topic, _IDVarTopic, "ok");
//   mqtt_publish_ok(dir_topic, _IDVarTopic);
// }

// void MQTT_pub_Settings_ok(e_IDVarTopic _IDVarTopic)
// {
//   e_IDDirTopic dir_topic[] = {_main_topic, _Settings, _empty, _empty};
//   mqtt_publish(dir_topic, _IDVarTopic, "ok");
// }

void MQTT_pub_Info_TimeReset(void)
{
  e_IDDirTopic dir_topic[] = {_main_topic, _Info, _empty, _empty};
  mqtt_publish(dir_topic, _TimeReset, NTP.getTimeDateString(NTP.getLastBootTime()).c_str());
}
