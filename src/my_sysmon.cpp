#include <Arduino.h>
#include "my_sysmon.h"
#include "main.h"
#include "my_debuglog.h"
#include "my_MQTT.h"
// #include "my_MQTT_sub.h"
#include "my_MQTT_pub.h"
#include "my_EEPROM.h"

bool v_b_SysMon_info_to_mqtt = true;
bool v_b_SysMon_info_to_rsdebug = true;

void SysMon_Init(void)
{
  ut_sysmon.StartStopwatchCore();
  // rsdebugDnflnF("#1");
#if defined(EEPROM_C)
  LoadInMemorySettingsSys();
  ut_sysmon.setInterval(g_p_sys_settings_ROM->SysMon_Ttask);
#elif defined(EEPROM_CPP)
  ut_sysmon.setInterval(ram_data.p_SYS_settings()->SysMon_Ttask);
  v_b_SysMon_info_to_mqtt = ram_data.p_SYS_settings()->SysMon_info_to_mqtt;
  v_b_SysMon_info_to_rsdebug = ram_data.p_SYS_settings()->SysMon_info_to_rsdebug;
#endif
  // rsdebugDnflnF("#2");
  ut_sysmon.cpuLoadReset();
  // rsdebugDnflnF("#3");
  ut_sysmon.enable();
  // rsdebugDnflnF("#4");
}

void cb_ut_sysmon(void)
{
  // uint32_t FreeRAM, RAMFragmentation, MaxFreeBlockSize;
  // unsigned long cpuTot;
  // float CPUload, CPUCore, CPUidle;
  s_Info_t s_Info;

  if (v_b_SysMon_info_to_mqtt || v_b_SysMon_info_to_rsdebug)
  {
    s_Info.WiFiRSSI = WiFi.RSSI();
    s_Info.FreeRAM = ESP.getFreeHeap();
#if defined(ESP32)
#elif defined(ESP8266)
    s_Info.RAMFragmentation = ESP.getHeapFragmentation();
    s_Info.MaxFreeBlockSize = ESP.getMaxFreeBlockSize();
#endif
    s_Info.cpuTot = ut_sysmon.getCpuLoadTotal();
    s_Info.CPUload = (float)ut_sysmon.getCpuLoadCycle() / (float)s_Info.cpuTot * 100;
    s_Info.CPUCore = (float)ut_sysmon.getCpuLoadCore() / (float)s_Info.cpuTot * 100;
    s_Info.CPUidle = 100 - s_Info.CPUload - s_Info.CPUCore;
  }
  if (v_b_SysMon_info_to_rsdebug)
  {
    rsdebugInf("\n");
    rsdebugInfln("***********************************************");
    // rsdebugInfln("CPU cpuTot %u of time.", cpuTot);
    if (wifi_state == _wifi_connected)
      rsdebugInfln("WiFi.RSSI: %d", s_Info.WiFiRSSI);
    rsdebugInfln("FreeRAM: %d", s_Info.FreeRAM);
#if defined(ESP32)
#elif defined(ESP8266)
    rsdebugInfln("RAMFragmentation: %d", s_Info.RAMFragmentation); //*
    rsdebugInfln("MaxFreeBlockSize: %d", s_Info.MaxFreeBlockSize); //*
#endif
    rsdebugInfln("CPUload core %.3f %%", s_Info.CPUCore);
    rsdebugInfln("CPUload work %.3f %%", s_Info.CPUload);
    rsdebugInfln("CPUload sleep %.3f %%", s_Info.CPUidle);
  }
  if (v_b_SysMon_info_to_mqtt)
    MQTT_pub_SysInfo(&s_Info);
  // ut_sysmon.cpuLoadReset(); // здесь не надо !!!
}
