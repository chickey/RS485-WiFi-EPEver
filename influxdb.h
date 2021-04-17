void Influxdb_postData() {
  String poststring;
  String poststring1;
  HTTPClient http;


  //Construct URL for the influxdb
  String url = "http://" + String(myConfig.influxdb_host) + ":" + myConfig.influxdb_httpPort + "/write?db=" + String(myConfig.influxdb_database) + "&u=" + String(myConfig.influxdb_user) + "&p=" + String(myConfig.influxdb_password);
  
  //Output to grafana using a http post
  poststring = poststring + "Panel-Voltage value=" + String(live.l.pV /100.f) + "\n";
  poststring = poststring + "Panel-Amp value=" + String(live.l.pI /100.f) + "\n";
  poststring = poststring + "Panel-Watt value=" + String(live.l.pP /100.f) + "\n";
  poststring = poststring + "Battery-Voltage value=" + String(live.l.bV /100.f) + "\n";
  poststring = poststring + "Battery-Amp value=" + String(live.l.bI /100.f) + "\n";
  poststring = poststring + "Batter-Watt value=" + String(live.l.bP /100.f) + "\n";
  poststring = poststring + "Load-Voltage value=" + String(live.l.lV /100.f) + "\n";
  poststring = poststring + "Load-Amp value=" + String(live.l.lI /100.f) + "\n";
  poststring = poststring + "Load-Watt value=" + String(live.l.lP /100.f) + "\n";
  poststring = poststring + "Battery-Current value=" + String(batteryCurrent/100.f) + "\n";
  poststring = poststring + "Battery-SOC value=" + String(batterySOC/1.0f) + "\n";
  poststring = poststring + "Load-State value=" + (loadState==1?"1":"0") + "\n";
  poststring = poststring + "Battery-MinVolte value=" + String(stats.s.bVmin /100.f) + "\n";
  poststring = poststring + "Battery-MaxVolt value=" + String(stats.s.bVmax/100.f) + "\n";
  poststring = poststring + "Panel-MinVolt value=" + String(stats.s.pVmin/100.f) + "\n";
  poststring = poststring + "Panel-MaxVolt value=" + String(stats.s.pVmax/100.f) + "\n";
  poststring = poststring + "Consumed-Day value=" + String(stats.s.consEnerDay/100.f) + "\n";
  poststring = poststring + "Consumed-All value=" + String(stats.s.consEnerTotal/100.f) + "\n";
  poststring = poststring + "Gen-Day value=" + String(stats.s.genEnerDay/100.f) + "\n";
  poststring = poststring + "Gen-All value=" + String(stats.s.genEnerTotal/100.f) + "\n";
  poststring = poststring + "Battery-Voltage-Status value=" + '"' + String(batt_volt_status[status_batt.volt]) + '"' + "\n";
  poststring = poststring + "Battery-Temp value=" + '"' + String(batt_temp_status[status_batt.temp]) + '"' + "\n";
  poststring = poststring + "Charger-Mode value=" + '"' + String(charger_charging_status[ charger_mode]) + '"' + "\n";
     
  http.begin(url);
  http.addHeader("Content-Type", "data-binary");
  int httpCode = http.POST(poststring);
  String payload = http.getString();
  Serial.println (payload);
  
  WiFiClient client;
  
  if (!client.connect(myConfig.influxdb_host, myConfig.influxdb_httpPort)) {
    Serial.println("connection failed");

  } else {
    // This will send the request to the server
    client.print(String("GET ") + url + " HTTP/1.1\r\nHost: " + myConfig.influxdb_host + "\r\nConnection: close\r\n\r\n");
    
    unsigned long timeout = millis() + 2500;
    // Read all the lines of the reply from server and print them to Serial
    while (client.connected())
    {
      yield();

      if (millis() > timeout) {
        Serial.println(">>> Client Timeout !");
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
