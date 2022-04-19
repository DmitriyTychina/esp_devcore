#include <Arduino.h>
#include "my_sysmon.h"
#include "main.h"
#include "my_debuglog.h"
#include "my_MQTT.h"
#include "my_EEPROM.h"

void SysMon_Init(void)
{
  ut_sysmon.StartStopwatchCore();
  LoadInMemorySettingsSys();
  ut_sysmon.setInterval(g_p_sys_settings_ROM->SysMon_Ttask);
  ut_sysmon.cpuLoadReset();
  ut_sysmon.enable();
}

void cb_sysmon(void)
{
  rsdebugInf("\n");
  uint32_t freeRAM = ESP.getFreeHeap();

  e_IDDirTopic dir_topic[] = {_main_topic, _Info, _SysMon, _empty};
  rsdebugInfln("FreeRAM: %u", freeRAM);
  rsdebugInfln("FreeSketchSpace: %u", ESP.getFreeSketchSpace());
  mqtt_publish(dir_topic, _FreeRAM, freeRAM);
  unsigned long cpuTot = ut_sysmon.getCpuLoadTotal();
  float CPUload = (float)ut_sysmon.getCpuLoadCycle() / (float)cpuTot * 100;
  float CPUCore = (float)ut_sysmon.getCpuLoadCore() / (float)cpuTot * 100;
  float CPUidle = 100 - CPUload - CPUCore;
  mqtt_publish(dir_topic, _CPUload, (float)CPUload, "%.3f");
  mqtt_publish(dir_topic, _CPUcore, (float)CPUCore, "%.3f");
  // rsdebugInfln("CPU cpuTot %u of time.", cpuTot);
  rsdebugInfln("CPU core %.3f %% of time.", CPUCore);
  rsdebugInfln("CPU work %.3f %% of time.", CPUload);
  rsdebugInfln("CPU sleep %.3f %% of time.", CPUidle);
  // rsdebugDnfln("wifi_station_get_rssi: %d", wifi_station_get_rssi());

  // ut_sysmon.cpuLoadReset(); // здесь не надо !!!
}
