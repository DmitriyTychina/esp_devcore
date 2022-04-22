#include <Arduino.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
// #include <NtpClientLib.h>

#include "my_scheduler.h"
#include "my_NTP.h"
#include "my_debuglog.h"
#include "my_EEPROM.h"
#include "MQTT_pub.h"

boolean syncEventTriggered; // True if a time even has been triggered
NTPSyncEvent_t ntpEvent;	// Last triggered event

#define minutesTimeZone 0
#define NTP_TIMEOUT 1500
#define qnt_errorNTP 5
uint8_t cnt_errorNTP = 0; // счетчик ошибок получения данных с сервера NTP для выбора другого сервера
uint8_t NumServerNTP = 0; // номер текущего сервера NTP

// NTPClient NTP; определен в <NtpClientLib.h>
s_NTP_settings_ROM *g_p_NTP_settings_ROM = NULL;

void processSyncEvent(NTPSyncEvent_t _ntpEvent)
{
	LoadInMemorySettingsNTP();
	if (_ntpEvent < 0)
	{
		// if (LoadInMemorySettingsNTP())
		// {
		// 	cnt_errorNTP = 0;
		// }
		if (++cnt_errorNTP > qnt_errorNTP)
		{
			cnt_errorNTP = 0;
			if (++NumServerNTP >= sizeof(g_p_NTP_settings_ROM->serversNTP) / sizeof(g_p_NTP_settings_ROM->serversNTP[0]))
			{
				NumServerNTP = 0;
			}
		}
		rsdebugWnfln("cnt_errorNTP=%d NumServerNTP=%d", cnt_errorNTP, NumServerNTP);
		rsdebugWnfln("serverNTP=%s", g_p_NTP_settings_ROM->serversNTP[NumServerNTP]);
		// NTP.begin(g_p_NTP_settings_ROM->serversNTP[NumServerNTP], g_p_NTP_settings_ROM->timezone, false /*переход на летнее/зимнее*/, minutesTimeZone);
		rsdebugEnfln("Time Sync error: %d", _ntpEvent);
		init_NTP_with_WiFi();
	}
	else if (_ntpEvent == timeSyncd)
	{
		cnt_errorNTP = 0;
		rsdebugInfln(" ");
		rsdebugInfln("Got NTP time: %s", NTP.getTimeDateString(NTP.getLastNTPSync()).c_str());
		rsdebugInfln("serverNTP=%s", g_p_NTP_settings_ROM->serversNTP[NumServerNTP]);
		rsdebugInfln("Uptime: %s since %s", NTP.getUptimeString().c_str(), NTP.getTimeDateString(NTP.getFirstSync()).c_str());
		// EmptyMemorySettingsNTP();
		MQTT_pub_Info_TimeReset();
	}
	else if (_ntpEvent == noResponse)
	{
		rsdebugElnF("NTP server not reachable");
	}
	else if (_ntpEvent == invalidAddress)
	{
		rsdebugElnF("Invalid NTP server address");
	}
	else if (_ntpEvent == errorSending)
	{
		rsdebugElnF("Error sending request");
	}
	else if (_ntpEvent == responseError)
	{
		rsdebugElnF("NTP response error");
	}
}

void init_NTP(void)
{
	NTP.onNTPSyncEvent([](NTPSyncEvent_t event)
					   {
						   ntpEvent = event;
						   syncEventTriggered = true; });
	// rsdebugDlnF("OK");
}

void init_NTP_with_WiFi(void)
{
	rsdebugInflnF("StartNTP");
	// bool NowLoad =
	LoadInMemorySettingsNTP();

	// rsdebugDln("T_syncNTP: %d", g_p_NTP_settings_ROM->T_syncNTP);
	// rsdebugDln("timezone: %d", g_p_NTP_settings_ROM->timezone);
	// rsdebugDln("serversNTP[%d]: %s", NumServerNTP, g_p_NTP_settings_ROM->serversNTP[NumServerNTP]);

	NTP.setInterval(g_p_NTP_settings_ROM->T_syncNTP); // в секундах период синхронизации
	NTP.setNTPTimeout(NTP_TIMEOUT);					  // в миллисекундах таймаут ответа сервера NTP
	NTP.begin(g_p_NTP_settings_ROM->serversNTP[NumServerNTP], g_p_NTP_settings_ROM->timezone, false /*переход на летнее/зимнее*/, minutesTimeZone);
	// uint32_t Ttask_NTP = g_p_NTP_settings_ROM->Ttask;
	// if (Ttask_NTP)
	// {
	ut_NTP.setInterval(g_p_NTP_settings_ROM->Ttask);
	ut_NTP.enable();
	// }
	// else
	// {
	// 	ut_NTP.disable();
	// }
	// rsdebuglnF("OK");
	// if (NowLoad)
	// 	EmptyMemorySettingsNTP();
}

void cb_ut_NTP(void)
{
	if (syncEventTriggered)
	{
		processSyncEvent(ntpEvent);
		syncEventTriggered = false;
	}
}
