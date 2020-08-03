WiFiClient wifi_client;
PubSubClient mqtt_client(wifi_client);

const char* mqtt_username = "";
const char* mqtt_password = "";
const char* mqtt_topic = "";

char mqtt_msg[64];
char buf[256];
long oldTime = 0;
int do_update = 0, switch_load = 0, MQTT_Enable = 0;
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
    if (mqtt_client.connect(clientId.c_str(),mqtt_username,mqtt_password)) {
      
      Serial.println("connected");
      
      // Once connected, publish an announcement...
      mqtt_client.publish("solar", "online");
      do_update = 1;
      
      // ... and resubscribe
      mqtt_client.subscribe("solar/load/control");
      mqtt_client.subscribe("solar/setting/sleep");
      
      
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
    if ( strncmp( topic, "solar/load/control", strlen("solar/load/control") ) == 0 ){

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

char* concat(char* str1, char* str2){
  char* result;
  strcpy(result, str1);
  strcat(result, str2);
  return result;
}

void mqtt_publish() {
  // publish via mqtt
  //
  Serial.println("Publishing: ");
    
  
  // time
  //
  sprintf(buf, "20%02d-%02d-%02d %02d:%02d:%02d" , rtc.r.y , rtc.r.M , rtc.r.d , rtc.r.h , rtc.r.m , rtc.r.s);
  mqtt_publish_s( "solar/status/time", buf );

  
  // panel
  // 
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic),"/panel/V"), live.l.pV /100.f);
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/panel/I"), live.l.pI /100.f);
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/panel/P"), live.l.pP /100.f);
  
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/battery/V"), live.l.bV /100.f);
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/battery/I"), live.l.bI /100.f);
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/battery/P"), live.l.bP /100.f);
  
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/load/V"), live.l.lV /100.f);
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/load/I"), live.l.lI /100.f);
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/load/P"), live.l.lP /100.f);


  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/co2reduction/t"), stats.s.c02Reduction/100.f);
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/battery/SOC"),   batterySOC/1.0f);
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/battery/netI"),  batteryCurrent/100.0f);
  mqtt_publish_s( concat(const_cast<char*>(mqtt_topic) , "/load/state"),    (char*) (loadState == 1? "on": "off") );  // pimatic state topic does not work with integers or floats ?!?
   
  
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/battery/minV"), stats.s.bVmin /100.f);
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/battery/maxV"), stats.s.bVmax /100.f);
  
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/panel/minV"), stats.s.pVmin /100.f);
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/panel/maxV"), stats.s.pVmax /100.f);
  
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/energy/consumed_day"), stats.s.consEnerDay/100.f );
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/energy/consumed_all"), stats.s.consEnerTotal/100.f );

  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/energy/generated_day"), stats.s.genEnerDay/100.f );
  mqtt_publish_f( concat(const_cast<char*>(mqtt_topic) , "/energy/generated_all"),  stats.s.genEnerTotal/100.f );


  mqtt_publish_s( concat(const_cast<char*>(mqtt_topic) , "/status/batt_volt"), batt_volt_status[status_batt.volt] );
  mqtt_publish_s( concat(const_cast<char*>(mqtt_topic) , "/status/batt_temp"), batt_temp_status[status_batt.temp] );

  //mqtt_publish_s( "solar/status/charger_input", charger_input_status[ charger_input ]  );
  mqtt_publish_s( strcat(const_cast<char*>(mqtt_topic) , "/status/charger_mode"),  charger_charging_status[ charger_mode ] );  
}
