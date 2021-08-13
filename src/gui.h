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
    ESPUI.jsonUpdateDocumentSize = 2000; // This is the default, and this value is not affected by the amount of widgets
    ESPUI.jsonInitialDocumentSize = 12000; // Default is 8000. Increased as there are a lot of widgets causing display to not work on newer versions of ESPUI library
 
    String deviceName = String(DEVICE_NAME) + " v" + String(SW_VERSION);
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
#ifdef DEBUG
    Serial.println("Saving");
#endif
    WriteConfigToEEPROM();
    ESPUI.updateControlValue(savestatustxt , "Changes Saved");
    break;
  }
}

void RebootButtontxt(Control *sender, int type) {
  switch (type) {
  case B_DOWN:
#ifdef DEBUG
    Serial.println("Rebooting");
#endif
    ESP.restart();
    break;
  }
}

void OverVoltDisttxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void OverVoltRecontxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void EQChargeVolttxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void BoostChargeVolttxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void FloatChargeVolttxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void BoostReconChargeVolttxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void BatteryChargePercenttxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void ChargeLimitVolttxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void DischargeLimitVolttxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void LowVoltDisconnecttxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void LowVoltReconnecttxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void UnderVoltWarningVolttxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void UnderVoltReconnectVolttxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void BatteryDischargePercenttxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void BoostDurationtxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void EQDurationtxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void BatteryCapactitytxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void DEVICEIDtxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
  myConfig.Device_ID = atoi ( (sender->value).c_str() );
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void DEVICEBAUDtxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
  Serial.begin(myConfig.Device_BAUD);
#endif
  myConfig.Device_BAUD = atoi ( (sender->value).c_str() );
  Serial.begin(myConfig.Device_BAUD);
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void MQTTIPtxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
  strcpy(myConfig.mqtt_server,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void MQTTPorttxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
  myConfig.mqtt_port = atoi ( (sender->value).c_str() );
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");  
}

void MQTTUsertxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
  strcpy(myConfig.mqtt_username,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void MQTTPasstxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
  strcpy(myConfig.mqtt_password,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void MQTTTopictxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
  strcpy(myConfig.mqtt_topic,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void InfluxDBIPtxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
  strcpy(myConfig.influxdb_host,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void InfluxDBPorttxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
  myConfig.influxdb_httpPort = atoi ( (sender->value).c_str() );
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void InfluxDBtxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
  strcpy(myConfig.influxdb_database,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void InfluxDBUsertxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
  strcpy(myConfig.influxdb_user,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void InfluxDBPasstxt(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
  strcpy(myConfig.influxdb_password,(sender->value).c_str());
  ESPUI.updateControlValue(savestatustxt , "Changes Unsaved");
}

void BatteryTypeList(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void ChargingModeList(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void RatedVoltagelvlList(Control *sender, int type) {
#ifdef DEBUG
  Serial.print("Text: ID: ");
  Serial.print(sender->id);
  Serial.print(", Value: ");
  Serial.println(sender->value);
#endif
}

void LoadSwitch(Control *sender, int value) {
  switch (value) {
  case S_ACTIVE:
    loadState = true;
    switch_load = 1;
    break;

  case S_INACTIVE:
    loadState = false;
    switch_load = 1;
    break;
  }
  
#ifdef DEBUG
  Serial.print(value == S_ACTIVE ? "Active:" : "Inactive");
  Serial.print(" ");
  Serial.println(sender->id);
#endif
}

void InfluxDBEnSwitch(Control *sender, int value) {
  switch (value) {
  case S_ACTIVE:
    myConfig.influxdb_enabled = 1;
    break;

  case S_INACTIVE:
    myConfig.influxdb_enabled = 0;
    break;
  }

#ifdef DEBUG
  Serial.print(value == S_ACTIVE ? "Active:" : "Inactive");
  Serial.print(" ");
  Serial.println(sender->id);
#endif
}

void MQTTEnSwitch(Control *sender, int value) {
  switch (value) {
  case S_ACTIVE:
    myConfig.MQTT_Enable = 1;
    break;

  case S_INACTIVE:
    myConfig.MQTT_Enable = 0;
    break;
  }

#ifdef DEBUG
  Serial.print(value == S_ACTIVE ? "Active:" : "Inactive");
  Serial.print(" ");
  Serial.println(sender->id);
#endif
}
