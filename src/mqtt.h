#include <ArduinoJson.h>

WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);

long oldTime = 0;
int switch_load = 0;
bool loadState = false, setPublishHADiscovery = false;

const char* will_Topic   = "/LWT";
const char* will_Message = "offline";
int will_QoS = 0;
bool will_Retain = true;

const char* HADISCOVERY_BASE_TOPIC    = "homeassistant/";

void mqtt_publish_s( const char* topic , const char* msg ){
#if defined(DEBUG) || defined(MQTT_DEBUG) 
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(msg);
#endif
  mqtt_client.publish(topic, msg);
}

void mqtt_publish_f( char* topic , float value  ){
  char mqtt_msg[64];
  snprintf (mqtt_msg, 64, "%7.3f", value);
  mqtt_client.publish(topic, mqtt_msg);
#if defined(DEBUG) || defined(MQTT_DEBUG)
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(mqtt_msg);
#endif
}

void mqtt_publish_i( char* topic , int value  ){
  char mqtt_msg[64];
  snprintf (mqtt_msg, 64, " %d", value);  
  mqtt_client.publish(topic, mqtt_msg);
#if defined(DEBUG) || defined(MQTT_DEBUG)
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(mqtt_msg);
#endif
}


// control load on / off here, setting sleep duration
void mqtt_callback(char* topic, byte* payload, unsigned int length) {

#if defined(DEBUG) || defined(MQTT_DEBUG)
    Serial.print(F("Message arrived ["));
    Serial.print(topic);
    Serial.print(F("] "));
    for (unsigned int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
#endif
    payload[length] = '\0';  
    char buf[MQTT_MAX_PACKET_SIZE];

    // solar/load/control
    sprintf_P(buf, PSTR("%s/load/control"),myConfig.mqtt_topic);
    if ( strncmp( topic, buf, length ) == 0 ){

        // Switch - but i can't seem to switch a coil directly here ?!?
        if ( strncmp_P( (char *) payload , PSTR("1"),1) == 0 || strcmp_P( (char *) payload , PSTR("on")) == 0  ) {
            loadState = true;
            switch_load = 1;
        }
        if ( strncmp_P( (char *) payload , PSTR("0"),1) == 0 || strcmp_P( (char *) payload , PSTR("off")) == 0  ) {
            loadState = false;
            switch_load = 1;
        } 
        if ( strncmp_P( (char *) payload , PSTR("reboot"),1) == 0 ) {
            ESP.restart();
        }
#ifdef ENABLE_HA_FACTORY_RESET_FUNCTIONS
        if ( strncmp_P( (char *) payload , PSTR("factoryreset"),1) == 0 ) {
            ESP.restart();
        }
#endif
    }
}

void mqtt_loadpublish() {

#if defined(DEBUG) || defined(MQTT_DEBUG)
  Serial.println(F("Publishing load: "));
#endif

  char buf[MQTT_MAX_PACKET_SIZE];
  sprintf_P(buf, PSTR("%s/load/state"), myConfig.mqtt_topic);
  mqtt_publish_s( buf, (char*) (loadState == 1? "on": "off") );  // pimatic state topic does not work with integers or floats ?!?
}

void mqtt_publish_old() {
  // time
  //
  //sprintf(buf, "20%02d-%02d-%02d %02d:%02d:%02d" , rtc.r.y , rtc.r.M , rtc.r.d , rtc.r.h , rtc.r.m , rtc.r.s);
  //mqtt_publish_s( "solar/status/time", buf );

  char buf[MQTT_MAX_PACKET_SIZE];
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
  sprintf(buf,"%s/battery/T",myConfig.mqtt_topic);
  mqtt_publish_f( buf, live.l.bT /100.f);  

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
  sprintf(buf,"%s/energy/consumed_day",myConfig.mqtt_topic);
  mqtt_publish_f( buf,  stats.s.consEnerMon/100.f );
  sprintf(buf,"%s/energy/consumed_year",myConfig.mqtt_topic);  
  mqtt_publish_f( buf,  stats.s.consEnerYear/100.f );
  sprintf(buf,"%s/energy/consumed_all",myConfig.mqtt_topic);
  mqtt_publish_f( buf,  stats.s.consEnerTotal/100.f );

  sprintf(buf,"%s/energy/generated_day",myConfig.mqtt_topic);
  mqtt_publish_f( buf,  stats.s.genEnerDay/100.f );
  sprintf(buf,"%s/energy/generated_month",myConfig.mqtt_topic);
  mqtt_publish_f( buf,  stats.s.genEnerMon/100.f );
  sprintf(buf,"%s/energy/generated_year",myConfig.mqtt_topic);
  mqtt_publish_f( buf,  stats.s.genEnerYear/100.f );
  sprintf(buf,"%s/energy/generated_all",myConfig.mqtt_topic);
  mqtt_publish_f( buf,   stats.s.genEnerTotal/100.f );

  sprintf(buf,"%s/status/batt_volt",myConfig.mqtt_topic);
  mqtt_publish_s( buf, batt_volt_status[status_batt.volt] );
  sprintf(buf,"%s/status/batt_temp",myConfig.mqtt_topic);
  mqtt_publish_s( buf, batt_temp_status[status_batt.temp] );

  //sprintf(buf,"%s/status/charger_input",mqtt_topic);
  //mqtt_publish_s( buf, charger_input_status[ charger_input ]  );
  sprintf(buf,"%s/status/charger_mode",myConfig.mqtt_topic);
  mqtt_publish_s(buf, charger_charging_status[charger_mode ] );  
}

void mqtt_publish() {
  // publish via mqtt using simple format
#ifdef LEGACY_MQTT
  mqtt_publish_old();
#endif

  // time
  //
  //sprintf(buf, "20%02d-%02d-%02d %02d:%02d:%02d" , rtc.r.y , rtc.r.M , rtc.r.d , rtc.r.h , rtc.r.m , rtc.r.s);
  //mqtt_publish_s( "solar/status/time", buf );

  StaticJsonDocument<MQTT_MAX_PACKET_SIZE> panelDoc;
  char buf[MQTT_MAX_PACKET_SIZE];
  char mqtt_topic[64];
  size_t jsonLength;

  // panel
  panelDoc[F("V")] = live.l.pV /100.f;
  panelDoc[F("I")] = live.l.pI /100.f;
  panelDoc[F("P")] = live.l.pP /100.f;
  panelDoc[F("minV")] = stats.s.pVmin /100.f;
  panelDoc[F("maxV")] = stats.s.pVmax /100.f;

  sprintf(mqtt_topic,"%s/panel",myConfig.mqtt_topic);
  jsonLength = serializeJson(panelDoc, buf);
  mqtt_client.publish(mqtt_topic, buf, jsonLength);
  panelDoc.clear();


  //battery
  panelDoc[F("V")] = live.l.bV /100.f;
  panelDoc[F("I")] = live.l.bI /100.f;
  panelDoc[F("P")] = live.l.bP /100.f;
  panelDoc[F("minV")] = stats.s.bVmin /100.f;
  panelDoc[F("maxV")] = stats.s.bVmax /100.f;
  panelDoc[F("SOC")] = batterySOC/1.0f;
  panelDoc[F("netI")] = batteryCurrent/100.0f;
  panelDoc[F("T")] = live.l.bT /100.f;


  sprintf_P(mqtt_topic, PSTR("%s/battery"), myConfig.mqtt_topic);
  jsonLength = serializeJson(panelDoc, buf);
  mqtt_client.publish(mqtt_topic, buf, jsonLength);
  panelDoc.clear();
  
  //load
  panelDoc[F("V")] = live.l.lV /100.f;
  panelDoc[F("I")] = live.l.lI /100.f;
  panelDoc[F("P")] = live.l.lP /100.f;
  panelDoc[F("state")] = (loadState == 1? F("on"): F("off"));

  sprintf_P(mqtt_topic, PSTR("%s/load"), myConfig.mqtt_topic);
  jsonLength = serializeJson(panelDoc, buf);
  mqtt_client.publish(mqtt_topic, buf, jsonLength);
  panelDoc.clear();

  //co2reduction
  panelDoc[F("t")] = stats.s.c02Reduction/100.f;

  sprintf_P(mqtt_topic, PSTR("%s/co2reduction"), myConfig.mqtt_topic);
  jsonLength = serializeJson(panelDoc, buf);
  mqtt_client.publish(mqtt_topic, buf, jsonLength);
  panelDoc.clear();

  //energy
  panelDoc[F("consumed_day")] = stats.s.consEnerDay/100.f;
  panelDoc[F("consumed_month")] = stats.s.consEnerMon/100.f;
  panelDoc[F("consumed_year")] = stats.s.consEnerYear/100.f;
  panelDoc[F("consumed_all")] = stats.s.consEnerTotal/100.f;

  panelDoc[F("generated_day")] = stats.s.genEnerDay/100.f;
  panelDoc[F("generated_month")] = stats.s.genEnerMon/100.f;
  panelDoc[F("generated_year")] = stats.s.genEnerYear/100.f;
  panelDoc[F("generated_all")] = stats.s.genEnerTotal/100.f;

  sprintf_P(mqtt_topic, PSTR("%s/energy"), myConfig.mqtt_topic);
  jsonLength = serializeJson(panelDoc, buf);
  mqtt_client.publish(mqtt_topic, buf, jsonLength);
  panelDoc.clear();

  //status
  panelDoc[F("batt_volt")] = batt_volt_status[status_batt.volt];
  panelDoc[F("batt_temp")] = batt_temp_status[status_batt.temp];
  panelDoc[F("charger_mode")] = charger_charging_status[ charger_mode ];
//  panelDoc["charger_input"] = charger_input_status[ charger_input ];

  sprintf_P(mqtt_topic, PSTR("%s/status"), myConfig.mqtt_topic);
  jsonLength = serializeJson(panelDoc, buf);
  mqtt_client.publish(mqtt_topic, buf, jsonLength);
}

void sendHA_Discovery_Packet( const __FlashStringHelper* type, const __FlashStringHelper* name, const __FlashStringHelper* stat_r_leaf, const __FlashStringHelper* val_tpl, const __FlashStringHelper* uniq_id, const __FlashStringHelper* unit_of_meas = 0, const __FlashStringHelper* dev_cla = 0, const __FlashStringHelper* cmd_t = 0, const __FlashStringHelper* pl_on = 0, const __FlashStringHelper* pl_off = 0)
{
  StaticJsonDocument<768> panelDoc;
  char buf[MQTT_MAX_PACKET_SIZE];
  char mqtt_topic[128];

  sprintf_P(mqtt_topic, PSTR("%s%S%s/%s%S/config"), HADISCOVERY_BASE_TOPIC, type, myConfig.mqtt_topic, baseMacChr, uniq_id);

  sprintf_P(buf, PSTR("%s%S"), myConfig.mqtt_topic, stat_r_leaf);
  panelDoc[F("stat_t")]       = buf;
  panelDoc[F("name")]         = name;
  
  sprintf_P(buf, PSTR("%s%S"), baseMacChr, uniq_id);
  panelDoc[F("uniq_id")]      = buf;

  sprintf_P(buf, PSTR("{{value_json.%S}}"), val_tpl);
  panelDoc[F("val_tpl")]      = buf;

  if (unit_of_meas != 0)
    panelDoc[F("unit_of_meas")] = unit_of_meas;

  if (dev_cla != 0)
  {
    panelDoc[F("dev_cla")] = dev_cla;
  #ifdef ENERGY_PANEL
    if (strcmp_P("energy", (PGM_P)dev_cla) == 0)
    {
      panelDoc[F("state_class")] = F("measurement");
      panelDoc[F("last_reset_topic")] = F("epever/status/last_reset");
    }
  #endif
  }

  sprintf_P(buf, PSTR("%s%S"), myConfig.mqtt_topic, will_Topic);
  panelDoc[F("avty_t")]       = buf;
  panelDoc[F("pl_avail")]     = F("online");
  panelDoc[F("pl_not_avail")] = F("offline");

  // there is a command topic, so add control attributes
  if (cmd_t != 0)
  {  
    if (pl_on != 0 && pl_off != 0)
    {
      panelDoc[F("stat_on")]  = pl_on;
      panelDoc[F("stat_off")] = pl_off;
    }

    sprintf_P(buf, PSTR("%s%S%S"), myConfig.mqtt_topic, stat_r_leaf, cmd_t);
    panelDoc[F("cmd_t")]    = buf;
  }

  if (pl_on != 0)
    panelDoc[F("pl_on")]    = pl_on;
    
  if (pl_off != 0)
    panelDoc[F("pl_off")]   = pl_off;
  

  JsonObject device = panelDoc.createNestedObject(F("device"));
    device[F("name")]  = DEVICE_DESCRIPTION;
    device[F("mdl")] = DEVICE_NAME;
    device[F("mf")] = DEVICE_MANUFACTURER;
    device[F("sw")]   = SW_VERSION;
  JsonArray identifiers = device.createNestedArray(F("identifiers"));
  identifiers.add(baseMacChr);

  size_t jsonLength = serializeJson(panelDoc, buf, MQTT_MAX_PACKET_SIZE);
  mqtt_client.publish(mqtt_topic, (const uint8_t*)buf, jsonLength, true);
}

void publishHADiscovery()
{ 
  if (myConfig.MQTT_Enable && myConfig.HADiscovery_Enable && mqtt_client.connected())
  {
    //Publish Panel Sensors
    sendHA_Discovery_Packet(F("sensor/"), F("Panel V"), F("/panel"), F("V"), F("panelV"), F("V"), F("voltage"));
    sendHA_Discovery_Packet(F("sensor/"), F("Panel I"), F("/panel"), F("I"), F("panelI"), F("A"), F("current"));
    sendHA_Discovery_Packet(F("sensor/"), F("Panel P"), F("/panel"), F("P"), F("panelP"), F("W"), F("energy"));
    sendHA_Discovery_Packet(F("sensor/"), F("Panel minV"), F("/panel"), F("minV"), F("panelminV"), F("V"), F("voltage"));
    sendHA_Discovery_Packet(F("sensor/"), F("Panel maxV"), F("/panel"), F("maxV"), F("panelmaxV"), F("V"), F("voltage"));

    //Publish Battery Sensors
    sendHA_Discovery_Packet(F("sensor/"), F("Battery V"), F("/battery"), F("V"), F("batteryV"), F("V"), F("voltage"));
    sendHA_Discovery_Packet(F("sensor/"), F("Battery I"), F("/battery"), F("I"), F("batteryI"), F("A"), F("current"));
    sendHA_Discovery_Packet(F("sensor/"), F("Battery P"), F("/battery"), F("P"), F("batteryP"), F("W"), F("energy"));
    sendHA_Discovery_Packet(F("sensor/"), F("Battery minV"), F("/battery"), F("V"), F("batteryminV"), F("V"), F("voltage"));
    sendHA_Discovery_Packet(F("sensor/"), F("Battery maxV"), F("/battery"), F("V"), F("batterymaxV"), F("V"), F("voltage"));
    sendHA_Discovery_Packet(F("sensor/"), F("Battery SOC"), F("/battery"), F("SOC"), F("batterySOC"), F("%"), F("battery"));
    sendHA_Discovery_Packet(F("sensor/"), F("Battery netI"), F("/battery"), F("netI"), F("batterynetI"), F("A"), F("current"));

    //Publish Load Sensors
    sendHA_Discovery_Packet(F("sensor/"), F("Load V"), F("/load"), F("V"), F("loadV"), F("V"), F("voltage"));
    sendHA_Discovery_Packet(F("sensor/"), F("Load I"), F("/load"), F("I"), F("loadI"), F("A"), F("current"));
    sendHA_Discovery_Packet(F("sensor/"), F("Load P"), F("/load"), F("P"), F("loadP"), F("W"), F("energy"));
    sendHA_Discovery_Packet(F("binary_sensor/"), F("Load State"), F("/load"), F("state"), F("loadState"), 0, 0, 0, F("on"), F("off"));

    //Publish Load Switch
    sendHA_Discovery_Packet(F("switch/"), F("Load Switch"), F("/load"), F("state"), F("loadSwitch"), 0 ,0 ,F("/control"), F("on"), F("off"));

    //Publish Energy Sensors
    sendHA_Discovery_Packet(F("sensor/"), F("Consumed Today"), F("/energy"), F("consumed_day"), F("consDay"), F("kWh"), F("energy"));
    sendHA_Discovery_Packet(F("sensor/"), F("Consumed Month"), F("/energy"), F("consumed_month"), F("consMth"), F("kWh"), F("energy"));
    sendHA_Discovery_Packet(F("sensor/"), F("Consumed Year"), F("/energy"), F("consumed_year"), F("consYr"), F("kWh"), F("energy"));
    sendHA_Discovery_Packet(F("sensor/"), F("Consumed All"), F("/energy"), F("consumed_all"), F("consAll"), F("kWh"), F("energy"));
    sendHA_Discovery_Packet(F("sensor/"), F("Generated Today"), F("/energy"), F("generated_day"), F("genDay"), F("kWh"), F("energy"));
    sendHA_Discovery_Packet(F("sensor/"), F("Generated Month"), F("/energy"), F("generated_month"), F("genMth"), F("kWh"), F("energy"));
    sendHA_Discovery_Packet(F("sensor/"), F("Generated Year"), F("/energy"), F("generated_year"), F("genYr"), F("kWh"), F("energy"));
    sendHA_Discovery_Packet(F("sensor/"), F("Generated All"), F("/energy"), F("generated_all"), F("genAll"), F("kWh"), F("energy"));

    //Publish CO2 Sensors
    sendHA_Discovery_Packet(F("sensor/"), F("CO2 Reduction"), F("/co2reduction"), F("t"), F("co2reduction"), F("tons"), F("power"));

    //Publish Status Sensors
    sendHA_Discovery_Packet(F("sensor/"), F("Battery Temperature"), F("/status"), F("batt_temp"), F("battTemp"), F("C"), F("temperature"));
    sendHA_Discovery_Packet(F("sensor/"), F("Battery Voltage"), F("/status"), F("batt_volt"), F("battVolt"));
    sendHA_Discovery_Packet(F("sensor/"), F("Charger Mode"), F("/status"), F("charger_mode"), F("chargeMode"));

    //Publish Restart Switch
    sendHA_Discovery_Packet(F("switch/"), F("Restart"), F("/load"), F("state"), F("restart"), 0 ,0 ,F("/control"), F("restart"), F(""));

#ifdef ENABLE_HA_FACTORY_RESET_FUNCTIONS
    //Publish Factory Reset Switch
    sendHA_Discovery_Packet(F("switch/"), F("Factory Reset"), F("/load"), F("state"), F("factoryreset"), 0 ,0 ,F("/control"), F("FactoryReset"), F(""));
#endif
    
    //set to false as they don't need to be republished
    setPublishHADiscovery = false;
  }
}

void mqtt_reconnect() {
  
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {

#if defined(DEBUG) || defined(MQTT_DEBUG) 
    Serial.print(F("Attempting MQTT connection..."));
#endif
    
    char buf[MQTT_MAX_PACKET_SIZE];
    strcpy(buf, myConfig.mqtt_topic);
    strcat(buf, will_Topic);

    // Attempt to connect
    mqtt_client.setServer(myConfig.mqtt_server, myConfig.mqtt_port);
    if (mqtt_client.connect( baseMacChr, 
                             myConfig.mqtt_username, 
                             myConfig.mqtt_password, 
                             buf,
                             will_QoS,
                             will_Retain,
                             will_Message)) {

      // Once connected, publish an announcement...
      mqtt_client.publish(buf, "online", true);

#if defined(DEBUG) || defined(MQTT_DEBUG)
      Serial.println(F("connected"));
#endif
      mqtt_client.setCallback(mqtt_callback);

      char buf[MQTT_MAX_PACKET_SIZE];
      // ... and resubscribe
      sprintf(buf,"%s/load/control",myConfig.mqtt_topic);
      mqtt_client.subscribe(buf);
      sprintf(buf,"%s/setting/sleep",myConfig.mqtt_topic);
      mqtt_client.subscribe(buf);

    } else {
#if defined(DEBUG) || defined(MQTT_DEBUG) 
      Serial.print(F("failed, rc="));
      Serial.print(mqtt_client.state());
      Serial.println(F(" try again in 5 seconds"));
#endif
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}