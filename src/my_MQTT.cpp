#include "my_MQTT.h"
#include "main.h"
#include "my_wifi.h"
#include "my_debuglog.h"
#include "my_scheduler.h"
#include "CirQueue.h"
#include "my_MQTT_sub.h"
#include "my_MQTT_pub.h"
#include "my_EEPROM.h"

#ifdef MQTT_QUEUE
#define size_Queue_MQTT 50 // Максимум 255
// s_element_MQTT *p_element_MQTT;
CirQueue p_Queue_MQTT(size_Queue_MQTT, sizeof(s_element_MQTT));
#else
// char last_topic[80];
// char last_payload[24];
#endif

WiFiClient net;
AsyncMqttClient client;
// uTask *p_queueMQTTsub = NULL;
e_state_MQTT MQTT_state;      // Состояние подключения к MQTT
uint16_t mqtt_count_conn = 0; // количество (пере)подключений к серверу mqtt
// #define mqtt_count_conn_reset 200

uint8_t mqtt_init(void)
{
#if defined(EEPROM_C)
  LoadInMemorySettingsEthernet();
#elif defined(EEPROM_CPP)
#endif
  // rsdebugInflnF("---MQTTinit");
  uint8_t _idx = get_idx_eth(WiFi.SSID());
  if (_idx)
  {
    // mqtt_init(_idx);
    // for (uint8_t aaa = 0; aaa < 5; aaa++)
    //   rsdebugInfln("------%s %d", g_p_ethernet_settings_ROM->settings_serv[aaa].MQTTip, aaa);
    if (mqtt_count_conn == 0)
    { // нужно делать только один раз, иначе калбеки вызываются по нескольку раз
      mqtt_count_conn++;
      client.onMessage(onMessageReceived);
      // client.onPublish(onMessagePublish); // для Led - лучше что-нибудь другое
      client.onConnect(onMqttConnect);
      client.onDisconnect(onMqttDisconnect);
    }
    // else if (mqtt_count_conn == mqtt_count_conn_reset)
    //   ESP.restart();

#if defined(EEPROM_C)
    client.setServer(g_p_ethernet_settings_ROM->settings_serv[_idx].MQTTip, 1883);
    client.setCredentials(g_p_ethernet_settings_ROM->MQTT_user, g_p_ethernet_settings_ROM->MQTT_pass);
#elif defined(EEPROM_CPP)
    client.setServer(ram_data.p_NET_settings()->settings_serv[_idx - 1].MQTTip, 1883);
    client.setCredentials(ram_data.p_NET_settings()->MQTT_user, ram_data.p_NET_settings()->MQTT_pass);
#endif
    client.setClientId(OTA_NAME);
    // client.setCleanSession(true);
  }
  else
  {
    rsdebugInfF("Нет адреса MQTT сервера");
  }
  return _idx;
}

void cb_ut_MQTT()
{
  // rsdebugDnfln("in_mqtt wifi_state[%d]", wifi_state);
  if (wifi_state == _wifi_connected)
  {
    // rsdebugDnfln("client.connected[%s]", client.connected() ? "1" : "0");
    if (!client.connected())
    {
      // rsdebugDnfln("MQTT_state[%d]", MQTT_state);
      if (MQTT_state == _MQTT_on_disconnected)
      {
        rsdebugWnflnF("Отключились от MQTT");
        // MQTTdisconnected();
        ut_MQTT.forceNextIteration();
      }
      else if (MQTT_state == _MQTT_disconnected)
      {
#if defined(EEPROM_C)
        LoadInMemorySettingsEthernet();
#elif defined(EEPROM_CPP)
#endif
        uint8_t _idx = mqtt_init();
        if (_idx)
        {
          rsdebugInfF("Подключаемся к MQTT серверу");
#if defined(EEPROM_C)
          rsdebugInfln("[%d]:%s", _idx, g_p_ethernet_settings_ROM->settings_serv[_idx].MQTTip);
#elif defined(EEPROM_CPP)
          rsdebugInfln("[%d]:%s", _idx, ram_data.p_NET_settings()->settings_serv[_idx - 1].MQTTip);
#endif
          MQTT_state = _MQTT_connecting;
          // ut_MQTT.forceNextIteration();
          ut_MQTT.setInterval(1003);
        }
      }
      else if (MQTT_state == _MQTT_connecting)
      {
        mqtt_init();
        rsdebugnfF(".");
        client.connect();
      }
    }
    else if (MQTT_state == _MQTT_on_connected)
    {
      MQTTconnected();
      MQTT_state = _MQTT_connected;
    }
#ifdef MQTT_QUEUE
    else
    {
      // rsdebugDnfF("O");
      // rsdebugDnfln("[%d]", p_Queue_MQTT.isCount());
      if (!p_Queue_MQTT.isEmpty())
      {
        // rsdebugDnfln("Обработка очереди[%d]", p_Queue_MQTT.isCount());
        s_element_MQTT in_element;
        p_Queue_MQTT.pop((uint8_t *)&in_element);
        String topic_DB;
        String topic_in = in_element.topic;
        String payload_in = in_element.payload;
        rsdebugDnfln("[%d]O: %s[%s]", p_Queue_MQTT.isCount(), in_element.topic, in_element.payload);
        // uint8_t i;
        // uint8_t n = sizeof(_arr_SubscribeElement) / sizeof(_arr_SubscribeElement[0]);
        for (uint8_t i = 0; i < sizeof(_arr_SubscribeElement) / sizeof(_arr_SubscribeElement[0]); i++)
        {
          // if ((_arr_SubscribeElement[i].IDVarTopic == v_all) && (topic_in.length() >= topic_DB.length() - 2))
          if (_arr_SubscribeElement[i].IDVarTopic == v_all)
          {
            rsdebugDnfF("^");
            topic_DB = CreateTopic((e_IDDirTopic *)_arr_SubscribeElement[i].IDDirTopic, v_empty);
            topic_in.remove(topic_DB.length());
            // rsdebugDln("topic_in=%s, topic=%s, topic_DB=%s", topic_in.c_str(), topic.c_str(), topic_DB.c_str());
            // rsdebugDln("i=%d, s1=%s, s2=%s", i, topic_in.c_str(), topic_DB.c_str());
          }
          else
          {
            topic_DB = CreateTopic((e_IDDirTopic *)_arr_SubscribeElement[i].IDDirTopic, _arr_SubscribeElement[i].IDVarTopic);
          }
          rsdebugDnfln("topic_in=%s[%d]", topic_in.c_str(), topic_in.length());
          rsdebugDnfln("topic_DB=%s[%d]", topic_DB.c_str(), topic_DB.length());

          if (topic_in.length() == topic_DB.length())
            if (topic_in == topic_DB)
            {
              // rsdebugDlnF("==:break");
              rsdebugDnfF("*");
              _arr_SubscribeElement[i].Callback(in_element);
              rsdebugDnfF("*");
              break;
            }
        }
        rsdebugDnfF(">");
        delete in_element.payload;
        delete in_element.topic;
        // delete in_element;
        rsdebugDnflnF(">");
      }
      if (!p_Queue_MQTT.isEmpty())
        ut_MQTT.forceNextIteration();
    }
#endif
  }
  else
    rsdebugElnF("Не подключены к WiFi"); // не должны увидеть
}

void StartMqtt()
{
  rsdebugInflnF("---StartMqtt");
  MQTT_state = _MQTT_disconnected;
  // rsdebugDnflnF("StartMqtt@1");
  ut_MQTT.forceNextIteration();
  // rsdebugDnflnF("StartMqtt@2");
  ut_MQTT.enable();
  // rsdebugDnflnF("StartMqtt@3");
}

void StopMqtt()
{
  rsdebugInflnF("---StopMqtt");
  // client.disconnect();
  ut_MQTT.disable();
}

// подключились к MQTT
void MQTTconnected(void)
{
#ifdef MQTT_QUEUE
#else
  last_topic[0] = 0;
  last_payload[0] = 0;
#endif
  // rsdebugInflnF("MQTTconnected");
  rsdebugInfF(" -подключились к MQTT#");
  rsdebugInfln("%d", mqtt_count_conn);
  mqtt_count_conn++;
  // публикуем при подключении
  MQTT_pub_allInfo(wifi_count_conn == 1);
  // подписываемся на все из массива _arr_SubscribeElement
  rsdebugInflnF("Подписываемся:");
  MQTT_sub_All();
  // запускаем задачи переодической публикации
  // запускаем "задачи" публикации по событию
}

// void MQTTdisconnected(void) // отключились от MQTT
// {
//   rsdebugInflnF("MQTTdisconnected");
//   // останавливаем "задачи" публикации по событию
//   // останавливаем задачи переодической публикации
//   // отписываемся ? - не надо при отключенном сервере MQTT
//   // client.unsubscribe("Water/info/CPU_load");
// }

void onMqttConnect(bool sessionPresent)
{
  // rsdebugDnfln("onMqttConnect[%d]", (uint8_t)sessionPresent);
#if defined(EEPROM_C)
  LoadInMemorySettingsEthernet();
  ut_MQTT.setInterval(g_p_ethernet_settings_ROM->MQTT_Ttask);
#elif defined(EEPROM_CPP)
  ut_MQTT.setInterval(ram_data.p_NET_settings()->MQTT_Ttask);
#endif
  ut_MQTT.forceNextIteration();
  MQTT_state = _MQTT_on_connected;
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  if (MQTT_state != _MQTT_connecting)
  {
    // rsdebugDnfln("onMqttDisconnect[%d]", (uint8_t)reason);
    // ut_MQTT.forceNextIteration();
    // MQTT_state = _MQTT_on_disconnected;
    MQTT_state = _MQTT_disconnected;
  }
  // }
}

String CreateTopic(o_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic)
{
  String result = ""; // "mqtt/0/";
  for (uint8_t i = 0; i < _IDDirTopic->_len_IDDirTopic; i++)
  {
    result.concat(ArrDirTopic[_IDDirTopic->_IDDirTopic[i]]);
    if (_IDDirTopic->_IDDirTopic[i] != 0)
      result.concat("/");
  }
  result.concat(ArrVarTopic[_IDVarTopic]);
  return result;
}
// String CreateTopic(e_IDDirTopic *_IDDirTopic, String _VarTopic)
// {
//   String result = "";
//   for (uint8_t i = 0; i < MQTT_MAX_LEVEL; i++)
//   {
//     result.concat(ArrDirTopic[_IDDirTopic[i]]);
//     if (_IDDirTopic[i] != 0)
//       result.concat("/");
//   }
//   result.concat(_VarTopic);
//   return result;
// }
// // задача переодической публикации
// void t_publicMQTT_cb(void)
// {
//   rsdebugInflnF("t_publicMQTT_cb");
// }
