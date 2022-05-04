#include <Arduino.h>
#include <ESP8266WiFi.h>

#include "my_wifi.h"
#include "my_scheduler.h"
#include "my_EEPROM.h"
#include "my_debuglog.h"
#include "main.h"
// #include "my_NTP.h"
#include "my_MQTT.h"

// #define PORTAL_TIMEOUT (1 * 60) // seconds
// #define AP_NAME "MY CAPTIVE PORTAL"
// #define AP_PASSWORD ""

// u_data_WiFi data_WiFi;
uint16_t wifi_count_conn; // количество (пере)подключений к WiFi
e_state_WiFi wifi_state;  // Состояние WIFi
// uint8_t idx_eth_rom = 0; // номер записи в g_p_ethernet_settings_ROM - для получения адреса сервера MQTT

#if defined(EEPROM_C)
s_ethernet_settings_ROM *g_p_ethernet_settings_ROM = NULL;
#elif defined(EEPROM_CPP)
#endif

inline void dependent_tasks_enable() // WiFi:Запускаем зависимые задачи
{
    rsdebugnflnF("-Запускаем задачи зависимые от WiFi");
    rsdebugInflnF("--RDebuglog enable");
    init_rdebuglog();
    rsdebugInflnF("--OTA enable");
    ut_OTA.enable(); // всегда работает когда есть WiFi
    rsdebugInflnF("--NTP enable");
    init_NTP_with_WiFi(); // t_NTP.enable() - внутри
    rsdebugInflnF("--MQTT enable");
    // mqtt_init();
    StartMqtt();
}

inline void dependent_tasks_disable() // WiFi:Останавливаем зависимые задачи
{
    rsdebugnflnF("-Останавливаем задачи зависимые от WiFi");
    rsdebugInflnF("--MQTT disable");
    StopMqtt();
    rsdebugInflnF("--NTP disable");
    ut_NTP.disable();
    rsdebugInflnF("--OTA disable");
    ut_OTA.disable();
    rsdebugInflnF("--RSDebuglog disable");
    ut_debuglog.disable();
}

/* ------------------------------------------------- */
void onStaModeGotIP(WiFiEvent_t event) // подключились и получили IP
{
    // rsdebugDnfln("***1 %d", g_p_ethernet_settings_ROM);
    wifi_state = _wifi_gotIP;
    // all_param_RAM.    // uint8_t nowIP[4]; // зачем IP держать в памяти? есть WiFi.localIP()
    rsdebugnf("\n");
    rsdebugInfF("Получили IP WiFi: ");
    rsdebugInfln("%s", WiFi.localIP().toString().c_str());
}

void onStaModeDisconnected(WiFiEvent_t event) // произошло отключение
{
    // rsdebugDnfln("***2 %d", g_p_ethernet_settings_ROM);
    // if (wifi_state == _wifi_connected)
    // {
    // WiFi.disconnect();
    if (!Debug.isSdebugEnabled())
        Debug.setSdebugEnabled(true);
    rsdebugnf("\n");
    rsdebugInfF("event:Дисконнект, из состояния: ");
    rsdebugInfln("%d", wifi_state);
    if (wifi_state == _wifi_connected || wifi_state == _wifi_connecting || wifi_state == _wifi_gotIP)
    {
        wifi_state = _wifi_disconnected;
        // t_wifi.setInterval(200);
        // t_wifi_connect.enable(); // включаем задачу t_wifi_connect
    }
    // }
}

void init_WiFi()
{
    WiFi.disconnect(); // - должно произойти событие?! - не происходит!
    wifi_count_conn = 0;
    if (WiFi.getPersistent())
        WiFi.persistent(false); // чтоб не записывать во flash(и не протирать) параметры WiFi при WiFi.begin
    if (WiFi.getAutoConnect())
        WiFi.setAutoConnect(false); // надо разобраться с механизмом
    if (WiFi.getAutoReconnect())
        WiFi.setAutoReconnect(false); // надо разобраться с механизмом, попробую, может перестанет терять сеть-не перестал
    WiFi.mode(WIFI_STA);
    WiFi.onEvent(onStaModeGotIP, WIFI_EVENT_STAMODE_GOT_IP);              // - регистрируем событие: получение IP
    WiFi.onEvent(onStaModeDisconnected, WIFI_EVENT_STAMODE_DISCONNECTED); // - регистрируем событие: дисконнект
    WiFi.hostname(OTA_NAME);
    // wifi_state = _wifi_disconnected; // надо?
    // yield();
    // t_wifi.setInterval(100);
    ut_wifi.enable(); // включаем задачу t_wifi
    // rsdebugDlnF("OK");
}

uint8_t get_idx_eth(String str_ssid) // 0 - не нашли, idx_eth_rom = +1
{
#if defined(EEPROM_C)
    for (uint8_t _idx = 0; _idx < sizeof(g_p_ethernet_settings_ROM->settings_serv) / sizeof(g_p_ethernet_settings_ROM->settings_serv[0]); _idx++)
    {
        if (str_ssid == g_p_ethernet_settings_ROM->settings_serv[_idx].SSID)
        {
            // нашли надо подключаться
            return _idx + 1;
        }
    }
    return 0;
#elif defined(EEPROM_CPP)
    for (uint8_t _idx = 0; _idx < sizeof(ram_data.p_NET_settings()->settings_serv) / sizeof(ram_data.p_NET_settings()->settings_serv[0]); _idx++)
    {
        if (str_ssid == ram_data.p_NET_settings()->settings_serv[_idx].SSID)
        {
            // нашли надо подключаться
            return _idx + 1;
        }
    }
    return 0;
#endif
}

void cb_ut_state_wifi(void)
{
    static uint32_t _stopwatch;
    // rsdebugDnfln("---***cb_state_wifi %d---%d", wifi_state, _stopwatch);
    // s_all_settings_ROM def_all_settings_ROM; // параметры по умолчанию - на случай если нет в ROM
    // deb_my_wifi("def_conn_param_ROM timezone: ");
    // deb_my_wifiln(def_conn_param_ROM.data.timezone);
    int16_t i, n;
    switch (wifi_state)
    {
    case _wifi_disconnected:
        // rsdebugDnfln("***3 %d", g_p_ethernet_settings_ROM);
        dependent_tasks_disable(); // WiFi:Останавливаем зависимые задачи
                                   // p_conn_param = (s_conn_param_ROM *)malloc(sizeof(s_conn_param_ROM)); // после коннекта удалить
        wifi_state = _wifi_startfindAP;
        // rsdebugDnfln("***4 %d", g_p_ethernet_settings_ROM);
        break; // дадим поработать другим задачам
    case _wifi_startfindAP:
        WiFi.disconnect(); // - должно произойти событие?! - не происходит!
        // rsdebugDnfln("***5 %d", g_p_ethernet_settings_ROM);
        rsdebugInflnF("Сканируем WiFi-сети");
        WiFi.scanNetworks(true, true);

        wifi_state = _wifi_findAP;
        ut_wifi.setInterval(500);
        break; // дадим поработать другим задачам
    case _wifi_findAP:
        n = WiFi.scanComplete();
        if (n == -1)
        {
            rsdebugnfF(".");
            // yield(); // ???
            break;
        }
        if (n == -2)
        {
            wifi_state = _wifi_findAP;
            break;
        }
        if (n == 0)
        {
            rsdebugInflnF("Нет сетей");
            ut_wifi.suspendNextCall(5000);
            wifi_state = _wifi_startfindAP;
            break;
        }
        if (n > 0)
        {
            // LoadInMemorySettingsEthernet();
            // rsdebugDnfln("***6 %d", g_p_ethernet_settings_ROM);
            // rsdebugDnfln("***7 %d", g_p_ethernet_settings_ROM);
            rsdebugnf("\n");
            rsdebugInfF("Найдено сетей: ");
            rsdebugInfln("%d", n);
            for (i = 0; i < n; i++)
            {
                rsdebugInfln("-- %s", WiFi.SSID(i).c_str());
            }
            for (i = 0; i < n; i++) // ищем наши
            {
                LoadInMemorySettingsEthernet();
                uint8_t _idx = get_idx_eth(WiFi.SSID(i));
                if (_idx)
                {
                    _idx--;
                    // нашли надо подключаться
                    // rsdebugDnfln("***8 %d", g_p_ethernet_settings_ROM);
                    // WiFi.mode(WIFI_STA);
                    rsdebugInfF("Соединяемся с WiFi");
#if defined(EEPROM_C)
                    rsdebugInfln("[%d]: %s", _idx, g_p_ethernet_settings_ROM->settings_serv[_idx].SSID);
                    WiFi.begin(g_p_ethernet_settings_ROM->settings_serv[_idx].SSID, g_p_ethernet_settings_ROM->settings_serv[_idx].PASS);
#elif defined(EEPROM_CPP)
                    rsdebugInfln("[%d]: %s", _idx, ram_data.p_NET_settings()->settings_serv[_idx].SSID);
                    WiFi.begin(ram_data.p_NET_settings()->settings_serv[_idx].SSID, ram_data.p_NET_settings()->settings_serv[_idx].PASS);
#endif
                    wifi_state = _wifi_connecting;
                    _stopwatch = millis();
                    break;
                }
            }
            // если нет наших сетей
            WiFi.scanDelete();
            if (wifi_state != _wifi_connecting)
            {
                rsdebugInflnF("Нет наших сетей :-(");
                ut_wifi.suspendNextCall(10000);
                wifi_state = _wifi_startfindAP;
            }
        }
        break;
    default:
    case _wifi_connecting:
        // rsdebugDnfln("***10 %d", g_p_ethernet_settings_ROM);
        rsdebugnfF(".");
        // rsdebugDnfln("\n2*****************t_wifi.getRunCounter()+30: %d", t_wifi.getRunCounter() + 30);
        // rsdebugDnfln("\n3*****************_stopwatch: %d", _stopwatch);
        // rsdebugDnfln("\n4*****************2-3 > (2 * 15): %d", (t_wifi.getRunCounter() + 30) - _stopwatch);
        if ((/* (t_wifi.getRunCounter() + 30) */ millis() - _stopwatch) > (2 * 15000)) // 15 секунд
        {
            rsdebugEnfln("\n_wifi_connecting --- timeout $millis[%d] $_stopwatch[%d]", (uint32_t)millis(), _stopwatch);
            wifi_state = _wifi_startfindAP;
        }
        break;
    case _wifi_gotIP:
        // rsdebugDnfln("***12 %d", g_p_ethernet_settings_ROM);
        // rsdebugDnfln("***12.5 %d", g_p_ethernet_settings_ROM);
        wifi_state = _wifi_connected;
        wifi_count_conn++;
        // rsdebugDnflnF("*************************** 1");
        // rsdebugDnfln("***13 %d", g_p_ethernet_settings_ROM);
        // LoadInMemorySettingsSys();
#if defined(EEPROM_C)
        ut_wifi.setInterval(g_p_ethernet_settings_ROM->WiFi_Ttask);
#elif defined(EEPROM_CPP)
        ut_wifi.setInterval(ram_data.p_NET_settings()->WiFi_Ttask);
#endif
        // rsdebugDnflnF("*************************** 2");
        // rsdebugDnfln("***14 %d", g_p_ethernet_settings_ROM);
        dependent_tasks_enable(); // WiFi:Запускаем зависимые задачи
        break;
    case _wifi_connected:
        // здесь выполняем uTask задачи зависящие от подключения к WiFi
        // rsdebugDnflnF("*************************** 5");
        // ut_OTA.execute(); // вынес в main
        // rsdebugDnflnF("*************************** 6");
        // ut_NTP.execute(); // вынес в main
        // deb_my_wifi(F("WiFi: _wifi_connected - этого мы не должны увидеть никогда !!!"));
        break;
    }
}
