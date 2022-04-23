#include "my_MQTT.h"
#include "main.h"
#include "my_wifi.h"
#include "my_debuglog.h"
#include "my_scheduler.h"
#include "CirQueue.h"
#include "MQTT_com.h"
#include "MQTT_pub.h"
#include "my_EEPROM.h"

#ifdef MQTT_QUEUE
#define size_Queue_MQTT 50 // Максимум 255
// s_element_MQTT *p_element_MQTT;
CirQueue p_Queue_MQTT(size_Queue_MQTT, sizeof(s_element_MQTT));
#else
char last_topic[80];
char last_payload[24];
#endif

const s_SubscribeElement _arr_SubscribeElement[] = {
/* _NTC */
// {{d_main_topic, d_devices, _NTC, d_settings}, v_all, 0, &MQTT_com_NTC_Settings},

// {{d_main_topic, d_settings, d_rs_debug, d_empty}, v_all, 0, &MQTT_com_Settings}, // для создания структуры CommandsDebug - read
// {{d_main_topic, d_settings, d_sysmon, d_empty}, v_all, 0, &MQTT_com_Settings},  // для создания структуры CommandsDebug - read
// {{d_main_topic, d_settings, d_ota, d_empty}, v_all, 0, &MQTT_com_Settings},     // для создания структуры CommandsDebug - read
// {{d_main_topic, d_settings, d_ntp, d_empty}, v_all, 0, &MQTT_com_Settings},     // для создания структуры CommandsDebug - read
// {{d_main_topic, d_settings, d_mqtt, d_empty}, v_all, 0, &MQTT_com_Settings},    // для создания структуры CommandsDebug - read
// {{d_main_topic, d_settings, d_wifi, d_empty}, v_all, 0, &MQTT_com_Settings},    // для создания структуры CommandsDebug - read
#ifdef USER_AREA
    // ****!!!!@@@@####$$$$%%%%^^^^USER_AREA_BEGIN
    {{d_main_topic, d_devices, d_door, d_commands}, v_latch, 0, &cb_MQTT_com_Door},
// USER_AREA_END****!!!!@@@@####$$$$%%%%^^^^
#endif // USER_AREA
    // {{d_main_topic, d_settings, d_empty, d_empty}, _Debug, 0, &cb_MQTT_com_Settings},
    {{d_main_topic, d_info, d_empty, d_empty}, vc_Read, 0, &cb_MQTT_com_Info},
    // {{d_main_topic, d_settings, d_empty, d_empty}, _ReadDflt, 0, &cb_MQTT_com_Settings},
    // {{d_main_topic, d_settings, d_empty, d_empty}, _ReadCrnt, 0, &cb_MQTT_com_Settings},
    // {{d_main_topic, d_settings, d_empty, d_empty}, _Edit, 0, &cb_MQTT_com_Settings},
    // {{d_main_topic, d_settings, d_empty, d_empty}, _Save, 0, &cb_MQTT_com_Settings},
    {{d_main_topic, d_settings, d_empty, d_empty}, v_all, 0, &cb_MQTT_com_Settings},
};

WiFiClient net;
AsyncMqttClient client;
// uTask *p_queueMQTTsub = NULL;
e_state_MQTT MQTT_state;      // Состояние подключения к MQTT
uint16_t mqtt_count_conn = 0; // количество (пере)подключений к серверу mqtt
// #define mqtt_count_conn_reset 200

String CreateTopic(e_IDDirTopic *_IDDirTopic, e_IDVarTopic _IDVarTopic)
{
  String result = ""; // "mqtt/0/";
  for (uint8_t i = 0; i < size_IDDirTopic; i++)
  {
    result.concat(ArrDirTopic[_IDDirTopic[i]]);
    if (_IDDirTopic[i] != 0)
      result.concat("/");
  }
  result.concat(ArrVarTopic[_IDVarTopic]);
  return result;
}

// String CreateTopic(e_IDDirTopic *_IDDirTopic, String _VarTopic)
// {
//   String result = "";
//   for (uint8_t i = 0; i < size_IDDirTopic; i++)
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

// подключились к MQTT
void onStartMQTT(void)
{
#ifdef MQTT_QUEUE
#else
  last_topic[0] = 0;
  last_payload[0] = 0;
#endif
  // rsdebugInflnF("onStartMQTT");
  mqtt_count_conn++;
  rsdebugInfF(" -подключились к MQTT №");
  rsdebugInfln("%d", mqtt_count_conn);
  // создаем очереди входящих сообщений подписок
  // if (!p_Queue_MQTT) // если нет указателя
  //   p_Queue_MQTT = new CirQueue(size_Queue_MQTT, sizeof(s_element_MQTT));
  // if (!p_Queue_MQTT)
  //   rsdebugElnF("p_Queue_MQTT: нет памяти");

  // публикуем при подключении
  MQTT_pub_allInfo(wifi_count_conn == 1);

  // подписываемся на все из массива _arr_SubscribeElement
  rsdebugInflnF("Подписываемся:");
  for (uint8 i = 0; i < (sizeof(_arr_SubscribeElement) / sizeof(_arr_SubscribeElement[0])); i++)
  {
    mqtt_subscribe(_arr_SubscribeElement[i]);
  }

  // запускаем задачи переодической публикации

  // запускаем "задачи" публикации по событию
}

// void onStopMQTT(void) // отключились от MQTT
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

//   // отписываемся ? - не надо при отключенном сервере MQTT
//   // client.unsubscribe("Water/info/CPU_load");

// }

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

void onMessageReceived(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
#ifdef MQTT_QUEUE
  char *_topic = new char[strlen(topic)];
  strcpy(_topic, topic);
  char *_payload = new char[len + 1];
  memcpy(_payload, payload, len);
  _payload[len] = 0;
  s_element_MQTT in_element = {_topic, _payload};
  // rsdebugDnfln("1topic[%s]", topic);
  rsdebugDnfln("[%d]MQTT incoming: %s[%s][%d][%d][%d]", p_Queue_MQTT.isCount(), _topic, _payload, len, index, total);
  if (p_Queue_MQTT.isFull())
  {
    rsdebugEnflnF("p_Queue_nCallback is full !!!!");
  }
  else
  {
    if (!p_Queue_MQTT.isEmpty())
    {
      // rsdebugDnflnF("@3");
      s_element_MQTT *_element = (s_element_MQTT *)p_Queue_MQTT.peeklast();
      // rsdebugDnflnF("@31");
      // rsdebugDnfln("[%d]MQTT peeklast: %s[%s]", p_Queue_MQTT->isCount() - 1, _element->topic->c_str(), _element->payload->c_str());
      // rsdebugDnfln("#1[%s][%d][%s][%d]", _element->topic, strlen(_element->topic), in_element.topic, strlen(_topic));
      // rsdebugDnfln("#2[%s][%d][%s][%d]", _element->payload, strlen(_element->payload), in_element.payload, strlen(_payload));
      // rsdebugDnflnF("@32");
      // rsdebugInfln("FreeRAM1: %d", ESP.getFreeHeap());
      if (!strcmp((*_element).topic, _topic) && !strcmp((*_element).payload, _payload))
      {
        rsdebugWnflnF("Duplicate !!!!");
        delete _topic;
        delete _payload;
        // delete in_element;
        return;
      }
      // rsdebugInfln("FreeRAM2: %d", ESP.getFreeHeap());
    }
    // {
    // rsdebugDnflnF("@3");
    // rsdebugInfln("FreeRAM3: %d", ESP.getFreeHeap());
    p_Queue_MQTT.push((uint8_t *)&in_element);
    // rsdebugInfln("FreeRAM4: %d", ESP.getFreeHeap());
    ut_MQTT.forceNextIteration();
  }
#else
  char tmp_payload[len + 1];
  memcpy(tmp_payload, payload, len);
  tmp_payload[len] = 0;
  s_element_MQTT in_element = {topic, tmp_payload};
  rsdebugDnfF("MQTT incoming: ");
  rsdebugDnfln("%s<%s>", topic, tmp_payload);
  if (!strcmp(last_topic, topic) && !strcmp(last_payload, tmp_payload))
  {
    rsdebugWnflnF("Duplicate !!!!");
    return;
  }
  else
  {
    strcpy(last_topic, topic);
    strcpy(last_payload, tmp_payload);
    String topic_in, topic_DB;
    topic_in = String(topic);
    // payload_in = String(tmp_payload);
    // uint8_t i;
    // uint8_t n = sizeof(_arr_SubscribeElement) / sizeof(_arr_SubscribeElement[0]);
    for (uint8_t i = 0; i < (sizeof(_arr_SubscribeElement) / sizeof(_arr_SubscribeElement[0])); i++)
    {
      if (_arr_SubscribeElement[i].IDVarTopic == v_all)
      {
        topic_DB = CreateTopic((e_IDDirTopic *)_arr_SubscribeElement[i].IDDirTopic, v_empty);
        topic_in.remove(topic_DB.length());
        // rsdebugDln("topic_in=%s, topic=%s, topic_DB=%s", topic_in.c_str(), topic.c_str(), topic_DB.c_str());
        // rsdebugDln("i=%d, s1=%s, s2=%s", i, topic_in.c_str(), topic_DB.c_str());
      }
      else
      {
        topic_DB = CreateTopic((e_IDDirTopic *)_arr_SubscribeElement[i].IDDirTopic, _arr_SubscribeElement[i].IDVarTopic);
      }
      if (topic_in.length() == topic_DB.length())
        if (topic_in == topic_DB)
        {
          // rsdebugDlnF("==:break");
          _arr_SubscribeElement[i].Callback(in_element);
          break;
        }
      // topic_DB = CreateTopic((e_IDDirTopic *)_arr_SubscribeElement[i].IDDirTopic, _arr_SubscribeElement[i].IDVarTopic);
      // // topic = topic_in;
      // if (_arr_SubscribeElement[i].IDVarTopic == v_all)
      // {
      //   uint16_t pos = topic_DB.length() - 2;
      //   topic_DB.remove(pos);
      //   topic_in.remove(pos);
      //   // rsdebugDln("topic_in=%s, topic=%s, topic_DB=%s", topic_in.c_str(), topic.c_str(), topic_DB.c_str());
      // }
      // // rsdebugDln("i=%d, s1=%s, s2=%s", i, topic_in.c_str(), topic_DB.c_str());
      // if (!strcmp(topic_in.c_str(), topic_DB.c_str()))
      // {
      //   // rsdebugDln("Callback[%i]", i);
      //   _arr_SubscribeElement[i].Callback(in_element);
      //   break;
      // }
      // // else if (i == n - 1)
      // //   return;
    }
  }
#endif
}

void mqtt_init(uint8_t _idx)
{
  LoadInMemorySettingsEthernet();
  rsdebugInflnF("---MQTTinit");
  // for (uint8_t aaa = 0; aaa < 5; aaa++)
  //   rsdebugInfln("------%s %d", g_p_ethernet_settings_ROM->settings_serv[aaa].MQTTip, aaa);
  if (mqtt_count_conn == 0)
  { // нужно делать только один раз, иначе калбеки вызываются по нескольку раз
    client.onMessage(onMessageReceived);
    client.onConnect(onMqttConnect);
    client.onDisconnect(onMqttDisconnect);
  }
  // else if (mqtt_count_conn == mqtt_count_conn_reset)
  //   ESP.restart();

  ut_MQTT.setInterval(g_p_ethernet_settings_ROM->MQTT_Ttask);
  client.setServer(g_p_ethernet_settings_ROM->settings_serv[_idx].MQTTip, 1883);
  client.setClientId(OTA_NAME);
  client.setCredentials(g_p_ethernet_settings_ROM->MQTT_user, g_p_ethernet_settings_ROM->MQTT_pass);
  // client.setCleanSession(true);
  // g_num_serv = num_serv;
}

void onMqttConnect(bool sessionPresent)
{
  // rsdebugDnfln("onMqttConnect[%d]", (uint8_t)sessionPresent);
  // if (MQTT_state != _MQTT_connected)
  // {
  LoadInMemorySettingsEthernet();
  ut_MQTT.setInterval(g_p_ethernet_settings_ROM->MQTT_Ttask);
  ut_MQTT.forceNextIteration();
  MQTT_state = _MQTT_on_connected;
  // }
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  // rsdebugDnfln("onMqttDisconnect[%d]", (uint8_t)reason);
  // if (!client.connected() && MQTT_state != _MQTT_disconnected && MQTT_state != _MQTT_connecting)
  // {
  //   rsdebugInflnF("Отключились от MQTT");
  // client.disconnect();
  // onStopMQTT();
  MQTT_state = _MQTT_disconnected;
  // }
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
      if (MQTT_state == _MQTT_disconnected)
      {
        // client.clearQueue();
        // client.disconnect();
        LoadInMemorySettingsEthernet();
        uint8_t _idx = get_idx_eth(WiFi.SSID());
        if (_idx)
        {
          _idx--;
          mqtt_init(_idx);
          rsdebugInfF("Подключаемся к MQTT серверу");
          rsdebugInfln("[%d]:%s", _idx, g_p_ethernet_settings_ROM->settings_serv[_idx].MQTTip);
          MQTT_state = _MQTT_connecting;
        }
        else
        {
          rsdebugInfF("Нет адреса MQTT сервера");
        }
        ut_MQTT.setInterval(2003);
      }
      else if (MQTT_state == _MQTT_connecting)
      {
        rsdebugnfF(".");
        client.connect();
      }
    }
    else if (MQTT_state == _MQTT_on_connected)
    {
      onStartMQTT();
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
  rsdebugInfF("MQTT pub: ");
  rsdebugInfln("%s<%s>", _topic, _payload);
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

bool is_equal_no(const char *char_str)
{
  return (!strcmp(char_str, "no"));
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

void mqtt_subscribe(s_SubscribeElement _sub_element)
{
  String topic = CreateTopic(_sub_element.IDDirTopic, _sub_element.IDVarTopic);
  rsdebugDnfln("sub %s", topic.c_str());
  client.subscribe(topic.c_str(), _sub_element.mqttQOS);
}

void mqtt_unsubscribe(s_SubscribeElement _sub_element)
{
}
