#include <Arduino.h>
#include <ArduinoOTA.h>

#include "device_def.h"
#include "my_ota.h"
#include "my_scheduler.h"
#include "my_debuglog.h"
#include "main.h"
#include "my_EEPROM.h"

void init_OTA(void)
{
	LoadInMemorySettingsSys();
	// rsdebugDln("*****g_p_sys_settings_ROM->OTA_Ttask: %d", g_p_sys_settings_ROM->OTA_Ttask);
	ut_OTA.setInterval(g_p_sys_settings_ROM->OTA_Ttask);
	// EmptyMemorySettingsSys();

	// rsdebuglnF("init_OTA");
	ArduinoOTA.onStart([]()
		{
			rsdebugnflnF("*OTA: Start");
			// ut_emptymemory.disable();
			// ut_sysmon.setInterval(0);
			// ut_NTP.setInterval(0);
			// ut_MQTT.setInterval(0);
			// ut_wifi.setInterval(0);
			ut_OTA.setInterval(1);
			ut_OTA.forceNextIteration();
			});
	ArduinoOTA.onEnd([]()
					 { rsdebugnflnF("\n\r*OTA: End"); 
					//  os_delay_us(500000);
                    // ESP.restart();
					});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
						  { rsdebugInf("\r*OTA: Progress:    %d%%						 ", progress / (total / 100)); });
	ArduinoOTA.onError([](ota_error_t error)
		{
			rsdebugEnfln("*OTA: Error[%u]: ", error);});
			// Serial.printf("*OTA: Error[%u]: ", error);
			// rsdebugDln("*****g_p_sys_settings_ROM->OTA_Ttask: %d", g_p_sys_settings_ROM->OTA_Ttask);
			LoadInMemorySettingsSys();
			ut_OTA.setInterval(g_p_sys_settings_ROM->OTA_Ttask);
			// ut_sysmon.setInterval(g_p_sys_settings_ROM->SysMon_Ttask);
			// ut_sysmon.enable();
			// LoadInMemorySettingsEthernet();
			// ut_wifi.setInterval(g_p_ethernet_settings_ROM->WiFi_Ttask);
			// ut_wifi.enable();
			// ut_MQTT.setInterval(g_p_ethernet_settings_ROM->MQTT_Ttask);
			// ut_MQTT.enable();
			// LoadInMemorySettingsNTP();
			// ut_NTP.setInterval(g_p_NTP_settings_ROM->Ttask);
			// ut_NTP.enable();
			// ut_emptymemory.enable();
			
			// if (error == OTA_AUTH_ERROR)
			// 	Serial1.println("Auth Failed");
			// else if (error == OTA_BEGIN_ERROR)
			// 	Serial1.println("Begin Failed");
			// else if (error == OTA_CONNECT_ERROR)
			// 	Serial1.println("Connect Failed");
			// else if (error == OTA_RECEIVE_ERROR)
			// 	Serial1.println("Receive Failed");
			// else if (error == OTA_END_ERROR)
			// 	Serial1.println("End Failed"); 
			// LoadInMemorySettingsEthernet();
			// ut_wifi.setInterval(g_p_ethernet_settings_ROM->WiFi_Ttask);
	// #ifdef OTA_NAME
	ArduinoOTA.setHostname(OTA_NAME); // Задаем имя сетевого порта для удаленной прошивки
									  // #endif
									  // #ifdef OTA_PASS
	ArduinoOTA.setPassword(OTA_PASS); // Задаем пароль доступа для удаленной прошивки
									  // #endif
	ArduinoOTA.setRebootOnSuccess(true);
	ArduinoOTA.begin();
	// rsdebugDnflnF("OK");
}

void t_ota_cb(void)
{
	ArduinoOTA.handle();
	// rsdebugDnflnF("*OTA*");
}