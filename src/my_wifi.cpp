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

u_combi_state_WiFi u;
uint16_t wifi_count_conn; // количество (пере)подключений к WiFi

s_ethernet_settings_ROM *g_p_ethernet_settings_ROM = NULL;

uint8_t akk_k = 0;

inline void dependent_tasks_enable() // WiFi:Запускаем зависимые задачи
{
    init_rdebuglog();
    rsdebugnflnF("WiFi connected: Запускаем зависимые задачи");
    // rsdebugInflnF("--RSDebuglog enable");
    rsdebugInflnF("--OTA enable");
    ut_OTA.enable(); // всегда работает когда есть WiFi
    rsdebugInflnF("--NTP enable");
    init_NTP_with_WiFi();    // t_NTP.enable() - внутри
    rsdebugInflnF("--MQTT enable");
    mqtt_init(akk_k /*g_p_ethernet_settings_ROM->settings_serv[k].MQTTip, g_p_ethernet_settings_ROM->MQTT_user, g_p_ethernet_settings_ROM->MQTT_pass*/);
    StartMqtt();
}

inline void dependent_tasks_disable() // WiFi:Останавливаем зависимые задачи
{
    rsdebugnflnF("WiFi disconnected: Останавливаем зависимые задачи");
    rsdebugInflnF("--MQTT disable");
    StopMqtt();
    rsdebugInflnF("--NTP disable");
    ut_NTP.disable();
    rsdebugInflnF("--OTA disable");
    ut_OTA.disable();

    // rsdebugInflnF("--RSDebuglog disable");
}

/* ------------------------------------------------- */
void onStaModeGotIP(WiFiEvent_t event) // подключились и получили IP
{
    // rsdebugDnfln("***1 %d", g_p_ethernet_settings_ROM);
    u.state_WiFi = _wifi_gotIP;
    // all_param_RAM.    // uint8_t nowIP[4]; // зачем IP держать в памяти? есть WiFi.localIP()
    rsdebugnf("\n");
    rsdebugInfln("enent:Получили IP: %s", WiFi.localIP().toString().c_str());
}

void onStaModeDisconnected(WiFiEvent_t event) // произошло отключение
{
    // rsdebugDnfln("***2 %d", g_p_ethernet_settings_ROM);
    // if (u.state_WiFi == _wifi_connected)
    // {
    // WiFi.disconnect();
    if (!Debug.isSdebugEnabled())
        Debug.setSdebugEnabled(true);
    rsdebugnf("\n");
    rsdebugInfln("event:Дисконнект, из состояния: %d", u.state_WiFi);
    if (u.state_WiFi == _wifi_connected || u.state_WiFi == _wifi_connecting || u.state_WiFi == _wifi_gotIP)
    {
        u.state_WiFi = _wifi_disconnected;
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
    // u.state_WiFi = _wifi_disconnected; // надо?
    // yield();
    // t_wifi.setInterval(100);
    ut_wifi.enable(); // включаем задачу t_wifi
    // rsdebugDlnF("OK");
}

void cb_state_wifi(void)
{
    // s_all_settings_ROM def_all_settings_ROM; // параметры по умолчанию - на случай если нет в ROM
    // deb_my_wifi("def_conn_param_ROM timezone: ");
    // deb_my_wifiln(def_conn_param_ROM.data.timezone);
    int16_t i, n;
    switch (u.state_WiFi)
    {
    case _wifi_disconnected:
        // rsdebugDnfln("***3 %d", g_p_ethernet_settings_ROM);
        dependent_tasks_disable(); // WiFi:Останавливаем зависимые задачи
                                   // p_conn_param = (s_conn_param_ROM *)malloc(sizeof(s_conn_param_ROM)); // после коннекта удалить
        u.state_WiFi = _wifi_startfindAP;
        // rsdebugDnfln("***4 %d", g_p_ethernet_settings_ROM);
        break; // дадим поработать другим задачам
    case _wifi_startfindAP:
        WiFi.disconnect(); // - должно произойти событие?! - не происходит!
        // rsdebugDnfln("***5 %d", g_p_ethernet_settings_ROM);
        rsdebugInflnF("Сканируем WiFi-сети");
        WiFi.scanNetworks(true, true);

        u.state_WiFi = _wifi_findAP;
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
            u.state_WiFi = _wifi_findAP;
            break;
        }
        if (n == 0)
        {
            rsdebugInflnF("Нет сетей");
            ut_wifi.suspendNextCall(5000);
            u.state_WiFi = _wifi_startfindAP;
            break;
        }
        if (n > 0)
        {
            LoadInMemorySettingsEthernet(/* f_WIFI */);
            // rsdebugDnfln("***6 %d", g_p_ethernet_settings_ROM);
            // rsdebugDnfln("***7 %d", g_p_ethernet_settings_ROM);
            rsdebugnf("\n");
            rsdebugInfln("Найдено сетей: %d", n);
            for (i = 0; i < n; i++)
            {
                rsdebugInfln("-- %s", WiFi.SSID(i).c_str());
            }
            for (akk_k = 0; akk_k < sizeof(g_p_ethernet_settings_ROM->settings_serv) / sizeof(g_p_ethernet_settings_ROM->settings_serv[0]); akk_k++)
            {
                for (i = 0; i < n; i++) // ищем наши
                {
                    if (WiFi.SSID(i) == g_p_ethernet_settings_ROM->settings_serv[akk_k].SSID)
                    {
                        // нашли надо подключаться
                        // memcpy(&p_param_serv, &p_conn_param.data.param_serv[k], sizeof(s_param_serv));
                        WiFi.scanDelete();
                        u.state_WiFi = _wifi_startconnecting;
                    }
                    if (u.state_WiFi == _wifi_startconnecting)
                        break;
                }
                if (u.state_WiFi == _wifi_startconnecting)
                    break;
            }
            // если нет наших сетей
            if (u.state_WiFi != _wifi_startconnecting)
            {
                rsdebugInflnF("Нет наших сетей :-(");
                WiFi.scanDelete();
                ut_wifi.suspendNextCall(10000);
                u.state_WiFi = _wifi_startfindAP;
            }
        }
        if (u.state_WiFi != _wifi_startconnecting)
            break;
    case _wifi_startconnecting:
        LoadInMemorySettingsEthernet();
        // rsdebugDnfln("***8 %d", g_p_ethernet_settings_ROM);
        // WiFi.mode(WIFI_STA);
        rsdebugInfln("Соединяемся с WiFi-сетью: %s, %d", g_p_ethernet_settings_ROM->settings_serv[akk_k].SSID, akk_k);
        WiFi.begin(g_p_ethernet_settings_ROM->settings_serv[akk_k].SSID, g_p_ethernet_settings_ROM->settings_serv[akk_k].PASS);
        u.state_WiFi = _wifi_connecting;
        // uTask *uTimerTimeout = new uTask(
        //     15000, []() {
        //         rsdebugEnflnF("\n_wifi_connecting --- timeout");
        //         u.state_WiFi = _wifi_startfindAP;
        //     },
        //     true);
        u.counter = millis(); // (uint8)(t_wifi.getRunCounter() + 30);
        // rsdebugDnfln("\n1*****************u.counter: %d", u.counter);
        // rsdebugDnfln("***9 %d", g_p_ethernet_settings_ROM);
        break;
    default:
    case _wifi_connecting:
        // rsdebugDnfln("***10 %d", g_p_ethernet_settings_ROM);
        rsdebugnfF(".");
        // rsdebugDnfln("\n2*****************t_wifi.getRunCounter()+30: %d", t_wifi.getRunCounter() + 30);
        // rsdebugDnfln("\n3*****************u.counter: %d", u.counter);
        // rsdebugDnfln("\n4*****************2-3 > (2 * 15): %d", (t_wifi.getRunCounter() + 30) - u.counter);
        if ((/* (t_wifi.getRunCounter() + 30) */ millis() - u.counter) > (2 * 15000)) // 15 секунд
        {
            rsdebugEnflnF("\n_wifi_connecting --- timeout");
            u.state_WiFi = _wifi_startfindAP;
        }
        // rsdebugDnf("%d", t_wifi.getRunCounter());
        // rsdebugDnfln("***11 %d", g_p_ethernet_settings_ROM);
        break;
    case _wifi_gotIP:
        // rsdebugDnfln("***12 %d", g_p_ethernet_settings_ROM);
        // rsdebugDnfln("***12.5 %d", g_p_ethernet_settings_ROM);
        u.state_WiFi = _wifi_connected;
        wifi_count_conn++;
        // t_wifi_connect.disable(); // выключаем (эту) задачу t_wifi_connect
        // rsdebugDnflnF("*************************** 1");
        // rsdebugDnfln("***13 %d", g_p_ethernet_settings_ROM);
        // LoadInMemorySettingsSys();
        ut_wifi.setInterval(g_p_ethernet_settings_ROM->WiFi_Ttask);
        // rsdebugDnflnF("*************************** 2");
        // rsdebugDnfln("***14 %d", g_p_ethernet_settings_ROM);
        dependent_tasks_enable(); // WiFi:Запускаем зависимые задачи
                                  // rsdebugDnflnF("*************************** 3");
                                  // rsdebugDnfln("***15 %d", g_p_ethernet_settings_ROM);
        // EmptyMemorySettingsEthernet();
        // rsdebugDnflnF("*************************** 4");
        // rsdebugDnfln("***16 %d", g_p_ethernet_settings_ROM);
        // rsdebugDln("****************************Hostname: %s", wifi_station_get_hostname());
        // g_p_ethernet_settings_ROM = NULL;
        break;
    case _wifi_connected:
        // здесь выполняем uTask задачи зависящие от подключения к WiFi
        // rsdebugDnflnF("*************************** 5");
        // ut_OTA.execute();
        // rsdebugDnflnF("*************************** 6");
        // ut_NTP.execute();
        // deb_my_wifi(F("WiFi: _wifi_connected - этого мы не должны увидеть никогда !!!"));
        break;
    }
}
