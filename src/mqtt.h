#include <ArduinoJson.h>

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);

char mqtt_msg[64];
char buf[256];
long oldTime = 0;
int switch_load = 0;
bool loadState;

const char* will_Topic   = "/LWT";
const char* will_Message = "offline";
int will_QoS = 1;
bool will_Retain = true;

void mqtt_publish_s( char* topic , char* msg ){
#ifdef DEBUG 
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(msg);
#endif
  mqtt_client.publish(topic, msg);
}

void mqtt_publish_f( char* topic , float value  ){
#ifdef DEBUG
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(mqtt_msg);
#endif
  snprintf (mqtt_msg, 64, "%7.3f", value);
  mqtt_client.publish(topic, mqtt_msg);
}

void mqtt_publish_i( char* topic , int value  ){
#ifdef DEBUG
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(mqtt_msg);
#endif
  snprintf (mqtt_msg, 64, " %d", value);
  
  mqtt_client.publish(topic, mqtt_msg);
}


// control load on / off here, setting sleep duration
//
void mqtt_callback(char* topic, byte* payload, unsigned int length) {

#ifdef DEBUG
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
#endif
    payload[length] = '\0';  

    // solar/load/control
    //
    sprintf(buf,"%s/load/control",myConfig.mqtt_topic);
    if ( strncmp( topic, buf, strlen(buf) ) == 0 ){

        // Switch - but i can't seem to switch a coil directly here ?!?
        if ( strncmp( (char *) payload , "1",1) == 0 || strcmp( (char *) payload , "on") == 0  ) {
            loadState = true;
            switch_load = 1;
        } 
        if ( strncmp( (char *) payload , "0",1) == 0 || strcmp( (char *) payload , "off") == 0  ) {
            loadState = false;
            switch_load = 1;
        } 
    } 
}

void mqtt_loadpublish() {

#ifdef DEBUG
  Serial.println("Publishing load: ");
#endif
  sprintf(buf,"%s/load/state",myConfig.mqtt_topic);
  mqtt_publish_s( buf,    (char*) (loadState == 1? "on": "off") );  // pimatic state topic does not work with integers or floats ?!?
}

void mqtt_publish_old() {
  // publish via mqtt
#ifdef DEBUG
  Serial.println("Publishing: ");
#endif
  // time
  //
  //sprintf(buf, "20%02d-%02d-%02d %02d:%02d:%02d" , rtc.r.y , rtc.r.M , rtc.r.d , rtc.r.h , rtc.r.m , rtc.r.s);
  //mqtt_publish_s( "solar/status/time", buf );

  // panel
  sprintf(buf,"%s/panel/V",myConfig.mqtt_topic);
  mqtt_publish_f( buf, live.l.pV /100.f);
  sprintf(buf,"%s/panel/I",myConfig.mqtt_topic);
  mqtt_publish_f( buf, live.l.pI /100.f);
  sprintf(buf,"%s/panel/P",myConfig.mqtt_topic);
  mqtt_publish_f( buf, live.l.pP /100.f);

  sprintf(buf,"%s/battery/V",myConfig.mqtt_topic);
  mqtt_publish_f( buf, live.l.bV /100.f);
  sprintf(buf,"%s/battery/I",myConfig.mqtt_topic);
  mqtt_publish_f( buf, live.l.bI /100.f);
  sprintf(buf,"%s/battery/P",myConfig.mqtt_topic);
  mqtt_publish_f( buf, live.l.bP /100.f);
  
  sprintf(buf,"%s/load/V",myConfig.mqtt_topic);  
  mqtt_publish_f( buf, live.l.lV /100.f);
  sprintf(buf,"%s/load/I",myConfig.mqtt_topic);
  mqtt_publish_f( buf, live.l.lI /100.f);
  sprintf(buf,"%s/load/P",myConfig.mqtt_topic);
  mqtt_publish_f( buf, live.l.lP /100.f);

  sprintf(buf,"%s/co2reduction/t",myConfig.mqtt_topic);
  mqtt_publish_f( buf,  stats.s.c02Reduction/100.f);
  sprintf(buf,"%s/battery/SOC",myConfig.mqtt_topic);
  mqtt_publish_f( buf,    batterySOC/1.0f);
  sprintf(buf,"%s/battery/netI",myConfig.mqtt_topic);
  mqtt_publish_f( buf,   batteryCurrent/100.0f);
  sprintf(buf,"%s/load/state",myConfig.mqtt_topic);
  mqtt_publish_s( buf,    (char*) (loadState == 1? "on": "off") );  // pimatic state topic does not work with integers or floats ?!?
   
  sprintf(buf,"%s/battery/minV",myConfig.mqtt_topic);
  mqtt_publish_f( buf,  stats.s.bVmin /100.f);
  sprintf(buf,"%s/battery/maxV",myConfig.mqtt_topic);
  mqtt_publish_f( buf,  stats.s.bVmax /100.f);
  
  sprintf(buf,"%s/panel/minV",myConfig.mqtt_topic);
  mqtt_publish_f( buf,  stats.s.pVmin /100.f);
  sprintf(buf,"%s/panel/maxV",myConfig.mqtt_topic);
  mqtt_publish_f( buf,  stats.s.pVmax /100.f);

  sprintf(buf,"%s/energy/consumed_day",myConfig.mqtt_topic);  
  mqtt_publish_f( buf,  stats.s.consEnerDay/100.f );
  sprintf(buf,"%s/energy/consumed_all",myConfig.mqtt_topic);
  mqtt_publish_f( buf,  stats.s.consEnerTotal/100.f );

  sprintf(buf,"%s/energy/generated_day",myConfig.mqtt_topic);
  mqtt_publish_f( buf,  stats.s.genEnerDay/100.f );
  sprintf(buf,"%s/energy/generated_all",myConfig.mqtt_topic);
  mqtt_publish_f( buf,   stats.s.genEnerTotal/100.f );

  sprintf(buf,"%s/status/batt_volt",myConfig.mqtt_topic);
  mqtt_publish_s( buf, batt_volt_status[status_batt.volt] );
  sprintf(buf,"%s/status/batt_temp",myConfig.mqtt_topic);
  mqtt_publish_s( buf, batt_temp_status[status_batt.temp] );

  //sprintf(buf,"%s/status/charger_input",mqtt_topic);
  //mqtt_publish_s( buf, charger_input_status[ charger_input ]  );
  sprintf(buf,"%s/status/charger_mode",myConfig.mqtt_topic);
  mqtt_publish_s( buf,  charger_charging_status[ charger_mode ] );  
}

void mqtt_publish() {
  // publish via mqtt using simple format
  mqtt_publish_old();

  // publish via mqtt in JSON
#ifdef DEBUG
  Serial.println("Publishing: ");
#endif
  // time
  //
  //sprintf(buf, "20%02d-%02d-%02d %02d:%02d:%02d" , rtc.r.y , rtc.r.M , rtc.r.d , rtc.r.h , rtc.r.m , rtc.r.s);
  //mqtt_publish_s( "solar/status/time", buf );

  StaticJsonDocument<256> panelDoc;
  char jsonBuffer[256];
  size_t jsonLength;

  // panel
  panelDoc["V"] = live.l.pV /100.f;
  panelDoc["I"] = live.l.pI /100.f;
  panelDoc["P"] = live.l.pP /100.f;
  panelDoc["minV"] = stats.s.pVmin /100.f;
  panelDoc["maxV"] = stats.s.pVmax /100.f;

  sprintf(buf,"%s/panel",myConfig.mqtt_topic);
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish(buf, jsonBuffer, jsonLength);
  panelDoc.clear();


  //battery
  panelDoc["V"] = live.l.bV /100.f;
  panelDoc["I"] = live.l.bI /100.f;
  panelDoc["P"] = live.l.bP /100.f;
  panelDoc["minV"] = stats.s.bVmin /100.f;
  panelDoc["maxV"] = stats.s.bVmax /100.f;
  panelDoc["SOC"] = batterySOC/1.0f;
  panelDoc["netI"] = batteryCurrent/100.0f;

  sprintf(buf,"%s/battery",myConfig.mqtt_topic);
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish(buf, jsonBuffer, jsonLength);
  panelDoc.clear();
  
  //load
  panelDoc["V"] = live.l.lV /100.f;
  panelDoc["I"] = live.l.lI /100.f;
  panelDoc["P"] = live.l.lP /100.f;
  panelDoc["state"] = (loadState == 1? "on": "off");

  sprintf(buf,"%s/load",myConfig.mqtt_topic);
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish(buf, jsonBuffer, jsonLength);
  panelDoc.clear();

  //co2reduction
  panelDoc["t"] = stats.s.c02Reduction/100.f;

  sprintf(buf,"%s/co2reduction",myConfig.mqtt_topic);
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish(buf, jsonBuffer, jsonLength);
  panelDoc.clear();

  //energy
  panelDoc["consumed_day"] = stats.s.consEnerDay/100.f;
  panelDoc["consumed_all"] = stats.s.consEnerTotal/100.f;
  panelDoc["generated_day"] = stats.s.genEnerDay/100.f;
  panelDoc["generated_all"] = stats.s.genEnerTotal/100.f;

  sprintf(buf,"%s/energy",myConfig.mqtt_topic);
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish(buf, jsonBuffer, jsonLength);
  panelDoc.clear();

  //status
  panelDoc["batt_volt"] = batt_volt_status[status_batt.volt];
  panelDoc["batt_temp"] = batt_temp_status[status_batt.temp];
  panelDoc["charger_mode"] = charger_charging_status[ charger_mode ];
//  panelDoc["charger_input"] = charger_input_status[ charger_input ];

  sprintf(buf,"%s/status",myConfig.mqtt_topic);
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish(buf, jsonBuffer, jsonLength);
  panelDoc.clear();
}

String getMacAddress() {
  uint8_t baseMac[6];
  char baseMacChr[13] = {0};
#  if defined(ESP8266)
  WiFi.macAddress(baseMac);
  sprintf(baseMacChr, "%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
#  elif defined(ESP32)
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  sprintf(baseMacChr, "%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
#  else
  sprintf(baseMacChr, "%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#  endif
  return String(baseMacChr);
}

String HADISCOVERY_BASE_TOPIC = "homeassistant/";
String HADISCOVERY_SENSOR = "sensor/";
String HADISCOVERY_SWITCH = "switch/";
String HADISCOVERY_BINARY_SENSOR = "binary_sensor/";

void pubHASwitch()
{
  StaticJsonDocument<512> panelDoc;
  char jsonBuffer[512];
  size_t jsonLength;

  String uniqueID = getMacAddress();
  String baseTopic = HADISCOVERY_BASE_TOPIC + HADISCOVERY_SWITCH + "epever/" + uniqueID;
  
  // Load Control Switch
  panelDoc["stat_t"]   = String(myConfig.mqtt_topic) + "/load";
  panelDoc["cmd_t"]    = String(myConfig.mqtt_topic) + "/load/control";
  panelDoc["name"]     = "Load Switch";
  panelDoc["uniq_id"]  = (uniqueID + "loadSwitch");
  panelDoc["val_tpl"]  = "{{ value_json.state }}";
  panelDoc["stat_on"]  = "on";
  panelDoc["stat_off"] = "off";
  panelDoc["pl_on"]    = "on";
  panelDoc["pl_off"]   = "off";
  panelDoc["avty_t"]       = String(myConfig.mqtt_topic) + will_Topic;
  panelDoc["pl_avail"]     = "online";
  panelDoc["pl_not_avail"] = "offline";

  JsonObject device = panelDoc.createNestedObject("device");
    device["name"]  = MQTT_CLIENT_ID;
    device["model"] = DEVICE_NAME;
    device["manufacturer"] = "EPEver";
    device["sw_version"]   = SW_VERSION;
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(uniqueID);

  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "loadSwitch/config").c_str(), jsonBuffer, jsonLength);
}

void pubHABinarySensor()
{
  StaticJsonDocument<512> panelDoc;
  char jsonBuffer[512];
  size_t jsonLength;

  String uniqueID = getMacAddress();
  String baseTopic = HADISCOVERY_BASE_TOPIC + HADISCOVERY_BINARY_SENSOR + "epever/" + uniqueID;

  panelDoc["stat_t"]  = String(myConfig.mqtt_topic) + "/load";
  panelDoc["name"]    = "Load State";
  panelDoc["uniq_id"] = (uniqueID + "loadState");
  panelDoc["val_tpl"] = "{{ value_json.state }}";
  panelDoc["dev_cla"]  = "power";
  panelDoc["avty_t"]       = String(myConfig.mqtt_topic) + will_Topic;
  panelDoc["pl_avail"]     = "online";
  panelDoc["pl_not_avail"] = "offline";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "loadState/config").c_str(), jsonBuffer, jsonLength);
}

void pubHA_CO2Sensors()
{
  StaticJsonDocument<512> panelDoc;
  char jsonBuffer[512];
  size_t jsonLength;

  String uniqueID = getMacAddress();
  String baseTopic = HADISCOVERY_BASE_TOPIC + HADISCOVERY_SENSOR + "epever/" + uniqueID;

  panelDoc["stat_t"]  = String(myConfig.mqtt_topic) + "/co2reduction";
  panelDoc["name"]    = "CO2 Reduction";
  panelDoc["uniq_id"] = (uniqueID + "co2reduction");
  panelDoc["val_tpl"] = "{{ value_json.t }}";
  panelDoc["unit_of_meas"] = "tons";
  panelDoc["avty_t"]       = String(myConfig.mqtt_topic) + will_Topic;
  panelDoc["pl_avail"]     = "online";
  panelDoc["pl_not_avail"] = "offline";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "co2reduction/config").c_str(), jsonBuffer, jsonLength);
}

void pubHA_PanelSensors()
{
  StaticJsonDocument<512> panelDoc;
  char jsonBuffer[512];
  size_t jsonLength;

  String uniqueID = getMacAddress();
  String baseTopic = HADISCOVERY_BASE_TOPIC + HADISCOVERY_SENSOR + "epever/" + uniqueID;

  panelDoc["stat_t"]       = String(myConfig.mqtt_topic) + "/panel";
  panelDoc["name"]         = "Panel V";
  panelDoc["uniq_id"]      = (uniqueID + "panelV");
  panelDoc["val_tpl"]      = "{{ value_json.V }}";
  panelDoc["unit_of_meas"] = "V";
  panelDoc["dev_cla"]      = "voltage";
  panelDoc["avty_t"]       = String(myConfig.mqtt_topic) + will_Topic;
  panelDoc["pl_avail"]     = "online";
  panelDoc["pl_not_avail"] = "offline";

  JsonObject device = panelDoc.createNestedObject("device");
    device["name"]  = MQTT_CLIENT_ID;
    device["model"] = DEVICE_NAME;
    device["manufacturer"] = "EPEver";
    device["sw_version"]   = SW_VERSION;
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(uniqueID);

  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "panelV/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Panel I";
  panelDoc["uniq_id"] = (uniqueID + "panelI");
  panelDoc["val_tpl"] = "{{ value_json.I }}";
  panelDoc["unit_of_meas"] = "A";
  panelDoc["dev_cla"]      = "current";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "panelI/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Panel P";
  panelDoc["uniq_id"] = (uniqueID + "panelP");
  panelDoc["val_tpl"] = "{{ value_json.P }}";
  panelDoc["unit_of_meas"] = "W";
  panelDoc["dev_cla"]      = "energy";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "panelP/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Panel minV";
  panelDoc["uniq_id"] = (uniqueID + "panelminV");
  panelDoc["val_tpl"] = "{{ value_json.minV }}";
  panelDoc["unit_of_meas"] = "V";
  panelDoc["dev_cla"]      = "voltage";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "panelminV/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Panel maxV";
  panelDoc["uniq_id"] = (uniqueID + "panelmaxV");
  panelDoc["val_tpl"] = "{{ value_json.maxV }}";
  panelDoc["unit_of_meas"] = "V";
  panelDoc["dev_cla"]      = "voltage";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "panelmaxV/config").c_str(), jsonBuffer, jsonLength);
}

void pubHA_BatterySensors()
{
  StaticJsonDocument<512> panelDoc;
  char jsonBuffer[512];
  size_t jsonLength;

  String uniqueID = getMacAddress();
  String baseTopic = HADISCOVERY_BASE_TOPIC + HADISCOVERY_SENSOR + "epever/" + uniqueID;

  panelDoc["stat_t"]  = String(myConfig.mqtt_topic) + "/battery";
  panelDoc["name"]    = "Battery V";
  panelDoc["uniq_id"] = (uniqueID + "batteryV");
  panelDoc["val_tpl"] = "{{ value_json.V }}";
  panelDoc["unit_of_meas"] = "V";
  panelDoc["dev_cla"]      = "voltage";
  panelDoc["avty_t"]       = String(myConfig.mqtt_topic) + will_Topic;
  panelDoc["pl_avail"]     = "online";
  panelDoc["pl_not_avail"] = "offline";

  JsonObject device = panelDoc.createNestedObject("device");
    device["name"]  = MQTT_CLIENT_ID;
    device["model"] = DEVICE_NAME;
    device["manufacturer"] = "EPEver";
    device["sw_version"]   = SW_VERSION;
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(uniqueID);

  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "batteryV/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Battery I";
  panelDoc["uniq_id"] = (uniqueID + "batteryI");
  panelDoc["val_tpl"] = "{{ value_json.I }}";
  panelDoc["unit_of_meas"] = "A";
  panelDoc["dev_cla"]      = "current";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "batteryI/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Battery P";
  panelDoc["uniq_id"] = (uniqueID + "batteryP");
  panelDoc["val_tpl"] = "{{ value_json.P }}";
  panelDoc["unit_of_meas"] = "W";
  panelDoc["dev_cla"]      = "energy";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "batteryP/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Battery minV";
  panelDoc["uniq_id"] = (uniqueID + "batteryminV");
  panelDoc["val_tpl"] = "{{ value_json.minV }}";
  panelDoc["unit_of_meas"] = "V";
  panelDoc["dev_cla"]      = "voltage";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "batteryminV/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Battery maxV";
  panelDoc["uniq_id"] = (uniqueID + "batterymaxV");
  panelDoc["val_tpl"] = "{{ value_json.maxV }}";
  panelDoc["unit_of_meas"] = "V";
  panelDoc["dev_cla"]      = "voltage";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "batterymaxV/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Battery SOC";
  panelDoc["uniq_id"] = (uniqueID + "batterySOC");
  panelDoc["val_tpl"] = "{{ value_json.SOC }}";
  panelDoc["unit_of_meas"] = "%";
  panelDoc["dev_cla"]      = "battery";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "batterySOC/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Battery netI";
  panelDoc["uniq_id"] = (uniqueID + "batterynetI");
  panelDoc["val_tpl"] = "{{ value_json.netI }}";
  panelDoc["unit_of_meas"] = "A";
  panelDoc["dev_cla"]      = "current";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "batterynetI/config").c_str(), jsonBuffer, jsonLength);

}

void pubHA_LoadSensors()
{
  StaticJsonDocument<512> panelDoc;
  char jsonBuffer[512];
  size_t jsonLength;

  String uniqueID = getMacAddress();
  String baseTopic = HADISCOVERY_BASE_TOPIC + HADISCOVERY_SENSOR + "epever/" + uniqueID;

  panelDoc["stat_t"]  = String(myConfig.mqtt_topic) + "/load";
  panelDoc["name"]    = "Load V";
  panelDoc["uniq_id"] = (uniqueID + "loadV");
  panelDoc["val_tpl"] = "{{ value_json.V }}";
  panelDoc["unit_of_meas"] = "V";
  panelDoc["dev_cla"]      = "voltage";
  panelDoc["avty_t"]       = String(myConfig.mqtt_topic) + will_Topic;
  panelDoc["pl_avail"]     = "online";
  panelDoc["pl_not_avail"] = "offline";

  JsonObject device = panelDoc.createNestedObject("device");
    device["name"]  = MQTT_CLIENT_ID;
    device["model"] = DEVICE_NAME;
    device["manufacturer"] = "EPEver";
    device["sw_version"]   = SW_VERSION;
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(uniqueID);

  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "loadV/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Load I";
  panelDoc["uniq_id"] = (uniqueID + "loadI");
  panelDoc["val_tpl"] = "{{ value_json.I }}";
  panelDoc["unit_of_meas"] = "A";
  panelDoc["dev_cla"]      = "current";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "loadI/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Load P";
  panelDoc["uniq_id"] = (uniqueID + "loadP");
  panelDoc["val_tpl"] = "{{ value_json.P }}";
  panelDoc["unit_of_meas"] = "W";
  panelDoc["dev_cla"]      = "energy";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "loadP/config").c_str(), jsonBuffer, jsonLength);
}

void pubHA_EnergySensors()
{
  StaticJsonDocument<512> panelDoc;
  char jsonBuffer[512];
  size_t jsonLength;

  String uniqueID = getMacAddress();
  String baseTopic = HADISCOVERY_BASE_TOPIC + HADISCOVERY_SENSOR + "epever/" + uniqueID;

  panelDoc["stat_t"]  = String(myConfig.mqtt_topic) + "/energy";
  panelDoc["name"]    = "Consumed Today";
  panelDoc["uniq_id"] = (uniqueID + "consDay");
  panelDoc["val_tpl"] = "{{ value_json.consumed_day }}";
  panelDoc["unit_of_meas"] = "W";
  panelDoc["dev_cla"]      = "energy";
  panelDoc["avty_t"]       = String(myConfig.mqtt_topic) + will_Topic;
  panelDoc["pl_avail"]     = "online";
  panelDoc["pl_not_avail"] = "offline";

  JsonObject device = panelDoc.createNestedObject("device");
    device["name"]  = MQTT_CLIENT_ID;
    device["model"] = DEVICE_NAME;
    device["manufacturer"] = "EPEver";
    device["sw_version"]   = SW_VERSION;
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(uniqueID);

  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "consDay/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Consumed All";
  panelDoc["uniq_id"] = (uniqueID + "consAll");
  panelDoc["val_tpl"] = "{{ value_json.consumed_all }}";
  panelDoc["unit_of_meas"] = "W";
  panelDoc["dev_cla"]      = "energy";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "consAll/config").c_str(), jsonBuffer, jsonLength);
  
  panelDoc["name"]    = "Generated Today";
  panelDoc["uniq_id"] = (uniqueID + "genDay");
  panelDoc["val_tpl"] = "{{ value_json.generated_day }}";
  panelDoc["unit_of_meas"] = "W";
  panelDoc["dev_cla"]      = "energy";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "genDay/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Generated All";
  panelDoc["uniq_id"] = (uniqueID + "genAll");
  panelDoc["val_tpl"] = "{{ value_json.generated_all }}";
  panelDoc["unit_of_meas"] = "W";
  panelDoc["dev_cla"]      = "energy";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "genAll/config").c_str(), jsonBuffer, jsonLength);
}

void pubHA_StatusSensors()
{
  StaticJsonDocument<512> panelDoc;
  char jsonBuffer[512];
  size_t jsonLength;

  String uniqueID = getMacAddress();
  String baseTopic = HADISCOVERY_BASE_TOPIC + HADISCOVERY_SENSOR + "epever/" + uniqueID;

  panelDoc["stat_t"]  = String(myConfig.mqtt_topic) + "/status";
  panelDoc["name"]    = "Battery Voltage";
  panelDoc["uniq_id"] = (uniqueID + "battVolt");
  panelDoc["val_tpl"] = "{{ value_json.batt_volt }}";
  panelDoc["avty_t"]       = String(myConfig.mqtt_topic) + will_Topic;
  panelDoc["pl_avail"]     = "online";
  panelDoc["pl_not_avail"] = "offline";

  JsonObject device = panelDoc.createNestedObject("device");
    device["name"]  = MQTT_CLIENT_ID;
    device["model"] = DEVICE_NAME;
    device["manufacturer"] = "EPEver";
    device["sw_version"]   = SW_VERSION;
  JsonArray identifiers = device.createNestedArray("identifiers");
  identifiers.add(uniqueID);

  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "batt_volt/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Battery Temperature";
  panelDoc["uniq_id"] = (uniqueID + "battTemp");
  panelDoc["val_tpl"] = "{{ value_json.batt_temp }}";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "batt_temp/config").c_str(), jsonBuffer, jsonLength);

  panelDoc["name"]    = "Charger Mode";
  panelDoc["uniq_id"] = (uniqueID + "chargeMode");
  panelDoc["val_tpl"] = "{{ value_json.charger_mode }}";
  jsonLength = serializeJson(panelDoc, jsonBuffer);
  mqtt_client.publish((baseTopic + "charger_mode/config").c_str(), jsonBuffer, jsonLength);

}

void publishHADiscovery()
{ 
  pubHA_PanelSensors();
  pubHA_BatterySensors();
  pubHA_LoadSensors();
  pubHA_EnergySensors();
  pubHA_CO2Sensors();
  pubHA_StatusSensors();
  pubHABinarySensor();
  pubHASwitch();
}

void mqtt_reconnect() {
  
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {

#ifdef DEBUG 
    Serial.print("Attempting MQTT connection...");
#endif
    
    char topic[strlen(myConfig.mqtt_topic)+strlen(will_Topic)];
    strcpy(topic, myConfig.mqtt_topic);
    strcat(topic, will_Topic);

    // Attempt to connect
    mqtt_client.setServer(myConfig.mqtt_server, myConfig.mqtt_port);
    if (mqtt_client.connect(MQTT_CLIENT_ID, 
                              myConfig.mqtt_username, 
                              myConfig.mqtt_password, 
                              topic,
                              will_QoS,
                              will_Retain,
                              will_Message)) {

      // Once connected, publish an announcement...
      mqtt_client.publish(topic, "online", true);

#ifdef DEBUG 
      Serial.println("connected");
#endif
      mqtt_client.setCallback(mqtt_callback);

      // ... and resubscribe
      sprintf(buf,"%s/load/control",myConfig.mqtt_topic);
      mqtt_client.subscribe(buf);
      sprintf(buf,"%s/setting/sleep",myConfig.mqtt_topic);
      mqtt_client.subscribe(buf);

#ifdef HADISCOVERY
      publishHADiscovery();
#endif

    } else {
#ifdef DEBUG 
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
#endif
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}