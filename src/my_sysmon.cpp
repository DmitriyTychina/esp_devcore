#include <Arduino.h>
#include "my_sysmon.h"
#include "main.h"
#include "my_debuglog.h"
#include "my_MQTT.h"
// #include "my_MQTT_sub.h"
#include "my_MQTT_pub.h"
#include "my_EEPROM.h"

void SysMon_Init(void)
{
  ut_sysmon.StartStopwatchCore();
  // rsdebugDnflnF("#1");
#if defined(EEPROM_C)
  LoadInMemorySettingsSys();
  ut_sysmon.setInterval(g_p_sys_settings_ROM->SysMon_Ttask);
#elif defined(EEPROM_CPP)
  ut_sysmon.setInterval(ram_data.p_SYS_settings()->SysMon_Ttask);
#endif
  // rsdebugDnflnF("#2");
  ut_sysmon.cpuLoadReset();
  // rsdebugDnflnF("#3");
  ut_sysmon.enable();
  // rsdebugDnflnF("#4");
}

void cb_ut_sysmon(void)
{
  // ut_sysmon.setInterval(4000);

  // rsdebugInfln("***********************************************");

  // rsdebugInfln("getResetReason: %s", ESP.getResetReason().c_str());//1
  // rsdebugInfln("getFreeHeap: %d", ESP.getFreeHeap());                   //*
  // rsdebugInfln("getHeapFragmentation: %d", ESP.getHeapFragmentation()); //*
  // rsdebugInfln("getMaxFreeBlockSize: %d", ESP.getMaxFreeBlockSize());   //*
  // // rsdebugInfln("getChipId: %d", ESP.getChipId());
  // rsdebugInfln("getCoreVersion: %s", ESP.getCoreVersion());//1
  // rsdebugInfln("getSdkVersion: %s", ESP.getSdkVersion());//1
  // rsdebugInfln("getCpuFreqMHz: %d", ESP.getCpuFreqMHz());//1*?
  // rsdebugInfln("getSketchSize: %d", ESP.getSketchSize());//1
  // rsdebugInfln("getFreeSketchSpace: %d", ESP.getFreeSketchSpace());//1
  // // rsdebugInfln("getSketchMD5: %s", ESP.getSketchMD5().c_str());
  // // rsdebugInfln("getFlashChipId: %d", ESP.getFlashChipId());
  // rsdebugInfln("getFlashChipSize: %d", ESP.getFlashChipSize());
  // rsdebugInfln("getFlashChipRealSize: %d", ESP.getFlashChipRealSize());//1
  // rsdebugInfln("getFlashChipSpeed: %d", ESP.getFlashChipSpeed());//1
  // // rsdebugInfln("getCycleCount: %d", ESP.getCycleCount());

  rsdebugInf("\n");
  rsdebugInfln("***********************************************");
  uint32_t freeRAM = ESP.getFreeHeap();
  uint32_t RAMFragmentation = ESP.getHeapFragmentation();
  uint32_t MaxFreeBlockSize = ESP.getMaxFreeBlockSize();
  e_IDDirTopic dirs_topic[] = {d_main_topic, d_info, d_sysmon, d_empty};
  // rsdebugInfln("FreeSketchSpace: %u", ESP.getFreeSketchSpace());
  unsigned long cpuTot = ut_sysmon.getCpuLoadTotal();
  float CPUload = (float)ut_sysmon.getCpuLoadCycle() / (float)cpuTot * 100;
  float CPUCore = (float)ut_sysmon.getCpuLoadCore() / (float)cpuTot * 100;
  float CPUidle = 100 - CPUload - CPUCore;
  mqtt_publish(dirs_topic, v_free_ram, freeRAM);
  mqtt_publish(dirs_topic, v_cpu_load_work, (float)CPUload, "%.3f");
  mqtt_publish(dirs_topic, v_cpu_load_core, (float)CPUCore, "%.3f");
  // rsdebugInfln("CPU cpuTot %u of time.", cpuTot);
  rsdebugInfln("FreeRAM: %d", freeRAM);
  rsdebugInfln("RAMFragmentation: %d", RAMFragmentation); //*
  rsdebugInfln("MaxFreeBlockSize: %d", MaxFreeBlockSize); //*
  rsdebugInfln("CPUload core %.3f %%", CPUCore);
  rsdebugInfln("CPUload work %.3f %%", CPUload);
  rsdebugInfln("CPUload sleep %.3f %%", CPUidle);
  // rsdebugDnfln("wifi_station_get_rssi: %d", wifi_station_get_rssi());
  // rsdebugDnfln("WiFi.SSID: %s", WiFi.SSID().c_str());

  // ut_sysmon.cpuLoadReset(); // здесь не надо !!!
}
