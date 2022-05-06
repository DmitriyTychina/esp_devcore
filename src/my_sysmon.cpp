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
  uint32_t FreeRAM, RAMFragmentation, MaxFreeBlockSize;
  unsigned long cpuTot;
  float CPUload, CPUCore, CPUidle;

  if (v_b_SysMon_info_to_mqtt || v_b_SysMon_info_to_rsdebug)
  {
    FreeRAM = ESP.getFreeHeap();
    RAMFragmentation = ESP.getHeapFragmentation();
    MaxFreeBlockSize = ESP.getMaxFreeBlockSize();
    cpuTot = ut_sysmon.getCpuLoadTotal();
    CPUload = (float)ut_sysmon.getCpuLoadCycle() / (float)cpuTot * 100;
    CPUCore = (float)ut_sysmon.getCpuLoadCore() / (float)cpuTot * 100;
    CPUidle = 100 - CPUload - CPUCore;
  }
  if (v_b_SysMon_info_to_rsdebug)
  {
    rsdebugInf("\n");
    rsdebugInfln("***********************************************");
    // rsdebugInfln("CPU cpuTot %u of time.", cpuTot);
    if (wifi_state == _wifi_connected)
      rsdebugInfln("WiFi.RSSI: %d", WiFi.RSSI());
    rsdebugInfln("FreeRAM: %d", FreeRAM);
    rsdebugInfln("RAMFragmentation: %d", RAMFragmentation); //*
    rsdebugInfln("MaxFreeBlockSize: %d", MaxFreeBlockSize); //*
    rsdebugInfln("CPUload core %.3f %%", CPUCore);
    rsdebugInfln("CPUload work %.3f %%", CPUload);
    rsdebugInfln("CPUload sleep %.3f %%", CPUidle);
  }
  if (v_b_SysMon_info_to_mqtt)
    MQTT_pub_SysInfo(FreeRAM, RAMFragmentation, MaxFreeBlockSize, CPUload, CPUCore);
  // ut_sysmon.cpuLoadReset(); // здесь не надо !!!
}
