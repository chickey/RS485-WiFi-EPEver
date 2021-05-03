WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);

char mqtt_msg[64];
char buf[256];
long oldTime = 0;
int do_update = 0, switch_load = 0;
bool loadState;

void mqtt_publish_s( char* topic , char* msg ){

  Serial.print(topic);
  Serial.print(": ");
  Serial.println(msg);
  
  mqtt_client.publish(topic, msg);
  
}

void mqtt_publish_f( char* topic , float value  ){

  Serial.print(topic);
  Serial.print(": ");
  
  snprintf (mqtt_msg, 64, "%7.3f", value);
  Serial.println(mqtt_msg);
  
  mqtt_client.publish(topic, mqtt_msg);
  
}
void mqtt_publish_i( char* topic , int value  ){

  Serial.print(topic);
  Serial.print(": ");
  
  snprintf (mqtt_msg, 64, " %d", value);
  Serial.println(mqtt_msg);
  
  mqtt_client.publish(topic, mqtt_msg);
  
}

void mqtt_reconnect() {
  
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    
    Serial.print("Attempting MQTT connection...");
    
    // Create a client ID
    String clientId = "EpEver Solar Monitor";
    
    // Attempt to connect
    if (mqtt_client.connect(clientId.c_str(),myConfig.mqtt_username,myConfig.mqtt_password,myConfig.mqtt_topic,0,true,"offline")) {
      
      Serial.println("connected");
      
      // Once connected, publish an announcement...
      mqtt_client.publish(myConfig.mqtt_topic, "online");
      do_update = 1;
      
      // ... and resubscribe
      sprintf(buf,"%s/load/control",myConfig.mqtt_topic);
      mqtt_client.subscribe(buf);
      sprintf(buf,"%s/setting/sleep",myConfig.mqtt_topic);
      mqtt_client.subscribe(buf);
      
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// control load on / off here, setting sleep duration
//
void mqtt_callback(char* topic, byte* payload, unsigned int length) {

    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    payload[length] = '\0';

    

    // solar/load/control
    //
    sprintf(buf,"%s/load/control",myConfig.mqtt_topic);
    if ( strncmp( topic, buf, strlen(buf) ) == 0 ){

        // Switch - but i can't seem to switch a coil directly here ?!?
        if ( strncmp( (char *) payload , "1",1) == 0 || strcmp( (char *) payload , "on") == 0  ) {
            loadState = true;
            do_update = 1;
            switch_load = 1;
        } 
        if ( strncmp( (char *) payload , "0",1) == 0 || strcmp( (char *) payload , "off") == 0  ) {
            loadState = false;
            do_update = 1;
            switch_load = 1;
        } 
    } 
}

void mqtt_publish() {
  // publish via mqtt
  //
  Serial.println("Publishing: ");
  
  // time
  //
  //sprintf(buf, "20%02d-%02d-%02d %02d:%02d:%02d" , rtc.r.y , rtc.r.M , rtc.r.d , rtc.r.h , rtc.r.m , rtc.r.s);
  //mqtt_publish_s( "solar/status/time", buf );

  // panel
  // 
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
