void handleOTAUpload(AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final)
{
    if (!index)
    {
        Serial.printf("UploadStart: %s\n", filename.c_str());
         // calculate sketch space required for the update, for ESP32 use the max constant
#if defined(ESP32)
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
#else
        const uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
        if (!Update.begin(maxSketchSpace))
#endif
        {
            // start with max available size
            Update.printError(Serial);
        }
#if defined(ESP8266)
        Update.runAsync(true);
#endif
    }

    if (len)
    {
        Update.write(data, len);
    }

    // if the final flag is set then this is the last frame of data
    if (final)
    {
        if (Update.end(true))
        {
            // true to set the size to the current progress
            Serial.printf("Update Success: %ub written\nRebooting...\n", index + len);
            ESP.restart();
        }
        else
        {
            Update.printError(Serial);
        }
    }
}

void setupGUI()
{
    ESPUI.begin("RS485-WiFi v0.51"); // It is important that ESPUI.begin(...) is called first so that ESPUI.server is initalized

    ESPUI.server->on("/ota", 
        HTTP_POST, 
        [](AsyncWebServerRequest* request) { request->send(200); }, 
        handleOTAUpload);

    ESPUI.server->on("/ota", 
        HTTP_GET, 
        [](AsyncWebServerRequest* request) {
            AsyncWebServerResponse* response = request->beginResponse_P(200, "text/html", OTA_INDEX);
            request->send(response);
        }
    );
}

void SaveButtontxt(Control *sender, int type) {
  switch (type) {
  case B_DOWN:
    Serial.println("Saving");
    WriteConfigToEEPROM();
    ESPUI.updateControlValue(savestatustxt , "Changes Saved");
    break;
  }
}

void RebootButtontxt(Control *sender, int type) {
  switch (type) {
  case B_DOWN:
    Serial.println("Rebooting");
    ESP.restart();
    break;
  }
}

void OverVoltDisttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void OverVoltRecontxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void EQChargeVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void BoostChargeVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void FloatChargeVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void BoostReconChargeVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void BatteryChargePercenttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void ChargeLimitVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void DischargeLimitVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void LowVoltDisconnecttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void LowVoltReconnecttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void UnderVoltWarningVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void UnderVoltReconnectVolttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void BatteryDischargePercenttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void BoostDurationtxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void EQDurationtxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void BatteryCapactitytxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void DEVICEIDtxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  myConfig.Device_ID = atoi ( (sender->value).c_str() );
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void DEVICEBAUDtxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  myConfig.Device_BAUD = atoi ( (sender->value).c_str() );
  Serial.begin(myConfig.Device_BAUD);
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void MQTTIPtxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.mqtt_server,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void MQTTPorttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  myConfig.mqtt_port = atoi ( (sender->value).c_str() );
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");  
}

void MQTTUsertxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.mqtt_username,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void MQTTPasstxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.mqtt_password,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void MQTTTopictxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.mqtt_topic,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void InfluxDBIPtxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.influxdb_host,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void InfluxDBPorttxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  myConfig.influxdb_httpPort = atoi ( (sender->value).c_str() );
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void InfluxDBtxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.influxdb_database,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void InfluxDBUsertxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.influxdb_user,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void InfluxDBPasstxt(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  strcpy(myConfig.influxdb_password,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void BatteryTypeList(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void ChargingModeList(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void RatedVoltagelvlList(Control *sender, int type) {
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
}

void LoadSwitch(Control *sender, int value) {
  switch (value) {
  case S_ACTIVE:
    Serial.print("Active:");
    loadState = true;
    do_update = 1;
    switch_load = 1;
    break;

  case S_INACTIVE:
    Serial.print("Inactive");
    loadState = false;
    do_update = 1;
    switch_load = 1;
    break;
  }
  
  Serial.print(" ");
  Serial.println(sender->id);
}

void InfluxDBEnSwitch(Control *sender, int value) {
  switch (value) {
  case S_ACTIVE:
    Serial.print("Active:");
    myConfig.influxdb_enabled = 1;
    break;

  case S_INACTIVE:
    Serial.print("Inactive");
    myConfig.influxdb_enabled = 0;
    break;
  }

  Serial.print(" ");
  Serial.println(sender->id);
}

void MQTTEnSwitch(Control *sender, int value) {
  switch (value) {
  case S_ACTIVE:
    Serial.print("Active:");
    myConfig.MQTT_Enable = 1;
    break;

  case S_INACTIVE:
    Serial.print("Inactive");
    myConfig.MQTT_Enable = 0;
    break;
  }

  Serial.print(" ");
  Serial.println(sender->id);
}
