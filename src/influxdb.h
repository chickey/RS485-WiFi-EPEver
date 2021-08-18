void Influxdb_postData() {
  String poststring(0);
  poststring.reserve(768); //reserve string in memory to prevent fragmentation
  HTTPClient http;
  WiFiClient client;

  //Construct URL for the influxdb
  String url(0);
  url.reserve(256);
  url = url + F("http://") + String(myConfig.influxdb_host) + F(":") + myConfig.influxdb_httpPort + F("/write?db=") + String(myConfig.influxdb_database) + F("&u=") + String(myConfig.influxdb_user) + F("&p=") + String(myConfig.influxdb_password);
  
  http.begin(client, url);
  http.addHeader(F("Content-Type"), F("data-binary"));
  
  //Output to grafana using a http post
  poststring.concat(F("Panel-Voltage value="));
  poststring.concat(live.l.pV /100.f);
  poststring.concat(F("\nPanel-Amp value="));
  poststring.concat(live.l.pI /100.f);
  poststring.concat(F("\nPanel-Watt value="));
  poststring.concat(live.l.pP /100.f);
  poststring.concat(F("\nBattery-Voltage value="));
  poststring.concat(live.l.bV /100.f);
  poststring.concat(F("\nBattery-Amp value="));
  poststring.concat(live.l.bI /100.f);
  poststring.concat(F("\nBatter-Watt value="));
  poststring.concat(live.l.bP /100.f);
  poststring.concat(F("\nLoad-Voltage value="));
  poststring.concat(live.l.lV /100.f);
  poststring.concat(F("\nLoad-Amp value="));
  poststring.concat(live.l.lI /100.f);
  poststring.concat(F("\nLoad-Watt value="));
  poststring.concat(live.l.lP /100.f);
  poststring.concat(F("\nBattery-Current value="));
  poststring.concat(batteryCurrent/100.f);
  poststring.concat(F("\nBattery-SOC value="));
  poststring.concat(batterySOC/1.0f);
  poststring.concat(F("\nLoad-State value="));
  poststring.concat(loadState==1?F("1\n"):F("0\n"));
  poststring.concat(F("Battery-MinVolte value="));
  poststring.concat(stats.s.bVmin /100.f);
  poststring.concat(F("\nBattery-MaxVolt value="));
  poststring.concat(stats.s.bVmax/100.f);
  poststring.concat(F("\nPanel-MinVolt value="));
  poststring.concat(stats.s.pVmin/100.f);
  poststring.concat(F("\nPanel-MaxVolt value="));
  poststring.concat(stats.s.pVmax/100.f);
  poststring.concat(F("\nConsumed-Day value="));
  poststring.concat(stats.s.consEnerDay/100.f);
  poststring.concat(F("\nConsumed-All value="));
  poststring.concat(stats.s.consEnerTotal/100.f);
  poststring.concat(F("\nGen-Day value="));
  poststring.concat(stats.s.genEnerDay/100.f);
  poststring.concat(F("\nGen-All value="));
  poststring.concat(stats.s.genEnerTotal/100.f);
  poststring.concat(F("\nBattery-Voltage-Status value=\""));
  poststring.concat(batt_volt_status[status_batt.volt]);
  poststring.concat(F("\nBattery-Temp value=\""));
  poststring.concat(batt_temp_status[status_batt.temp]);
  poststring.concat(F("\nCharger-Mode value=\""));
  poststring.concat(charger_charging_status[charger_mode]);
  poststring.concat(F("\"\n"));

  http.POST(poststring);
  String payload = http.getString();
#ifdef DEBUG
  Serial.println (payload);
#endif
  
  if (!client.connect(myConfig.influxdb_host, myConfig.influxdb_httpPort)) {
#ifdef DEBUG
    Serial.println(F("connection failed"));
#endif
  } else {
    // This will send the request to the server
    client.print(String(F("GET ")) + url + F(" HTTP/1.1\r\nHost: ") + myConfig.influxdb_host + F("\r\nConnection: close\r\n\r\n"));
    
    unsigned long timeout = millis() + 2500;
    // Read all the lines of the reply from server and print them to Serial
    while (client.connected())
    {
      yield();

      if (millis() > timeout) {
#ifdef DEBUG
        Serial.println(F(">>> Client Timeout !"));
#endif
        client.stop();
        return;
      }

      if (client.available())
      {
        String line = client.readStringUntil('\n');
        //Serial.println(line);
      }
    }
    client.stop();
  }
}
