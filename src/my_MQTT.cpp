#include "my_MQTT.h"
#include "main.h"
#include "my_wifi.h"
#include "my_debuglog.h"
#include "my_scheduler.h"
#include "CirQueue.h"
#include "MQTT_com.h"
#include "MQTT_pub.h"
#include "my_EEPROM.h"

typedef void (*t_Callback_char)(s_element_Queue_MQTT);

#define size_IDDirTopic 4
struct s_SubscribeTopicsCallback
{
  e_IDDirTopic IDDirTopic[size_IDDirTopic];
  e_IDVarTopic IDVarTopic;
  uint8 mqttQOS;
  t_Callback_char Callback;
};

#define size_Queue_MQTT 50 // Максимум 255
CirQueue *p_Queue_MQTT = NULL;

const s_SubscribeTopicsCallback SubscribeTopicsCallback[] = {
    /* _NTC */
    // {{_main_topic, _Devices, _NTC, _Settings}, _all, 0, &MQTT_com_NTC_Settings},

    // {{_main_topic, _Settings, _RSdebug, _empty}, _all, 0, &MQTT_com_Settings}, // для создания структуры CommandsDebug - read
    // {{_main_topic, _Settings, _SysMon, _empty}, _all, 0, &MQTT_com_Settings},  // для создания структуры CommandsDebug - read
    // {{_main_topic, _Settings, _OTA, _empty}, _all, 0, &MQTT_com_Settings},     // для создания структуры CommandsDebug - read
    // {{_main_topic, _Settings, _NTP, _empty}, _all, 0, &MQTT_com_Settings},     // для создания структуры CommandsDebug - read
    // {{_main_topic, _Settings, _MQTT, _empty}, _all, 0, &MQTT_com_Settings},    // для создания структуры CommandsDebug - read
    // {{_main_topic, _Settings, _WIFI, _empty}, _all, 0, &MQTT_com_Settings},    // для создания структуры CommandsDebug - read
#ifdef USER_AREA
    // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
    {{_main_topic, _Devices, _Door, _Commands}, _Latch, 0, &MQTT_com_Door},
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
    {{_main_topic, _Settings, _empty, _empty}, _all, 0, &MQTT_com_Settings}};

WiFiClient net;
AsyncMqttClient client;
uTask *p_queueMQTTsub = NULL;

e_state_MQTT state_MQTT;      // Состояние подключения к MQTT
uint16_t mqtt_count_conn = 0; // количество (пере)подключений к серверу mqtt
volatile uint8_t g_num_serv;  // Номер сервера
// uint32_t last_ts_mqtt_count_conn;                 // время последнего реконнекта
// #define t_swift_mqtt_count_conn 2000              // время "быстрого" реконнекта
// uint8_t cnt_swift_mqtt_count_conn;                // счетчик "быстрых" реконнектов
// #define cnt_swift_mqtt_count_conn_for_reset 10    // кол-во "быстрых" реконнектов
// #define t_reset_cnt_swift_mqtt_count_conn 3600000 // через сколько сбрасываем счетчик "быстрых" реконнектов

String CreateTopic(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic)
{
  String result = "";
  for (uint8_t i = 0; i < size_IDDirTopic; i++)
  {
    result.concat(ArrDirTopic[_IDDirTopic[i]]);
    if (_IDDirTopic[i] != 0)
      result.concat("/");
  }
  result.concat(ArrVarTopic[_IDVarTopic]);
  return result;
}

// // задача переодической публикации
// void t_publicMQTT_cb(void)
// {
//   rsdebugInflnF("t_publicMQTT_cb");
// }

// подключились к MQTT
void onStartMQTT(void)
{
  // rsdebugInflnF("onStartMQTT");
  mqtt_count_conn++;
  rsdebugInfF(" - подключились к MQTT ");
  rsdebugInfln("%d раз", mqtt_count_conn);
  // создаем очереди входящих сообщений подписок
  if (!p_Queue_MQTT) // если нет указателя
    p_Queue_MQTT = new CirQueue(size_Queue_MQTT, sizeof(s_element_Queue_MQTT));
  if (!p_Queue_MQTT)
    rsdebugElnF("p_Queue_MQTT: нет памяти");

  // публикуем при подключении
  e_IDDirTopic dir_topic[] = {_main_topic, _Info, _empty, _empty};
  station_config config_tmp;
  wifi_station_get_config(&config_tmp);
  mqtt_publish(dir_topic, _CurrentWiFiAP, (const char *)config_tmp.ssid);
  mqtt_publish(dir_topic, _CurrentIP, WiFi.localIP().toString().c_str());
  mqtt_publish(dir_topic, _CntReconnWiFi, (uint32_t)wifi_count_conn);
  mqtt_publish(dir_topic, _CntReconnMQTT, (uint32_t)mqtt_count_conn);
#ifdef USER_AREA
  // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
  onStartMQTT_pub_door();
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA

  // только после сброса
  if (wifi_count_conn == 1)
  {
    mqtt_publish(dir_topic, _ReasonReset, get_glob_reason(false).c_str());
    dir_topic[1] = _Settings;
    mqtt_publish_no(dir_topic, _Default);
    mqtt_publish_no(dir_topic, _Read);
    mqtt_publish_ok(dir_topic, _Save);
    mqtt_publish_ok(dir_topic, _Debug);
#ifdef USER_AREA
    // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
    // MQTT_pub_Commands_ok(_SaveSettings);
    // mqtt_publish(dir_topic, _TimeReset, NTP.getTimeDateString(NTP.getLastBootTime()).c_str());
  }

  // создаем задачу обработки очереди сообщений от подписок
  if (p_queueMQTTsub)
  {
    delete p_queueMQTTsub;
    p_queueMQTTsub = NULL;
  }
  p_queueMQTTsub = new uTask(100, &t_queueMQTTsub_cb, true);

  // подписываемся на все из массива SubscribeTopicsCallback
  // client.setTimeout(1000);
  rsdebugInflnF("Отписываемся/подписываемся:");
  for (uint8 i = 0; i < (sizeof(SubscribeTopicsCallback) / sizeof(SubscribeTopicsCallback[0])); i++)
  {
    String topic = CreateTopic((e_IDDirTopic *)SubscribeTopicsCallback[i].IDDirTopic, (e_IDVarTopic)SubscribeTopicsCallback[i].IDVarTopic);
    if (mqtt_count_conn > 1) // сначала отписываемся если переподключились к MQTT
    {
      rsdebugDnfln("- %s", topic.c_str());
      client.unsubscribe(topic.c_str());
    }
    // client.loop();
    rsdebugDnfln("+ %s", topic.c_str());
    client.subscribe(topic.c_str(), SubscribeTopicsCallback[i].mqttQOS);
  }

  // запускаем задачи переодической публикации

  // запускаем "задачи" публикации по событию
  MQTT_pub_Info_TimeReset();
}

// отключились от MQTT
// void onStopMQTT(void)
// {
//   rsdebugInflnF("onStopMQTT");
//   // if (!t_debuglog.isEnabled())
//   //   t_debuglog.enable();
//   // if (!Debug.isRdebugEnabled())
//   //   Debug.setRdebugEnabled(true);
//   // if (!Debug.isSdebugEnabled())
//   //   Debug.setSdebugEnabled(true);
//   // останавливаем "задачи" публикации по событию

//   // останавливаем задачи переодической публикации

//   // удаляем задачу обработки очереди сообщений от подписок
//   if (p_queueMQTTsub)
//   {
//     delete p_queueMQTTsub;
//     p_queueMQTTsub = NULL;
//   }

//   // отписываемся ? - не надо при отключенном сервере MQTT
//   // client.unsubscribe("Water/info/CPU_load");

//   // // удаляем очереди входящих сообщений подписок
//   // // очередь "номер подписки из массива"
//   // if (p_Queue_nCallback)
//   // {
//   //   delete p_Queue_nCallback;
//   //   p_Queue_nCallback = NULL;
//   // }
//   // // очередь "строка сообщения"
//   // if (p_Queue_StrPayload)
//   // {
//   //   delete p_Queue_StrPayload;
//   //   p_Queue_StrPayload = NULL;
//   // }
//   // очередь "топик - сообщение"
//   if (p_Queue_MQTT)
//   {
//     delete p_Queue_MQTT;
//     p_Queue_MQTT = NULL;
//   }
// }

void StartMqtt()
{
  rsdebugInflnF("StartMqtt");
  state_MQTT = _MQTT_disconnected;
  ut_MQTT.enable();
}

void StopMqtt()
{
  rsdebugInflnF("StopMqtt");
  // onStopMQTT();
  client.disconnect();
  ut_MQTT.disable();
}

// задача обработки очереди сообщений подписок и вызов колбеков
void t_queueMQTTsub_cb(void)
{
  if (!p_Queue_MQTT->isEmpty())
  {
    // rsdebugDnfln("Очередь сообщений=%d", p_Queue_MQTT->isCount());
    s_element_Queue_MQTT tmp_element;
    p_Queue_MQTT->pop((uint8_t *)&tmp_element);
    String topic, topic1, payload1, topic2;
    topic1 = *tmp_element.topic;
    payload1 = *tmp_element.payload;
    uint8_t i;
    uint8_t n = sizeof(SubscribeTopicsCallback) / sizeof(SubscribeTopicsCallback[0]);
    for (i = 0; i < n; i++)
    {
      topic2 = CreateTopic((e_IDDirTopic *)SubscribeTopicsCallback[i].IDDirTopic, SubscribeTopicsCallback[i].IDVarTopic);
      topic = topic1;
      if (SubscribeTopicsCallback[i].IDVarTopic == _all)
      {
        uint16_t pos = topic2.length() - 2;
        topic2.remove(pos);
        topic.remove(pos);
        // rsdebugDln("topic1=%s, topic=%s, topic2=%s", topic1.c_str(), topic.c_str(), topic2.c_str());
      }
      // rsdebugDln("i=%d, s1=%s, s2=%s", i, topic1.c_str(), topic2.c_str());
      if (!strcmp(topic.c_str(), topic2.c_str()))
      {
        // rsdebugDlnF("==:break");
        break;
      }
      else if (i == n - 1)
      {
        // rsdebugDlnF("!=:return");
        return;
      }
    }
    if (!p_Queue_MQTT->isEmpty())
    {
      ut_MQTT.forceNextIteration();
      p_queueMQTTsub->forceNextIteration();
    }
    SubscribeTopicsCallback[i].Callback(tmp_element);
    delete tmp_element.payload;
    delete tmp_element.topic;
  }
}

void messageReceived(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  String *_topic = new String(topic);
  char tmp_payload[len + 1];
  memcpy(tmp_payload, payload, len);
  tmp_payload[len] = 0;
  String *_payload = new String(tmp_payload);
  s_element_Queue_MQTT tmp_element = {_topic, _payload};
  // rsdebugDnfln("[%d]MQTT incoming: %s[%s][%d][%d][%d]", p_Queue_MQTT->isCount(), topic, tmp_payload, len, index, total);
  rsdebugDnfln("[%d]MQTT incoming: %s[%s]", p_Queue_MQTT->isCount(), topic, tmp_payload);
  if (size_Queue_MQTT - p_Queue_MQTT->isCount() <= size_Queue_MQTT / 8)
  {
    t_queueMQTTsub_cb(); // ну на крайний случай
  }
  // rsdebugDnflnF("@1");
  if (p_Queue_MQTT->isFull())
  {
    rsdebugEnflnF("p_Queue_nCallback is full !!!!");
  }
  else
  {
    // rsdebugDnflnF("@2");
    if (!p_Queue_MQTT->isEmpty())
    {
      // rsdebugDnflnF("@3");
      s_element_Queue_MQTT *_element = (s_element_Queue_MQTT *)p_Queue_MQTT->peeklast();
      // rsdebugDnflnF("@31");
      // rsdebugDnfln("[%d]MQTT peeklast: %s[%s]", p_Queue_MQTT->isCount() - 1, _element->topic->c_str(), _element->payload->c_str());
      // rsdebugDnfln("#1[%s][%d][%s][%d]", _element->topic->c_str(), _element->topic->length(), (*tmp_element.topic).c_str(), (*tmp_element.topic).length());
      // rsdebugDnfln("#2[%s][%d][%s][%d]", _element->payload->c_str(), _element->payload->length(), (*tmp_element.payload).c_str(), (*tmp_element.payload).length());
      // rsdebugDnflnF("@32");
      if (!strcmp((_element->topic->c_str()), (*tmp_element.topic).c_str()) && !strcmp((_element->payload->c_str()), (*tmp_element.payload).c_str()))
      {
        rsdebugDnflnF("duplicate !!!!");
        delete _topic;
        delete _payload;
      }
      else
      {
        // rsdebugDnflnF("@3");
        p_Queue_MQTT->push((uint8_t *)&tmp_element);
      }
      // rsdebugDnflnF("@4");
    }
    else
    {
      // rsdebugDnflnF("@5");
      p_Queue_MQTT->push((uint8_t *)&tmp_element);
      // rsdebugDnflnF("@6");
      ut_MQTT.forceNextIteration();
      // rsdebugDnflnF("@7");
      p_queueMQTTsub->forceNextIteration();
      // rsdebugDnflnF("@8");
    }
    // rsdebugDnflnF("@9");
  }
  // rsdebugDnflnF("@10");
}

void mqtt_init(uint8_t num_serv)
{
  LoadInMemorySettingsEthernet();
  // rsdebugInfF("MQTT сервер: ");
  // rsdebugInfln("%d.%d.%d.%d - %d", g_p_ethernet_settings_ROM->settings_serv[num_serv].MQTTip[0],
  //              g_p_ethernet_settings_ROM->settings_serv[num_serv].MQTTip[1],
  //              g_p_ethernet_settings_ROM->settings_serv[num_serv].MQTTip[2],
  //              g_p_ethernet_settings_ROM->settings_serv[num_serv].MQTTip[3],
  //              num_serv);
  // for (uint8_t aaa = 0; aaa < 5; aaa++)
  //   rsdebugInfln("------%s %d", g_p_ethernet_settings_ROM->settings_serv[aaa].MQTTip, aaa);
  client.onMessage(messageReceived);
  client.onConnect(onMqttConnect);
  client.onDisconnect(onMqttDisconnect);
  ut_MQTT.setInterval(g_p_ethernet_settings_ROM->MQTT_Ttask);
  // IPAddress ip_tmp;
  // ip_tmp.fromString(g_p_ethernet_settings_ROM->settings_serv[num_serv].MQTTip);
  // rsdebugInfln("mqtt_init:%s", ip_tmp.toString().c_str());
  // client.setServer(ip_tmp, 1883);
  client.setServer(g_p_ethernet_settings_ROM->settings_serv[num_serv].MQTTip, 1883);
  client.setClientId(OTA_NAME);
  client.setCredentials(g_p_ethernet_settings_ROM->MQTT_user, g_p_ethernet_settings_ROM->MQTT_pass);
  // client.setCleanSession(true);
  g_num_serv = num_serv;
}

void onMqttConnect(bool sessionPresent)
{
  if (state_MQTT != _MQTT_connected)
  {
    LoadInMemorySettingsEthernet();
    ut_MQTT.setInterval(g_p_ethernet_settings_ROM->MQTT_Ttask);
    state_MQTT = _MQTT_on_connected;
  }
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  if (!client.connected() && state_MQTT != _MQTT_disconnected)
  {
    rsdebugInflnF("Отключились от MQTT");
    // onStopMQTT();
    state_MQTT = _MQTT_disconnected;
  }
}

void t_MQTT_cb()
{
  if (u.state_WiFi == _wifi_connected)
  {
    if (!client.connected())
    {
      if (state_MQTT == _MQTT_disconnected)
      {
        LoadInMemorySettingsEthernet();
        rsdebugInfF("Подключаемся к MQTT серверу: ");
        rsdebugInfln("%s[%d]", g_p_ethernet_settings_ROM->settings_serv[g_num_serv].MQTTip, g_num_serv);
        mqtt_init(g_num_serv);
        state_MQTT = _MQTT_connecting;
        ut_MQTT.setInterval(1003);
      }
      if (state_MQTT == _MQTT_connecting)
      {
        rsdebugnfF(".");
        client.connect();
      }
    }
    if (state_MQTT == _MQTT_on_connected)
    {
      onStartMQTT();
      state_MQTT = _MQTT_connected;
    }
    if (p_queueMQTTsub)
      p_queueMQTTsub->execute();
  }
  else
    rsdebugElnF("Не подключены к WiFi"); // не должны увидеть
}

void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, bool _payload)
{
  // if (!client.connected())
  //   return;
  mqtt_publish(_IDDirTopic, _IDVarTopic, _payload ? "yes" : "no");
}

void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, int32_t _payload)
{
  if (!client.connected())
    return;
  char msg[16];
  sprintf(msg, "%d", _payload);
  mqtt_publish(_IDDirTopic, _IDVarTopic, msg);
}

void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, uint32_t _payload)
{
  if (!client.connected())
    return;
  char msg[16];
  sprintf(msg, "%d", _payload);
  mqtt_publish(_IDDirTopic, _IDVarTopic, msg);
}

void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, float _payload, const char *_format)
{
  if (!client.connected())
    return;
  char msg[16];
  sprintf(msg, _format, _payload);
  mqtt_publish(_IDDirTopic, _IDVarTopic, msg);
}

// void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, float payload)
// {
//   if (!client.connected())
//     return;
//   char msg[16];
//   sprintf(msg, "%.2f", payload);
//   mqtt_publish(_IDDirTopic, _IDVarTopic, msg);
// }

void mqtt_publish(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic, const char *_payload)
{
  if (!client.connected())
    return;
  String topic = CreateTopic(_IDDirTopic, _IDVarTopic);
  mqtt_publish(topic.c_str(), _payload);
}

void mqtt_publish(const char *_topic, const char *_payload)
{
  client.publish(_topic, 0, false, _payload);
  rsdebugDnfln("MQTT pub: %s[%s]", _topic, _payload);
}

void mqtt_publish_ok(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic)
{
  mqtt_publish(_IDDirTopic, _IDVarTopic, "ok");
}

void mqtt_publish_no(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic)
{
  mqtt_publish(_IDDirTopic, _IDVarTopic, "no");
}

bool is_equal_ok(const char *char_str)
{
  return (!strcmp(char_str, "ok"));
}

bool is_equal_enable(const char *char_str)
{
  return (!strcmp(char_str, "enable") ||
          !strcmp(char_str, "true") ||
          !strcmp(char_str, "open") ||
          !strcmp(char_str, "yes") ||
          !strcmp(char_str, "1") ||
          !strcmp(char_str, "on"));
}

bool is_equal_disable(const char *char_str)
{
  return (!strcmp(char_str, "disable") ||
          !strcmp(char_str, "false") ||
          !strcmp(char_str, "close") ||
          !strcmp(char_str, "no") ||
          !strcmp(char_str, "0") ||
          !strcmp(char_str, "off"));
}
